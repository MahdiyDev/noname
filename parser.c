#include "libs/dynamic_array.h"
#include "libs/error.h"
#include "libs/string.h"
#include "string.h"
#include "expression.h"
#include "parser.h"

struct Error* parse_expression(struct Parser* parser, struct Expr** result);
struct Error* parse_declaration(struct Parser* parser, struct Stmt** result);
struct Error* parse_statement(struct Parser* parser, struct Stmt** result);
struct Error* parse_varaible_declaration(struct Parser* parser, struct Stmt** result);
struct Error* parse_block(struct Parser* parser, Stmts** result);

struct Error* consume_and_expect(struct Parser* parser, const char* expexted_str)
{
    if (!sv_equal_cstr(parser->token->lexeme, expexted_str)) {
        return error_f("at %s:%zu:%zu Expected '%s' but got '%.*s'", lex_loc_fmt_ptr(parser->token), expexted_str, sv_fmt(parser->token->lexeme));
    }
    lex_get_token(parser->lexer, parser->token); // Consume 'expexted_str'
    return NULL;
}

struct Error* parse_primary(struct Parser* parser, struct Expr** result)
{
    struct Error* error = NULL;

    if (parser->token->id == LEXER_VALUE) {
        *result = create_literal_expr(parser->allocator, parser->token->value);
        lex_get_token(parser->lexer, parser->token); // Advance to the next token
        return NULL;
    }

    if (parser->token->id == LEXER_SYMBOL) {
        *result = create_variable_expr(parser->allocator, *parser->token);
        lex_get_token(parser->lexer, parser->token); // Advance to the next token
        return NULL;
    }

    if (sv_equal_cstr(parser->token->lexeme, "\"")) {
        // TODO: support escaping
        *result = create_variable_expr(parser->allocator, *parser->token);
    }

    if (parser->token->id == LEXER_PUNCT && sv_equal_cstr(parser->token->lexeme, "(")) {
        lex_get_token(parser->lexer, parser->token); // Consume '('

        if (has_error(parse_expression(parser, result))) {
            return trace(error);
        }

        if (has_error(consume_and_expect(parser, ")"))) {
            return trace(error);
        }

        *result = create_group_expr(parser->allocator, *result);
        return NULL;
    }

    return error_f("at %s:%zu:%zu Unexpected token '%.*s'", lex_loc_fmt_ptr(parser->token), sv_fmt(parser->token->lexeme));
}

struct Error* parse_finish_call(struct Parser* parser, struct Expr** result)
{
    struct Error* error = NULL;

    if (has_error(consume_and_expect(parser, "("))) {
        return trace(error);
    }

    struct Expr *calle = *result;

    Exprs* arguments = NULL;
    da_init(arguments);

    if (!sv_equal_cstr(parser->token->lexeme, ")")) {
        do {
            if (arguments->count >= 255) {
                return error("Can't have more than 255 arguments.");
            }

            struct Expr* expression = NULL;
            if (has_error(parse_expression(parser, &expression))) {
                trace(error);
            }

            da_append(arguments, expression);

            if (sv_equal_cstr(parser->token->lexeme, ",")) {
                lex_get_token(parser->lexer, parser->token); // Consume ','
            } else {
                break;
            }
        } while(true);
    }

    if (has_error(consume_and_expect(parser, ")"))) {
        return trace(error);
    }

    lexer_token paren = *parser->token;

    *result = create_call_expr(parser->allocator, calle, paren, arguments);

    return NULL;
}

struct Error* parse_call(struct Parser* parser, struct Expr** result)
{
    struct Error* error = NULL;

    if (has_error(parse_primary(parser, result))) {
        return trace(error);
    }

    while (true) {
        if (sv_equal_cstr(parser->token->lexeme, "(")) {
            if (has_error(parse_finish_call(parser, result))) {
                return trace(error);
            }
        } else {
            break;
        }
    }

    return NULL;
}

struct Error* parse_unary(struct Parser* parser, struct Expr** result)
{
    struct Error* error = NULL;

