#include "function.h"
#include "lexer.h"
#include "libs/string.h"
#include "interpreter.h"
#include "environment.h"
#include <string.h>

#define UNREACHABLE() { fprintf(stderr, "%s:%d\n", __FILE__, __LINE__); abort();}

struct Error* evaluate(struct Interpreter* intp, struct Expr* expr, struct lexer_token_value* result);
struct Error* execute(struct Interpreter* intp, struct Stmt* stmt);

struct Error* visit_literal_expr(struct Expr* expr, struct lexer_token_value* result)
{
    *result = expr->literal.value;
    return NULL;
}

struct Error* visit_group_expr(struct Interpreter* intp, struct Expr* expr, struct lexer_token_value* result)
{
    struct Error* error = NULL;

    if (has_error(evaluate(intp, expr->group.expression, result))) {
        return trace(error);
    }

    return NULL;
}

bool is_truthy(int value)
{
    return value == true;
}

struct Error* visit_unary_expr(struct Interpreter* intp, struct Expr* expr, struct lexer_token_value* result)
{
    struct Error* error = NULL;

    struct lexer_token_value right = {0};
    if (has_error(evaluate(intp, expr->unary.right, &right))) {
        return trace(error);
    }

    if (right.type != VALUE_TYPE_INT) {
        return error("Can't do unary expression.");
    }

    if (sv_equal_cstr(expr->unary.operator.lexeme, "-")) {
        result->int_value = -right.int_value;
        return NULL;
    }
    else if (sv_equal_cstr(expr->unary.operator.lexeme, "!")) {
        // TODO: change this to boolean value
        result->int_value = !is_truthy(right.int_value);
        return NULL;
    }

    UNREACHABLE();
}

struct Error* visit_binary_expr(struct Interpreter* intp, struct Expr* expr, struct lexer_token_value* result)
{
    struct Error* error = NULL;

    struct lexer_token_value left_value = {0};
    if (has_error(evaluate(intp, expr->binary.left, &left_value))) {
        return trace(error);
    }

    if (left_value.type != VALUE_TYPE_INT) {
        return error("Can't do binnary expression.");
    }

    struct lexer_token_value right_value = {0};
    if (has_error(evaluate(intp, expr->binary.right, &right_value))) {
        return trace(error);
    }

    if (right_value.type != VALUE_TYPE_INT) {
        return error("Can't do binnary expression.");
    }

    int left = left_value.int_value;
    int right = right_value.int_value;

    if (sv_equal_cstr(expr->binary.operator.lexeme, "==")) {
        result->int_value = left == right;
        return NULL;
    }
    else if (sv_equal_cstr(expr->binary.operator.lexeme, "!=")) {
        result->int_value = left != right;
        return NULL;
    }
    else if (sv_equal_cstr(expr->binary.operator.lexeme, ">=")) {
        result->int_value = left >= right;
        return NULL;
    }
    else if (sv_equal_cstr(expr->binary.operator.lexeme, ">")) {
        result->int_value = left > right;
        return NULL;
    }
    else if (sv_equal_cstr(expr->binary.operator.lexeme, "<=")) {
        result->int_value = left <= right;
        return NULL;
    }
    else if (sv_equal_cstr(expr->binary.operator.lexeme, "<")) {
        result->int_value = left < right;
        return NULL;
    }
    else if (sv_equal_cstr(expr->binary.operator.lexeme, "-")) {
        result->int_value = left - right;
        return NULL;
    }
    else if (sv_equal_cstr(expr->binary.operator.lexeme, "+")) {
        result->int_value = left + right;
        return NULL;
    }
    else if (sv_equal_cstr(expr->binary.operator.lexeme, "/")) {
        result->int_value = left / right;
        return NULL;
    }
    else if (sv_equal_cstr(expr->binary.operator.lexeme, "*")) {
        result->int_value = left * right;
        return NULL;
    }

    UNREACHABLE();
}

struct Error* visit_variable_expr(struct Interpreter* intp, struct Expr* expr, struct lexer_token_value* result)
{
    struct Error* error = NULL;

    return trace(env_get(intp->env, expr->variable.name, result));
}

struct Error* visit_assign_expr(struct Interpreter* intp, struct Expr* expr, struct lexer_token_value* result)
{
    struct Error* error = NULL;

