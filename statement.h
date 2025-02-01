#pragma once

#include "expression.h"

typedef enum {
    STMT_EXPRESSION,
    STMT_PRINT,
    STMT_VAR,
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
    };
};

typedef struct {
    size_t count;
    size_t capacity;
    struct Stmt** items;
} Stmts;

struct Stmt* create_expression_stmt(struct Expr* expression);
struct Stmt* create_print_stmt(struct Expr* expression);
struct Stmt* create_variable_stmt(lexer_token name, struct Expr* initializer);

void free_stmt(struct Stmt* stmt);
