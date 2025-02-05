#include "interpreter.h"
#include "libs/dynamic_array.h"
#include "libs/error.h"
#include "libs/string.h"
#include "libs/temp_alloc.h"
#include "statement.h"
#include "string.h"
#include "expression.h"
#include "lexer.h"

struct Error* parse_expression(temp_allocator allocator, lexer* l, lexer_token* t, struct Expr** result);
struct Error* parse_declaration(temp_allocator allocator, lexer* l, lexer_token* t, struct Stmt** result);
struct Error* parse_statement(temp_allocator allocator, lexer* l, lexer_token* t, struct Stmt** result);

struct Error* consume_and_expect(lexer* l, lexer_token* t, const char* expexted_str)
{
    if (!sv_equal_cstr(t->lexeme, expexted_str)) {
        return error_f("at %s:%zu:%zu Expected '%s' but got '%.*s'", lex_loc_fmt_ptr(t), expexted_str, sv_fmt(t->lexeme));
    }
    lex_get_token(l, t); // Consume 'expexted_str'
    return NULL;
}

struct Error* ignore_newline(lexer* l, lexer_token* t)
{
    while (t->id == LEXER_NEWLINE) {
        if (!lex_get_token(l, t) || t->id == LEXER_END) {
            break;
        }
    }

    if (t->id == LEXER_END) {
        return error_f("at %s:%zu:%zu Unexpected end of input while parsing expression.", lex_loc_fmt_ptr(t));
    }
    return NULL;
}

struct Error* parse_primary(temp_allocator allocator, lexer* l, lexer_token* t, struct Expr** result)
{
    struct Error* error = NULL;

    if (has_error(ignore_newline(l, t))) {
        return trace(error);
    }

    if (t->id == LEXER_INT) {
        *result = create_literal_expr(allocator, t->int_value);
        lex_get_token(l, t); // Advance to the next token
        return NULL;
    }

    if (t->id == LEXER_SYMBOL) {
        *result = create_variable_expr(allocator, *t);
        lex_get_token(l, t); // Advance to the next token
        return NULL;
    }

    if (t->id == LEXER_PUNCT && sv_equal_cstr(t->lexeme, "(")) {
        lex_get_token(l, t); // Consume '('

        if (has_error(parse_expression(allocator, l, t, result))) {
            return trace(error);
        }

        if (has_error(consume_and_expect(l, t, ")"))) {
            return trace(error);
        }

        *result = create_group_expr(allocator, *result);
        return NULL;
    }

    return error_f("at %s:%zu:%zu Unexpected token '%.*s'", lex_loc_fmt_ptr(t), sv_fmt(t->lexeme));
}

struct Error* parse_unary(temp_allocator allocator, lexer* l, lexer_token* t, struct Expr** result)
{
    struct Error* error = NULL;

    if (has_error(ignore_newline(l, t))) {
        return trace(error);
    }

    if (sv_in_carr(t->lexeme, to_c_array(const char*, "!", "-"))) {
        lexer_token operator_tok = *t;
        lex_get_token(l, t); // Consume operator

        struct Expr* right = NULL;
        if (has_error(parse_unary(allocator, l, t, &right))) {
            return trace(error);
        }

        *result = create_unary_expr(allocator, operator_tok, right);
        return NULL;
    }

    return trace(parse_primary(allocator, l, t, result));
}

struct Error* parse_factor(temp_allocator allocator, lexer* l, lexer_token* t, struct Expr** result)
{
    struct Error* error = NULL;
    if (has_error(parse_unary(allocator, l, t, result))) {
        return trace(error);
    }

    if (has_error(ignore_newline(l, t))) {
        return trace(error);
    }

    while (sv_in_carr(t->lexeme, to_c_array(const char*, "/", "*"))) {
        lexer_token operator_tok = *t; // Save the current operator

        if (!lex_get_token(l, t)) {
            return error_f("at %s:%zu:%zu Unexpected end of input after '%.*s'", lex_loc_fmt_ptr(t), sv_fmt(operator_tok.lexeme));
        }

        struct Expr* right = NULL;
        if (has_error(parse_unary(allocator, l, t, &right))) {
            return trace(error);
        }

        *result = create_binary_expr(allocator, *result, operator_tok, right);
    }

