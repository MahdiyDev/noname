#pragma once

#include "lexer.h"

typedef enum {
    EXPR_BINARY,
    EXPR_UNARY,
    EXPR_GROUP,
    EXPR_LITERAL,
    EXPR_VAR,
} ExprType;

struct Expr {
    ExprType type;
    union {
        struct {
            struct Expr* left;
            lexer_token operator;
            struct Expr* right;
        } binary;

        struct {
            lexer_token operator;
            struct Expr* right;
        } unary;

        struct {
            int value;
        } literal;

        struct {
            struct Expr* expression;
        } group;

        struct {
            lexer_token name;
        } variable;
    };
};

struct Expr* create_binary_expr(struct Expr* left, lexer_token operator, struct Expr* right);
struct Expr* create_unary_expr(lexer_token operator, struct Expr* right);
struct Expr* create_literal_expr(int value);
struct Expr* create_group_expr(struct Expr* expression);
struct Expr* create_variable_expr(lexer_token name);

void print_expr(struct Expr* expr);
void free_expr(struct Expr* expr);