    if (sv_in_carr(parser->token->lexeme, to_c_array(const char*, "!", "-"))) {
        lexer_token operator_tok = *parser->token;
        lex_get_token(parser->lexer, parser->token); // Consume operator

        struct Expr* right = NULL;
        if (has_error(parse_unary(parser, &right))) {
            return trace(error);
        }

        *result = create_unary_expr(parser->allocator, operator_tok, right);
        return NULL;
    }

    return trace(parse_call(parser, result));
}

struct Error* parse_factor(struct Parser* parser, struct Expr** result)
{
    struct Error* error = NULL;

    if (has_error(parse_unary(parser, result))) {
        return trace(error);
    }

    while (sv_in_carr(parser->token->lexeme, to_c_array(const char*, "/", "*"))) {
        lexer_token operator_tok = *parser->token; // Save the current operator

        if (!lex_get_token(parser->lexer, parser->token)) {
            return error_f("at %s:%zu:%zu Unexpected end of input after '%.*s'", lex_loc_fmt_ptr(parser->token), sv_fmt(operator_tok.lexeme));
        }

        struct Expr* right = NULL;
        if (has_error(parse_unary(parser, &right))) {
            return trace(error);
        }

        *result = create_binary_expr(parser->allocator, *result, operator_tok, right);
    }

    return NULL;
}

struct Error* parse_term(struct Parser* parser, struct Expr** result)
{
    struct Error* error = NULL;

    if (has_error(parse_factor(parser, result))) {
        return trace(error);
    }

    while (sv_in_carr(parser->token->lexeme, to_c_array(const char*, "-", "+"))) {
        lexer_token operator_tok = *parser->token; // Save the current operator

        if (!lex_get_token(parser->lexer, parser->token)) {
            return error_f("at %s:%zu:%zu Unexpected end of input after '%.*s'", lex_loc_fmt_ptr(parser->token), sv_fmt(operator_tok.lexeme));
        }

        struct Expr* right = NULL;
        if (has_error(parse_factor(parser, &right))) {
            return trace(error);
        }

        *result = create_binary_expr(parser->allocator, *result, operator_tok, right);
    }

    return NULL;
}

struct Error* parse_comparison(struct Parser* parser, struct Expr** result)
{
    struct Error* error = NULL;

    if (has_error(parse_term(parser, result))) {
        return trace(error);
    }

    while (sv_in_carr(parser->token->lexeme, to_c_array(const char*, ">=", ">", "<=", "<"))) {
        lexer_token operator_tok = *parser->token; // Save the current operator

        if (!lex_get_token(parser->lexer, parser->token)) {
            return error_f("at %s:%zu:%zu Unexpected end of input after '%.*s'", lex_loc_fmt_ptr(parser->token), sv_fmt(operator_tok.lexeme));
        }

        struct Expr* right = NULL;
        if (has_error(parse_term(parser, &right))) {
            return trace(error);
        }

        *result = create_binary_expr(parser->allocator, *result, operator_tok, right);
    }

    return NULL;
}

struct Error* parse_equality(struct Parser* parser, struct Expr** result)
{
    struct Error* error = NULL;

    if (has_error(parse_comparison(parser, result))) {
        return trace(error);
    }

    while (sv_in_carr(parser->token->lexeme, to_c_array(const char*, "!=", "=="))) {
        lexer_token operator_tok = *parser->token; // Save the current operator

        if (!lex_get_token(parser->lexer, parser->token)) {
            return error_f("at %s:%zu:%zu Unexpected end of input after '%.*s'", lex_loc_fmt_ptr(parser->token), sv_fmt(operator_tok.lexeme));
        }

        struct Expr* right = NULL;
        if (has_error(parse_comparison(parser, &right))) {
            return trace(error);
        }

        *result = create_binary_expr(parser->allocator, *result, operator_tok, right);
    }

    return NULL;
}

struct Error* parse_and(struct Parser* parser, struct Expr** result)
{
    struct Error* error = NULL;

    if (has_error(parse_equality(parser, result))) {
        return trace(error);
    }

