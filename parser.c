#include "interpreter.h"
#include "libs/dynamic_array.h"
#include "libs/error.h"
#include "libs/string.h"
#include "libs/temp_alloc.h"
#include "statement.h"
#include "string.h"
#include "expression.h"
#include "lexer.h"

struct Parser {
    temp_allocator allocator;
    lexer* lexer;
    lexer_token* token;
};

struct Error* parse_expression(struct Parser* parser, struct Expr** result);
struct Error* parse_declaration(struct Parser* parser, struct Stmt** result);
struct Error* parse_statement(struct Parser* parser, struct Stmt** result);

struct Error* consume_and_expect(struct Parser* parser, const char* expexted_str)
{
    if (!sv_equal_cstr(parser->token->lexeme, expexted_str)) {
        return error_f("at %s:%zu:%zu Expected '%s' but got '%.*s'", lex_loc_fmt_ptr(parser->token), expexted_str, sv_fmt(parser->token->lexeme));
    }
    lex_get_token(parser->lexer, parser->token); // Consume 'expexted_str'
    return NULL;
}

struct Error* ignore_newline(struct Parser* parser)
{
    while (parser->token->id == LEXER_NEWLINE) {
        if (!lex_get_token(parser->lexer, parser->token) || parser->token->id == LEXER_END) {
            break;
        }
    }

    if (parser->token->id == LEXER_END) {
        return error_f("at %s:%zu:%zu Unexpected end of input while parsing expression.", lex_loc_fmt_ptr(parser->token));
    }
    return NULL;
}

struct Error* parse_primary(struct Parser* parser, struct Expr** result)
{
    struct Error* error = NULL;

    if (has_error(ignore_newline(parser))) {
        return trace(error);
    }

    if (parser->token->id == LEXER_INT) {
        *result = create_literal_expr(parser->allocator, parser->token->int_value);
        lex_get_token(parser->lexer, parser->token); // Advance to the next token
        return NULL;
    }

