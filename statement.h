#pragma once

#include "expression.h"

typedef struct {
    size_t count;
    size_t capacity;
    struct Stmt** items;
} Stmts;

typedef enum {
    STMT_EXPRESSION,
    STMT_PRINT,
    STMT_VAR,
    STMT_BLOCK,
    STMT_IF,
    STMT_WHILE,
} StmtType;

struct Stmt {
    StmtType type;

    union {
        struct {
            struct Expr* expression;
        } expression;

        struct {
            struct Expr* expression;
        } print;

        struct {
            lexer_token name;
            struct Expr* initializer;
        } variable;

        struct {
            Stmts* statements;
        } block;

        struct {
            struct Expr* condition;
            struct Stmt* then_branch;
            struct Stmt* else_branch;
        } if_stmt;

        struct {
            struct Expr* condition;
            struct Stmt* body;
        } while_stmt;
    };
};

struct Stmt* create_expression_stmt(struct Expr* expression);
struct Stmt* create_print_stmt(struct Expr* expression);
struct Stmt* create_variable_stmt(lexer_token name, struct Expr* initializer);
struct Stmt* create_block_stmt(Stmts* statements);
struct Stmt* create_if_stmt(struct Expr* condition, struct Stmt* then_branch, struct Stmt* else_branch);
struct Stmt* create_while_stmt(struct Expr* condition, struct Stmt* body);

void free_stmt(struct Stmt* stmt);