    while (sv_equal_cstr(parser->token->lexeme, "and")) {
        lexer_token operator_tok = *parser->token; // Save the current operator

        if (!lex_get_token(parser->lexer, parser->token)) {
            return error_f("at %s:%zu:%zu Unexpected end of input after '%.*s'", lex_loc_fmt_ptr(parser->token), sv_fmt(operator_tok.lexeme));
        }

        struct Expr* right = NULL;
        if (has_error(parse_equality(parser, &right))) {
            return trace(error);
        }

        *result = create_logical_expr(parser->allocator, *result, operator_tok, right);
    }

    return NULL;
}

struct Error* parse_or(struct Parser* parser, struct Expr** result)
{
    struct Error* error = NULL;

    if (has_error(parse_and(parser, result))) {
        return trace(error);
    }

    while (sv_equal_cstr(parser->token->lexeme, "or")) {
        lexer_token operator_tok = *parser->token; // Save the current operator

        if (!lex_get_token(parser->lexer, parser->token)) {
            return error_f("at %s:%zu:%zu Unexpected end of input after '%.*s'", lex_loc_fmt_ptr(parser->token), sv_fmt(operator_tok.lexeme));
        }

        struct Expr* right = NULL;
        if (has_error(parse_and(parser, &right))) {
            return trace(error);
        }

        *result = create_logical_expr(parser->allocator, *result, operator_tok, right);
    }

    return NULL;
}

struct Error* parse_assignment(struct Parser* parser, struct Expr** result)
{
    struct Error* error = NULL;

    struct Expr* expr = NULL;
    if (has_error(parse_or(parser, &expr))) {
        return trace(error);
    }

    if (sv_equal_cstr(parser->token->lexeme, "=")) {
        lexer_token equals = *parser->token;
        lex_get_token(parser->lexer, parser->token); // Consume equal '='

        struct Expr* value = NULL;
        if (has_error(parse_assignment(parser, &value))) {
            return trace(error);
        }

        if (expr->type == EXPR_VAR) {
            lexer_token name = expr->variable.name;
            *result = create_assign_expr(parser->allocator, name, value);
            return NULL;
        }

        return error_f("at %s:%zu:%zu Invalid assignment target.", lex_loc_fmt_ptr(parser->token));
    }

    *result = expr;
    return NULL;
}

struct Error* parse_expression(struct Parser* parser, struct Expr** result)
{
    struct Error* error = NULL;

    return trace(parse_assignment(parser, result));
}

struct Error* parse_expression_statement(struct Parser* parser, struct Stmt** result)
{
    struct Error* error = NULL;

    struct Expr* expr = NULL;
    if (has_error(parse_expression(parser, &expr))) {
        return trace(error);
    }

    if (has_error(consume_and_expect(parser, ";"))) {
        return trace(error);
    }

    *result = create_expression_stmt(parser->allocator, expr);

    return NULL;
}

struct Error* parse_if_statement(struct Parser* parser, struct Stmt** result)
{
    struct Error* error = NULL;

    lex_get_token(parser->lexer, parser->token); // Consume 'if'

    if (has_error(consume_and_expect(parser, "("))) {
        return trace(error);
    }

    struct Expr* condition = NULL;
    if (has_error(parse_expression(parser, &condition))) {
        return trace(error);
    }

    if (has_error(consume_and_expect(parser, ")"))) {
        return trace(error);
    }

    struct Stmt* then_branch = NULL;
    if (has_error(parse_declaration(parser, &then_branch))) {
        return trace(error);
    }

    struct Stmt* else_branch = NULL;
    if (sv_equal_cstr(parser->token->lexeme, "else")) {
        lex_get_token(parser->lexer, parser->token); // Consume 'else'

        if (has_error(parse_declaration(parser, &else_branch))) {
            return trace(error);
        }
    }

    *result = create_if_stmt(parser->allocator, condition, then_branch, else_branch);

    return NULL;
}

struct Error* parse_for_statement(struct Parser* parser, struct Stmt** result)
{
    struct Error* error = NULL;