    struct lexer_token_value value = {0};
    if (has_error(evaluate(intp, expr->assign.value, &value))) {
        return trace(error);
    }

    return trace(env_assign(intp->env, expr->assign.name, value));
}

struct Error* visit_logical_expr(struct Interpreter* intp, struct Expr* expr, struct lexer_token_value* result)
{
    struct Error* error = NULL;

    struct lexer_token_value left = {0};
    if (has_error(evaluate(intp, expr->logical.left, &left))) {
        return trace(error);
    }

    if (left.type != VALUE_TYPE_INT) {
        return error("Can't do logical expression.");
    }

    if (sv_equal_cstr(expr->logical.operator.lexeme, "or")) {
        if (is_truthy(left.int_value)) {
            *result = left;
            return NULL;
        }
    } else {
        if (!is_truthy(left.int_value)) {
            *result = left;
            return NULL;
        }
    }

    struct lexer_token_value right = {0};
    if (has_error(evaluate(intp, expr->logical.right, &right))) {
        return trace(error);
    }
    *result = right;

    return NULL;
}

struct Error* visit_call_expr(struct Interpreter* intp, struct Expr* expr, struct lexer_token_value* result)
{
    struct Error* error = NULL;

    struct lexer_token_value calle = {0};
    if (has_error(evaluate(intp, expr->call.calle, &calle))) {
        return trace(error);
    }

    Arguments* arguments = NULL;
    da_init(arguments);

    for (int i = 0; i < expr->call.arguments->count; i++) {
        struct lexer_token_value argument = {0};
        if (has_error(evaluate(intp, expr->call.arguments->items[i], &argument))) {
            return trace(error);
        }

        da_append(arguments, argument);
    }

    if (calle.type != VALUE_TYPE_CALLABLE) {
        return error("Can only call functions");
    }

    if (arguments->count != calle.callable_value.arity) {
        return error_f("at %s:%zu:%zu Expected %d arguments but got %zu,", lex_loc_fmt(expr->call.paren), calle.callable_value.arity, arguments->count);
    }

    if (has_error(calle.callable_value.call(calle.callable_value, intp, arguments, result))) {
        return trace(error);
    }

    return NULL;
}

struct Error* evaluate(struct Interpreter* intp, struct Expr* expr, struct lexer_token_value* result)
{
    struct Error* error = NULL;

    switch (expr->type) {
    case EXPR_BINARY:
        return trace(visit_binary_expr(intp, expr, result));
    case EXPR_UNARY:
        return trace(visit_unary_expr(intp, expr, result));
    case EXPR_GROUP:
        return trace(visit_group_expr(intp, expr, result));
    case EXPR_LITERAL:
        return trace(visit_literal_expr(expr, result));
    case EXPR_VAR:
        return trace(visit_variable_expr(intp, expr, result));
    case EXPR_ASSIGN:
        return trace(visit_assign_expr(intp, expr, result));
    case EXPR_LOGICAL:
        return trace(visit_logical_expr(intp, expr, result));
    case EXPR_CALL:
        return trace(visit_call_expr(intp, expr, result));
    }

    UNREACHABLE();
}

struct Error* visit_expression_stmt(struct Interpreter* intp, struct Stmt* stmt)
{
    struct Error* error = NULL;

    struct lexer_token_value value = {0};
    return trace(evaluate(intp, stmt->expression.expression, &value));
}

struct Error* visit_print_stmt(struct Interpreter* intp, struct Stmt* stmt)
{
    struct Error* error = NULL;

    struct lexer_token_value value = {0};
    if (has_error(evaluate(intp, stmt->print.expression, &value))) {
        return trace(error);
    }

    switch (value.type) {
    case VALUE_TYPE_INT:
        printf("%d\n", value.int_value);
        break;
    case VALUE_TYPE_INT_LONG_LONG:
        printf("%lld\n", value.int_long_long_value);
        break;
    case VALUE_TYPE_STRING: 
        printf("%.*s\n", sv_fmt(value.string_value));
        break;
    case VALUE_TYPE_CALLABLE: 
        printf("<native fun>\n");
        break;
    }

    return NULL;
}

struct Error* visit_variable_stmt(struct Interpreter* intp, struct Stmt* stmt)
{
    struct Error* error = NULL;