    return NULL;
}

struct Error* parse_term(temp_allocator allocator, lexer* l, lexer_token* t, struct Expr** result)
{
    struct Error* error = NULL;
    if (has_error(parse_factor(allocator, l, t, result))) {
        return trace(error);
    }

    if (has_error(ignore_newline(l, t))) {
        return trace(error);
    }

    while (sv_in_carr(t->lexeme, to_c_array(const char*, "-", "+"))) {
        lexer_token operator_tok = *t; // Save the current operator

        if (!lex_get_token(l, t)) {
            return error_f("at %s:%zu:%zu Unexpected end of input after '%.*s'", lex_loc_fmt_ptr(t), sv_fmt(operator_tok.lexeme));
        }

        struct Expr* right = NULL;
        if (has_error(parse_factor(allocator, l, t, &right))) {
            return trace(error);
        }

        *result = create_binary_expr(allocator, *result, operator_tok, right);
    }

    return NULL;
}

struct Error* parse_comparison(temp_allocator allocator, lexer* l, lexer_token* t, struct Expr** result)
{
    struct Error* error = NULL;
    if (has_error(parse_term(allocator, l, t, result))) {
        return trace(error);
    }

    if (has_error(ignore_newline(l, t))) {
        return trace(error);
    }

    while (sv_in_carr(t->lexeme, to_c_array(const char*, ">", ">=", "<", "<="))) {
        lexer_token operator_tok = *t; // Save the current operator

        if (!lex_get_token(l, t)) {
            return error_f("at %s:%zu:%zu Unexpected end of input after '%.*s'", lex_loc_fmt_ptr(t), sv_fmt(operator_tok.lexeme));
        }

        struct Expr* right = NULL;
        if (has_error(parse_term(allocator, l, t, &right))) {
            return trace(error);
        }

        *result = create_binary_expr(allocator, *result, operator_tok, right);
    }

    return NULL;
}

struct Error* parse_equality(temp_allocator allocator, lexer* l, lexer_token* t, struct Expr** result)
{
    struct Error* error = NULL;
    if (has_error(parse_comparison(allocator, l, t, result))) {
        return trace(error);
    }

    if (has_error(ignore_newline(l, t))) {
        return trace(error);
    }

    while (sv_in_carr(t->lexeme, to_c_array(const char*, "!=", "=="))) {
        lexer_token operator_tok = *t; // Save the current operator

        if (!lex_get_token(l, t)) {
            return error_f("at %s:%zu:%zu Unexpected end of input after '%.*s'", lex_loc_fmt_ptr(t), sv_fmt(operator_tok.lexeme));
        }

        struct Expr* right = NULL;
        if (has_error(parse_comparison(allocator, l, t, &right))) {
            return trace(error);
        }

        *result = create_binary_expr(allocator, *result, operator_tok, right);
    }

    return NULL;
}

struct Error* parse_and(temp_allocator allocator, lexer* l, lexer_token* t, struct Expr** result)
{
    struct Error* error = NULL;

    if (has_error(parse_equality(allocator, l, t, result))) {
        return trace(error);
    }

    while (sv_equal_cstr(t->lexeme, "and")) {
        lexer_token operator_tok = *t; // Save the current operator

        if (!lex_get_token(l, t)) {
            return error_f("at %s:%zu:%zu Unexpected end of input after '%.*s'", lex_loc_fmt_ptr(t), sv_fmt(operator_tok.lexeme));
        }

        struct Expr* right = NULL;
        if (has_error(parse_equality(allocator, l, t, &right))) {
            return trace(error);
        }

        *result = create_logical_expr(allocator, *result, operator_tok, right);
    }

    return NULL;
}

struct Error* parse_or(temp_allocator allocator, lexer* l, lexer_token* t, struct Expr** result)
{
    struct Error* error = NULL;

    if (has_error(parse_and(allocator, l, t, result))) {
        return trace(error);
    }