    lex_get_token(parser->lexer, parser->token); // Consume 'for'

    if (has_error(consume_and_expect(parser, "("))) {
        return trace(error);
    }

    struct Stmt* initializer = NULL;
    if (sv_equal_cstr(parser->token->lexeme, ";")) {
        initializer = NULL;
    } else if (sv_equal_cstr(parser->token->lexeme, "var")) {
        if (has_error(parse_varaible_declaration(parser, &initializer))) {
            return trace(error);
        }
    } else {
        if (has_error(parse_expression_statement(parser, &initializer))) {
            return trace(error);
        }
    }

    struct Expr* condition = NULL;
    if (!sv_equal_cstr(parser->token->lexeme, ";")) {
        if (has_error(parse_expression(parser, &condition))) {
            return trace(error);
        }
    }

    if (has_error(consume_and_expect(parser, ";"))) {
        return trace(error);
    }

    struct Expr* increment = NULL;
    if (!sv_equal_cstr(parser->token->lexeme, ")")) {
        if (has_error(parse_expression(parser, &increment))) {
            return trace(error);
        }
    }

    if (has_error(consume_and_expect(parser, ")"))) {
        return trace(error);
    }

    struct Stmt* body = NULL;
    if (has_error(parse_statement(parser, &body))) {
        return trace(error);
    }

    if (increment != NULL) {
        Stmts *statements = NULL;
        da_init(statements);

        da_append(statements, body);
        da_append(statements, create_expression_stmt(parser->allocator, increment));

        body = create_block_stmt(parser->allocator, statements);
    }

    if (condition == NULL) {
        condition = create_literal_expr(
            parser->allocator,
            (struct lexer_token_value) { .type = VALUE_TYPE_INT, .int_value = 1 }
        );
    }
    body = create_while_stmt(parser->allocator, condition, body);

    if (initializer != NULL) {
        Stmts *statements = NULL;
        da_init(statements);

        da_append(statements, initializer);
        da_append(statements, body);

        body = create_block_stmt(parser->allocator, statements);
    }

    *result = body;
    return NULL;
}

struct Error* parse_while_statement(struct Parser* parser, struct Stmt** result)
{
    struct Error* error = NULL;

    lex_get_token(parser->lexer, parser->token); // Consume 'while'

    if (has_error(consume_and_expect(parser, "("))) {
        return trace(error);
    }

    struct Expr* condition = NULL;
    if (has_error(parse_expression(parser, &condition))) {
        return trace(error);
    }

    if (has_error(consume_and_expect(parser, ")"))) {
        return trace(error);
    }

    struct Stmt* body = NULL;
    if (has_error(parse_declaration(parser, &body))) {
        return trace(error);
    }

    *result = create_while_stmt(parser->allocator, condition, body);

    return NULL;
}

struct Error* parse_function_statement(struct Parser* parser, struct Stmt** result, char* kind)
{
    struct Error* error = NULL;

    lex_get_token(parser->lexer, parser->token); // Consume 'fun'
    
    lexer_token name = *parser->token;
    if (parser->token->id != LEXER_SYMBOL) {
        return error_f("Expected %s name.", kind);
    }
    lex_get_token(parser->lexer, parser->token); // Consume 'identifier'

    if (has_error(consume_and_expect(parser, "("))) {
        return trace(error);
    }

    LexerTokens* parameters = NULL;
    da_init(parameters);

    if (!sv_equal_cstr(parser->token->lexeme, ")")) {
        do {
            if (parameters->count >= 255) {
                da_free(parameters);
                return error("Can't have more than 255 parameters.");
            }

            if (parser->token->id != LEXER_SYMBOL) {
                da_free(parameters);
                return error("Expected parameter name.");
            }

            lexer_token param = *parser->token;
            da_append(parameters, param);

            lex_get_token(parser->lexer, parser->token); // Consume 'identifier'

            if (!sv_equal_cstr(parser->token->lexeme, ",")) {
                break;
            }

            lex_get_token(parser->lexer, parser->token); // Consume ','
        } while (true);
    }