    if (parser->token->id == LEXER_SYMBOL) {
        *result = create_variable_expr(parser->allocator, *parser->token);
        lex_get_token(parser->lexer, parser->token); // Advance to the next token
        return NULL;
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

struct Error* parse_unary(struct Parser* parser, struct Expr** result)
{
    struct Error* error = NULL;

    if (has_error(ignore_newline(parser))) {
        return trace(error);
    }

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

    return trace(parse_primary(parser, result));
}

struct Error* parse_factor(struct Parser* parser, struct Expr** result)
{
    struct Error* error = NULL;
    if (has_error(parse_unary(parser, result))) {
        return trace(error);
    }

    if (has_error(ignore_newline(parser))) {
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

    if (has_error(ignore_newline(parser))) {
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

    if (has_error(ignore_newline(parser))) {
        return trace(error);
    }

    while (sv_in_carr(parser->token->lexeme, to_c_array(const char*, ">", ">=", "<", "<="))) {
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

    if (has_error(ignore_newline(parser))) {
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

    if (has_error(ignore_newline(parser))) {
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

    if (has_error(ignore_newline(parser))) {
        return trace(error);
    }

    return trace(parse_assignment(parser, result));
}

struct Error* parse_expression_statement(struct Parser* parser, struct Stmt** result)
{
    struct Error* error = NULL;

    struct Expr* expr = NULL;
    if (has_error(parse_expression(parser, &expr))) {
        return trace(error);
    }

    if (has_error(ignore_newline(parser))) {
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

struct Error* parse_print_statement(struct Parser* parser, struct Stmt** result)
{
    struct Error* error = NULL;
    lex_get_token(parser->lexer, parser->token); // Consume 'print'

    if (has_error(consume_and_expect(parser, "("))) {
        return trace(error);
    }

    struct Expr* value = NULL;
    if (has_error(parse_expression(parser, &value))) {
        return trace(error);
    }

    if (has_error(consume_and_expect(parser, ")"))) {
        return trace(error);
    }

    if (has_error(ignore_newline(parser))) {
        return trace(error);
    }

    if (has_error(consume_and_expect(parser, ";"))) {
        return trace(error);
    }

    *result = create_print_stmt(parser->allocator, value);

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

    if (has_error(ignore_newline(parser))) {
        return trace(error);
    }

    struct Stmt* body = NULL;
    if (has_error(parse_declaration(parser, &body))) {
        return trace(error);
    }

    *result = create_while_stmt(parser->allocator, condition, body);

    return NULL;
}

struct Error* parse_block(struct Parser* parser, Stmts** result)
{
    struct Error* error = NULL;

    lex_get_token(parser->lexer, parser->token); // Consume '{'
    
    if (has_error(ignore_newline(parser))) {
        return trace(error);
    }

    Stmts* statements = NULL;
    da_init(statements);

    while (!sv_equal_cstr(parser->token->lexeme, "}") && parser->token->id != LEXER_END) {
        struct Stmt* statement = NULL;
        if (has_error(parse_declaration(parser, &statement))) {
            return trace(error);
        }

        da_append(statements, statement);

        if (has_error(ignore_newline(parser))) {
            return trace(error);
        }
    }

    lex_get_token(parser->lexer, parser->token); // Consume '}'

    *result = statements;
    return NULL;
}

struct Error* parse_statement(struct Parser* parser, struct Stmt** result)
{
    struct Error* error = NULL;
    
    if (has_error(ignore_newline(parser))) {
        return trace(error);
    }

    if (sv_equal_cstr(parser->token->lexeme, "if")) return trace(parse_if_statement(parser, result));
    if (sv_equal_cstr(parser->token->lexeme, "print")) return trace(parse_print_statement(parser, result));
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
    
    if (has_error(ignore_newline(parser))) {
        return trace(error);
    }

    if (sv_equal_cstr(parser->token->lexeme, "var")) return trace(parse_varaible_declaration(parser, result));

    return trace(parse_statement(parser, result));
}

struct Error* parse(struct Parser* parser, Stmts* result)
{
    struct Error* error = NULL;

    lex_get_token(parser->lexer, parser->token); // Get first token

    while (parser->token->id != LEXER_END) {
        if (has_error(ignore_newline(parser))) {
            return NULL;
        }

        struct Stmt* stmt = NULL;
        if (has_error(parse_declaration(parser, &stmt))) {
            return trace(error);
        }
        da_append(result, stmt);
    }

    return NULL;
}

const char* puncts[] = { "(", ")", "{", "}", ",", ".", "-", "+", ";", "*" };

const char *sl_comments[] = {
    "//",
};

const multi_line_comments ml_comments[] = {
    {"/*", "*/"},
};

int main(int argc, char** argv)
{
    int exit_code = EXIT_SUCCESS;

    if (argc < 2) {
        fprintf(stderr, "usage: %s file\n", argv[0]);
        return EXIT_FAILURE;
    }
    const char* file_path = argv[1];

    string_builder* sb = sb_init(NULL);
    if (!sb_read_file(sb, file_path)) return_defer(exit_code, EXIT_FAILURE);

    lexer l = lexer_create(file_path, sb_to_sv(sb));

    l.puncts = puncts;
    l.puncts_count = arr_count(puncts);
    l.sl_comments = sl_comments;
    l.sl_comments_count = arr_count(sl_comments);
    l.ml_comments = ml_comments;
    l.ml_comments_count = arr_count(ml_comments);

    lexer_token t = {0};

    struct Error* error = NULL;

    Stmts* stmts = NULL;
    da_init(stmts);

    temp_allocator allocator = temp_init();

    struct Parser parser;

    parser.lexer = &l;
    parser.token = &t;
    parser.allocator = allocator;

    if (has_error(parse(&parser, stmts))) {
        print_error(error);

        for (size_t i = 0; i < stmts->count; i++) {
            free_stmt(stmts->items[i]);
        }

        da_free(stmts);
        return_defer(exit_code, EXIT_FAILURE);
    }

    struct Interpreter* intp = interpreter_init();

    if (has_error(interpret(intp, stmts))) {
        print_error(error);

        interpreter_destroy(intp);
        da_free(stmts);
        return_defer(exit_code, EXIT_FAILURE);
    }

    interpreter_destroy(intp);

    for (size_t i = 0; i < stmts->count; i++) {
        free_stmt(stmts->items[i]);
    }

    da_free(stmts);

defer:
    sb_free(sb);
    return exit_code;
}