    while (sv_equal_cstr(t->lexeme, "or")) {
        lexer_token operator_tok = *t; // Save the current operator

        if (!lex_get_token(l, t)) {
            return error_f("at %s:%zu:%zu Unexpected end of input after '%.*s'", lex_loc_fmt_ptr(t), sv_fmt(operator_tok.lexeme));
        }

        struct Expr* right = NULL;
        if (has_error(parse_and(allocator, l, t, &right))) {
            return trace(error);
        }

        *result = create_logical_expr(allocator, *result, operator_tok, right);
    }

    return NULL;
}

struct Error* parse_assignment(temp_allocator allocator, lexer* l, lexer_token* t, struct Expr** result)
{
    struct Error* error = NULL;

    struct Expr* expr = NULL;
    if (has_error(parse_or(allocator, l, t, &expr))) {
        return trace(error);
    }

    if (has_error(ignore_newline(l, t))) {
        return trace(error);
    }

    if (sv_equal_cstr(t->lexeme, "=")) {
        lexer_token equals = *t;
        lex_get_token(l, t); // Consume equal '='

        struct Expr* value = NULL;
        if (has_error(parse_assignment(allocator, l, t, &value))) {
            return trace(error);
        }

        if (expr->type == EXPR_VAR) {
            lexer_token name = expr->variable.name;
            *result = create_assign_expr(allocator, name, value);
            return NULL;
        }

        return error_f("at %s:%zu:%zu Invalid assignment target.", lex_loc_fmt_ptr(t));
    }

    *result = expr;
    return NULL;
}

struct Error* parse_expression(temp_allocator allocator, lexer* l, lexer_token* t, struct Expr** result)
{
    struct Error* error = NULL;

    if (has_error(ignore_newline(l, t))) {
        return trace(error);
    }

    return trace(parse_assignment(allocator, l, t, result));
}

struct Error* parse_expression_statement(temp_allocator allocator, lexer* l, lexer_token* t, struct Stmt** result)
{
    struct Error* error = NULL;

    struct Expr* expr = NULL;
    if (has_error(parse_expression(allocator, l, t, &expr))) {
        return trace(error);
    }

    if (has_error(ignore_newline(l, t))) {
        return trace(error);
    }

    if (has_error(consume_and_expect(l, t, ";"))) {
        return trace(error);
    }

    *result = create_expression_stmt(allocator, expr);

    return NULL;
}

struct Error* parse_if_statement(temp_allocator allocator, lexer* l, lexer_token* t, struct Stmt** result)
{
    struct Error* error = NULL;
    lex_get_token(l, t); // Consume 'if'

    if (has_error(consume_and_expect(l, t, "("))) {
        return trace(error);
    }

    struct Expr* condition = NULL;
    if (has_error(parse_expression(allocator, l, t, &condition))) {
        return trace(error);
    }

    if (has_error(consume_and_expect(l, t, ")"))) {
        return trace(error);
    }

    struct Stmt* then_branch = NULL;
    if (has_error(parse_declaration(allocator, l, t, &then_branch))) {
        return trace(error);
    }

    struct Stmt* else_branch = NULL;
    if (sv_equal_cstr(t->lexeme, "else")) {
        lex_get_token(l, t); // Consume 'else'

        if (has_error(parse_declaration(allocator, l, t, &else_branch))) {
            return trace(error);
        }
    }

    *result = create_if_stmt(allocator, condition, then_branch, else_branch);

    return NULL;
}

struct Error* parse_print_statement(temp_allocator allocator, lexer* l, lexer_token* t, struct Stmt** result)
{
    struct Error* error = NULL;
    lex_get_token(l, t); // Consume 'print'

    if (has_error(consume_and_expect(l, t, "("))) {
        return trace(error);
    }

    struct Expr* value = NULL;
    if (has_error(parse_expression(allocator, l, t, &value))) {
        return trace(error);
    }

    if (has_error(consume_and_expect(l, t, ")"))) {
        return trace(error);
    }

    if (has_error(ignore_newline(l, t))) {
        return trace(error);
    }

    if (has_error(consume_and_expect(l, t, ";"))) {
        return trace(error);
    }

    *result = create_print_stmt(allocator, value);

    return NULL;
}

