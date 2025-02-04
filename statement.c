#include "statement.h"
#include "expression.h"

struct Stmt* create_expression_stmt(struct Expr* expression)
{
    struct Stmt* stmt = malloc(sizeof(struct Stmt));
    stmt->type = STMT_EXPRESSION;
    stmt->expression.expression = expression;
    return stmt;
}

struct Stmt* create_print_stmt(struct Expr* expression)
{
    struct Stmt* stmt = malloc(sizeof(struct Stmt));
    stmt->type = STMT_PRINT;
    stmt->print.expression = expression;
    return stmt;
}

struct Stmt* create_variable_stmt(lexer_token name, struct Expr* initializer)
{
    struct Stmt* stmt = malloc(sizeof(struct Stmt));
    stmt->type = STMT_VAR;
    stmt->variable.name = name;
    stmt->variable.initializer = initializer;
    return stmt;
}

struct Stmt* create_block_stmt(Stmts* statements)
{
    struct Stmt* stmt = malloc(sizeof(struct Stmt));
    stmt->type = STMT_BLOCK;
    stmt->block.statements = statements;
    return stmt;
}

struct Stmt* create_if_stmt(struct Expr* condition, struct Stmt* then_branch, struct Stmt* else_branch)
{
    struct Stmt* stmt = malloc(sizeof(struct Stmt));
    stmt->type = STMT_IF;
    stmt->if_stmt.condition = condition;
    stmt->if_stmt.then_branch = then_branch;
    stmt->if_stmt.else_branch = else_branch;
    return stmt;
}

struct Stmt* create_while_stmt(struct Expr* condition, struct Stmt* body)
{
    struct Stmt* stmt = malloc(sizeof(struct Stmt));
    stmt->type = STMT_WHILE;
    stmt->while_stmt.condition = condition;
    stmt->while_stmt.body = body;
    return stmt;
}

void free_stmt(struct Stmt* stmt)
{
    if (!stmt) return;

    switch (stmt->type) {
    case STMT_EXPRESSION:
        free_expr(stmt->expression.expression);
        break;
    case STMT_PRINT:
        free_expr(stmt->print.expression);
        break;
    case STMT_VAR:
        free_expr(stmt->variable.initializer);
        break;
    case STMT_BLOCK:
        da_free(stmt->block.statements);
        break;
    case STMT_IF:
        free_expr(stmt->if_stmt.condition);
        free_stmt(stmt->if_stmt.then_branch);
        free_stmt(stmt->if_stmt.else_branch);
        break;
    case STMT_WHILE:
        free_expr(stmt->while_stmt.condition);
        free_stmt(stmt->while_stmt.body);
        break;
    }

    free(stmt);
}