    if (has_error(consume_and_expect(parser, ")"))) {
        return trace(error);
    }

    Stmts* body = NULL;
    if (has_error(parse_block(parser, &body))) {
        da_free(parameters);
        return trace(error);
    }

    *result = create_function_stmt(parser->allocator, name, parameters, body);

    return NULL;
}

struct Error* parse_return_statement(struct Parser* parser, struct Stmt** result)
{
    struct Error* error = NULL;

    lexer_token keyword = *parser->token;
    lex_get_token(parser->lexer, parser->token); // Consume 'return'

    struct Expr* value = NULL;
    if (!sv_equal_cstr(parser->token->lexeme, ";")) {
        if (has_error(parse_expression(parser, &value))) {
            return trace(error);
        }
    }

    if (has_error(consume_and_expect(parser, ";"))) {
        return trace(error);
    }

    *result = create_return_stmt(parser->allocator, keyword, value);
    return NULL;
}

struct Error* parse_block(struct Parser* parser, Stmts** result)
{
    struct Error* error = NULL;

    lex_get_token(parser->lexer, parser->token); // Consume '{'
    
    Stmts* statements = NULL;
    da_init(statements);

    while (!sv_equal_cstr(parser->token->lexeme, "}") && parser->token->id != LEXER_END) {
        struct Stmt* statement = NULL;
        if (has_error(parse_declaration(parser, &statement))) {
            return trace(error);
        }

        da_append(statements, statement);
    }

    lex_get_token(parser->lexer, parser->token); // Consume '}'

    *result = statements;
    return NULL;
}

struct Error* parse_statement(struct Parser* parser, struct Stmt** result)
{
    struct Error* error = NULL;

    if (sv_equal_cstr(parser->token->lexeme, "fun")) return trace(parse_function_statement(parser, result, "function"));
    if (sv_equal_cstr(parser->token->lexeme, "return")) return trace(parse_return_statement(parser, result));
    if (sv_equal_cstr(parser->token->lexeme, "for")) return trace(parse_for_statement(parser, result));
    if (sv_equal_cstr(parser->token->lexeme, "if")) return trace(parse_if_statement(parser, result));
    if (sv_equal_cstr(parser->token->lexeme, "while")) return trace(parse_while_statement(parser, result));
    if (sv_equal_cstr(parser->token->lexeme, "{")) {
        Stmts* statements = NULL;

        if (has_error(parse_block(parser, &statements))) {
            return trace(error);
        }

        *result = create_block_stmt(parser->allocator, statements);
        return NULL;
    }

    return trace(parse_expression_statement(parser, result));
}

struct Error* parse_varaible_declaration(struct Parser* parser, struct Stmt** result)
{
    struct Error* error = NULL;

    lex_get_token(parser->lexer, parser->token); // Consume 'var'

    lexer_token name = *parser->token;

    struct Expr* initializer = NULL;

    lex_get_token(parser->lexer, parser->token); // Consume 'var name'
    
    if (sv_equal_cstr(parser->token->lexeme, "=")) {
        lex_get_token(parser->lexer, parser->token); // Consume '='
        if (has_error(parse_expression(parser, &initializer))) {
            return trace(error);
        }
    }
    
    if (has_error(consume_and_expect(parser, ";"))) {
        return trace(error);
    }
    
    *result = create_variable_stmt(parser->allocator, name, initializer);

    return NULL;
}

struct Error* parse_declaration(struct Parser* parser, struct Stmt** result)
{   
    struct Error* error = NULL;

    if (sv_equal_cstr(parser->token->lexeme, "var")) return trace(parse_varaible_declaration(parser, result));

    return trace(parse_statement(parser, result));
}

struct Error* parse(struct Parser* parser, Stmts* result)
{
    struct Error* error = NULL;

    lex_get_token(parser->lexer, parser->token); // Get first token

    while (parser->token->id != LEXER_END) {
        struct Stmt* stmt = NULL;
        if (has_error(parse_declaration(parser, &stmt))) {
            return trace(error);
        }
        da_append(result, stmt);
    }

    return NULL;
}
