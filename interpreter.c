#include "interpreter.h"
#include "libs/string.h"
#include "statement.h"
#include "environment.h"

#define UNREACHABLE() { fprintf(stderr, "%s:%d\n", __FILE__, __LINE__); abort();}

int evaluate(struct Interpreter* intp, struct Expr* expr);

int visit_literal_expr(struct Expr* expr)
{
    return expr->literal.value;
}

int visit_group_expr(struct Interpreter* intp, struct Expr* expr)
{
    return evaluate(intp, expr->group.expression);
}

bool is_truthy(int value)
{
    return value == true;
}

int visit_unary_expr(struct Interpreter* intp, struct Expr* expr)
{
    int right = evaluate(intp, expr->unary.right);

    if (sv_equal_cstr(expr->unary.operator.lexeme, "-")) {
        return -right;
    }
    else if (sv_equal_cstr(expr->unary.operator.lexeme, "!")) {
        // TODO: change this to boolean value
        return !is_truthy(right);
    }

    UNREACHABLE();
}

int visit_binary_expr(struct Interpreter* intp, struct Expr* expr)
{
    int left = evaluate(intp, expr->binary.left);
    int right = evaluate(intp, expr->binary.right);

    if (sv_equal_cstr(expr->binary.operator.lexeme, "==")) {
        return left == right;
    }
    else if (sv_equal_cstr(expr->binary.operator.lexeme, "!=")) {
        return left != right;
    }
    else if (sv_equal_cstr(expr->binary.operator.lexeme, ">=")) {
        return left >= right;
    }
    else if (sv_equal_cstr(expr->binary.operator.lexeme, ">")) {
        return left > right;
    }
    else if (sv_equal_cstr(expr->binary.operator.lexeme, "<=")) {
        return left <= right;
    }
    else if (sv_equal_cstr(expr->binary.operator.lexeme, "<")) {
        return left < right;
    }
    else if (sv_equal_cstr(expr->binary.operator.lexeme, "-")) {
        return left - right;
    }
    else if (sv_equal_cstr(expr->binary.operator.lexeme, "+")) {
        return left + right;
    }
    else if (sv_equal_cstr(expr->binary.operator.lexeme, "/")) {
        return left / right;
    }
    else if (sv_equal_cstr(expr->binary.operator.lexeme, "*")) {
        return left * right;
    }

    UNREACHABLE();
}

int visit_variable_expr(struct Interpreter* intp, struct Expr* expr)
{
    int value = 0;
    env_get(intp->env, expr->variable.name, &value);
    return value;
}

int evaluate(struct Interpreter* intp, struct Expr* expr)
{
    switch (expr->type) {
    case EXPR_BINARY:
        return visit_binary_expr(intp, expr);
    case EXPR_UNARY:
        return visit_unary_expr(intp, expr);
    case EXPR_GROUP:
        return visit_group_expr(intp, expr);
    case EXPR_LITERAL:
        return visit_literal_expr(expr);
    case EXPR_VAR:
        return visit_variable_expr(intp, expr);
    }

    UNREACHABLE();
}

void visit_expression_stmt(struct Interpreter* intp, struct Stmt* stmt)
{
    evaluate(intp, stmt->expression.expression);
}

void visit_print_stmt(struct Interpreter* intp, struct Stmt* stmt)
{
    int value = evaluate(intp, stmt->print.expression);
    printf("%d\n", value);
}

void visit_variable_stmt(struct Interpreter* intp, struct Stmt* stmt)
{
    int value = 0;
    if (stmt->variable.initializer != NULL) {
        value = evaluate(intp, stmt->variable.initializer);
    }

    env_define(intp->env, stmt->variable.name.lexeme, value);
}

void execute(struct Interpreter* intp, struct Stmt* stmt)
{
    switch (stmt->type) {
    case STMT_EXPRESSION:
        visit_expression_stmt(intp, stmt);
        break;
    case STMT_PRINT:
        visit_print_stmt(intp, stmt);
        break;
    case STMT_VAR:
        visit_variable_stmt(intp, stmt);
        break;
    }
}

struct Interpreter* interpreter_init()
{
    struct Interpreter* intp = malloc(sizeof(struct Interpreter));
    intp->env = env_init();
    return intp;
}

void interpreter_destroy(struct Interpreter* intp)
{
    env_destroy(intp->env);
    free(intp);
}

void interpret(struct Interpreter* intp, Stmts* stmts)
{
    for (size_t i = 0; i < stmts->count; i++) {
        execute(intp, stmts->items[i]);
        free_stmt(stmts->items[i]);
    }
}