    struct lexer_token_value value = {0};
    if (stmt->variable.initializer != NULL) {
        if (has_error(evaluate(intp, stmt->variable.initializer, &value))) {
            return trace(error);
        }
    }

    env_define(intp->env, stmt->variable.name.lexeme, value);
    return NULL;
}

struct Error* execute_block(struct Interpreter* intp, Stmts* stmts, struct Enviroment* env)
{
    struct Error* error = NULL;

    struct Enviroment* prev_env = intp->env;

    for (size_t i = 0; i < stmts->count; i++) {
        intp->env = env;

        if (has_error(execute(intp, stmts->items[i]))) {
            intp->env = prev_env; // Restore env just in case
            return trace(error);
        }
    }

    env_destroy(env);
    intp->env = prev_env; // Restore env
    return NULL;
}

struct Error* visit_block_stmt(struct Interpreter* intp, struct Stmt* stmt)
{
    struct Error* error = NULL;

    return trace(execute_block(intp, stmt->block.statements, env_init(intp->env)));
}

struct Error* visit_if_stmt(struct Interpreter* intp, struct Stmt* stmt)
{
    struct Error* error = NULL;

    struct lexer_token_value value = {0};
    if (has_error(evaluate(intp, stmt->if_stmt.condition, &value))) {
        return trace(error);
    }

    if (value.type != VALUE_TYPE_INT) {
        return error("Can't do if statement.");
    }

    if (is_truthy(value.int_value)) {
        if (has_error(execute(intp, stmt->if_stmt.then_branch))) {
            return trace(error);
        }
    } else if (stmt->if_stmt.else_branch != NULL) {
        if (has_error(execute(intp, stmt->if_stmt.else_branch))) {
            return trace(error);
        }
    }

    return NULL;
}

struct Error* visit_while_stmt(struct Interpreter* intp, struct Stmt* stmt)
{
    struct Error* error = NULL;

    struct lexer_token_value value = {0};
    if (has_error(evaluate(intp, stmt->while_stmt.condition, &value))) {
        return trace(error);
    }

    while (is_truthy(value.int_value)) {
        if (has_error(execute_block(intp, stmt->while_stmt.body->block.statements, env_init(intp->env)))) {
            return trace(error);
        }

        if (has_error(evaluate(intp, stmt->while_stmt.condition, &value))) {
            return trace(error);
        }
    }

    return NULL;
}

struct Error* visit_function_stmt(struct Interpreter* intp, struct Stmt* stmt)
{
    struct Error* error = NULL;
    struct lexer_token_value function = create_function(stmt);

    env_define(intp->env, stmt->function_stmt.name.lexeme, function);
    return NULL;
}

struct Error* execute(struct Interpreter* intp, struct Stmt* stmt)
{
    struct Error* error = NULL;

    switch (stmt->type) {
    case STMT_EXPRESSION:
        return trace(visit_expression_stmt(intp, stmt));
    case STMT_PRINT:
        return trace(visit_print_stmt(intp, stmt));
    case STMT_VAR:
        return trace(visit_variable_stmt(intp, stmt));
    case STMT_BLOCK:
        return trace(visit_block_stmt(intp, stmt));
    case STMT_IF:
        return trace(visit_if_stmt(intp, stmt));
    case STMT_WHILE:
        return trace(visit_while_stmt(intp, stmt));
    case STMT_FUNCTION:
        return trace(visit_function_stmt(intp, stmt));
    }
}

struct Interpreter* interpreter_init()
{
    struct Interpreter* intp = malloc(sizeof(struct Interpreter));
    
    intp->global_env = env_init(NULL);
    intp->env = intp->global_env;

    struct lexer_token_value clock;
    clock.callable_value.arity = 0;
    clock.callable_value.call = native_clock_fun;

    env_define(intp->global_env, sv_from_cstr("clock"), clock);

    return intp;
}

void interpreter_destroy(struct Interpreter* intp)
{
    env_destroy(intp->env);
    free(intp);
}

struct Error* interpret(struct Interpreter* intp, Stmts* stmts)
{
    struct Error* error = NULL;

    for (size_t i = 0; i < stmts->count; i++) {
        if (has_error(execute(intp, stmts->items[i]))) {
            return trace(error);
        }
    }

    return NULL;
}
