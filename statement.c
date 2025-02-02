#include "statement.h"

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
        }
    free(stmt);
}