struct Error* parse_while_statement(temp_allocator allocator, lexer* l, lexer_token* t, struct Stmt** result)
{
    struct Error* error = NULL;
    lex_get_token(l, t); // Consume 'while'

    if (has_error(consume_and_expect(l, t, "("))) {
        return trace(error);
    }

    struct Expr* condition = NULL;
    if (has_error(parse_expression(allocator, l, t, &condition))) {
        return trace(error);
    }

    if (has_error(consume_and_expect(l, t, ")"))) {
        return trace(error);
    }

    if (has_error(ignore_newline(l, t))) {
        return trace(error);
    }

    struct Stmt* body = NULL;
    if (has_error(parse_declaration(allocator, l, t, &body))) {
        return trace(error);
    }

    *result = create_while_stmt(allocator, condition, body);

    return NULL;
}

struct Error* parse_block(temp_allocator allocator, lexer* l, lexer_token* t, Stmts** result)
{
    struct Error* error = NULL;

    lex_get_token(l, t); // Consume '{'
    
    if (has_error(ignore_newline(l, t))) {
        return trace(error);
    }

    Stmts* statements = NULL;
    da_init(statements);

    while (!sv_equal_cstr(t->lexeme, "}") && t->id != LEXER_END) {
        struct Stmt* statement = NULL;
        if (has_error(parse_declaration(allocator, l, t, &statement))) {
            return trace(error);
        }

        da_append(statements, statement);

        if (has_error(ignore_newline(l, t))) {
            return trace(error);
        }
    }

    lex_get_token(l, t); // Consume '}'

    *result = statements;
    return NULL;
}

struct Error* parse_statement(temp_allocator allocator, lexer* l, lexer_token* t, struct Stmt** result)
{
    struct Error* error = NULL;
    
    if (has_error(ignore_newline(l, t))) {
        return trace(error);
    }

    if (sv_equal_cstr(t->lexeme, "if")) return trace(parse_if_statement(allocator, l, t, result));
    if (sv_equal_cstr(t->lexeme, "print")) return trace(parse_print_statement(allocator, l, t, result));
    if (sv_equal_cstr(t->lexeme, "while")) return trace(parse_while_statement(allocator, l, t, result));
    if (sv_equal_cstr(t->lexeme, "{")) {
        Stmts* statements = NULL;

        if (has_error(parse_block(allocator, l, t, &statements))) {
            return trace(error);
        }

        *result = create_block_stmt(allocator, statements);
        return NULL;
    }

    return trace(parse_expression_statement(allocator, l, t, result));
}

struct Error* parse_varaible_declaration(temp_allocator allocator, lexer* l, lexer_token* t, struct Stmt** result)
{
    struct Error* error = NULL;
    lex_get_token(l, t); // Consume 'var'

    lexer_token name = *t;

    struct Expr* initializer = NULL;

    lex_get_token(l, t); // Consume 'var name'
    
    if (sv_equal_cstr(t->lexeme, "=")) {
        lex_get_token(l, t); // Consume '='
        if (has_error(parse_expression(allocator, l, t, &initializer))) {
            return trace(error);
        }
    }
    
    if (has_error(consume_and_expect(l, t, ";"))) {
        return trace(error);
    }
    
    *result = create_variable_stmt(allocator, name, initializer);

    return NULL;
}

struct Error* parse_declaration(temp_allocator allocator, lexer* l, lexer_token* t, struct Stmt** result)
{   
    struct Error* error = NULL;
    
    if (has_error(ignore_newline(l, t))) {
        return trace(error);
    }

    if (sv_equal_cstr(t->lexeme, "var")) return trace(parse_varaible_declaration(allocator, l, t, result));

    return trace(parse_statement(allocator, l, t, result));
}

struct Error* parse(temp_allocator allocator, lexer* l, lexer_token* t, Stmts* result)
{
    struct Error* error = NULL;

    lex_get_token(l, t); // Get first token

    while (t->id != LEXER_END) {
        if (has_error(ignore_newline(l, t))) {
            return NULL;
        }

        struct Stmt* stmt = NULL;
        if (has_error(parse_declaration(allocator, l, t, &stmt))) {
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

    if (has_error(parse(allocator, &l, &t, stmts))) {
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
