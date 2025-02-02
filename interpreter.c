#include "interpreter.h"
#include "libs/string.h"
#include "statement.h"
#include "environment.h"

#define UNREACHABLE() { fprintf(stderr, "%s:%d\n", __FILE__, __LINE__); abort();}

struct Error* evaluate(struct Interpreter* intp, struct Expr* expr, int* result);
struct Error* execute(struct Interpreter* intp, struct Stmt* stmt);

struct Error* visit_literal_expr(struct Expr* expr, int* result)
{
    *result = expr->literal.value;
    return NULL;
}

struct Error* visit_group_expr(struct Interpreter* intp, struct Expr* expr, int* result)
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

struct Error* visit_unary_expr(struct Interpreter* intp, struct Expr* expr, int* result)
{
    struct Error* error = NULL;

    int right = 0;
    if (has_error(evaluate(intp, expr->unary.right, &right))) {
        return trace(error);
    }

    if (sv_equal_cstr(expr->unary.operator.lexeme, "-")) {
        *result = -right;
        return NULL;
    }
    else if (sv_equal_cstr(expr->unary.operator.lexeme, "!")) {
        // TODO: change this to boolean value
        *result = !is_truthy(right);
        return NULL;
    }

    UNREACHABLE();
}

struct Error* visit_binary_expr(struct Interpreter* intp, struct Expr* expr, int* result)
{
    struct Error* error = NULL;

    int left = 0;
    if (has_error(evaluate(intp, expr->binary.left, &left))) {
        return trace(error);
    }

    int right = 0;
    if (has_error(evaluate(intp, expr->binary.right, &right))) {
        return trace(error);
    }

    if (sv_equal_cstr(expr->binary.operator.lexeme, "==")) {
        *result = left == right;
        return NULL;
    }
    else if (sv_equal_cstr(expr->binary.operator.lexeme, "!=")) {
        *result = left != right;
        return NULL;
    }
    else if (sv_equal_cstr(expr->binary.operator.lexeme, ">=")) {
        *result = left >= right;
        return NULL;
    }
    else if (sv_equal_cstr(expr->binary.operator.lexeme, ">")) {
        *result = left > right;
        return NULL;
    }
    else if (sv_equal_cstr(expr->binary.operator.lexeme, "<=")) {
        *result = left <= right;
        return NULL;
    }
    else if (sv_equal_cstr(expr->binary.operator.lexeme, "<")) {
        *result = left < right;
        return NULL;
    }
    else if (sv_equal_cstr(expr->binary.operator.lexeme, "-")) {
        *result = left - right;
        return NULL;
    }
    else if (sv_equal_cstr(expr->binary.operator.lexeme, "+")) {
        *result = left + right;
        return NULL;
    }
    else if (sv_equal_cstr(expr->binary.operator.lexeme, "/")) {
        *result = left / right;
        return NULL;
    }
    else if (sv_equal_cstr(expr->binary.operator.lexeme, "*")) {
        *result = left * right;
        return NULL;
    }

    UNREACHABLE();
}

struct Error* visit_variable_expr(struct Interpreter* intp, struct Expr* expr, int* result)
{
    struct Error* error = NULL;

    return trace(env_get(intp->env, expr->variable.name, result));
}

struct Error* visit_assign_expr(struct Interpreter* intp, struct Expr* expr, int* result)
{
    struct Error* error = NULL;

    int value = 0;
    if (has_error(evaluate(intp, expr->assign.value, &value))) {
        return trace(error);
    }

    return trace(env_assign(intp->env, expr->assign.name, value));
}

struct Error* evaluate(struct Interpreter* intp, struct Expr* expr, int* result)
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
    }

    UNREACHABLE();
}

struct Error* visit_expression_stmt(struct Interpreter* intp, struct Stmt* stmt)
{
    struct Error* error = NULL;

    int value = 0;
    return trace(evaluate(intp, stmt->expression.expression, &value));
}

struct Error* visit_print_stmt(struct Interpreter* intp, struct Stmt* stmt)
{
    struct Error* error = NULL;

    int value = 0;
    if (has_error(evaluate(intp, stmt->print.expression, &value))) {
        return trace(error);
    }

    printf("%d\n", value);
    return NULL;
}

struct Error* visit_variable_stmt(struct Interpreter* intp, struct Stmt* stmt)
{
    struct Error* error = NULL;

    int value = 0;
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

        free_stmt(stmts->items[i]);
    }

    intp->env = prev_env; // Restore env
    return NULL;
}

struct Error* visit_block_stmt(struct Interpreter* intp, struct Stmt* stmt)
{
    struct Error* error = NULL;

    return trace(execute_block(intp, stmt->block.statements, env_init(intp->env)));
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
    }
}

struct Interpreter* interpreter_init()
{
    struct Interpreter* intp = malloc(sizeof(struct Interpreter));
    intp->env = env_init(NULL);
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

        free_stmt(stmts->items[i]);
    }

    return NULL;
}
