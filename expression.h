#pragma once

#include "lexer.h"
#include "libs/temp_alloc.h"

typedef struct {
    size_t count;
    size_t capacity;
    struct Expr** items;
} Exprs;

typedef enum {
    EXPR_BINARY,
    EXPR_UNARY,
    EXPR_GROUP,
    EXPR_LITERAL,
    EXPR_VAR,
    EXPR_ASSIGN,
    EXPR_LOGICAL,
    EXPR_CALL,
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
            lexer_token_value value;
        } literal;

        struct {
            struct Expr* expression;
        } group;

        struct {
            lexer_token name;
        } variable;

        struct {
            lexer_token name;
            struct Expr* value;
        } assign;

        struct {
            struct Expr* left;
            lexer_token operator;
            struct Expr* right;
        } logical;

        struct {
            struct Expr* calle;
            lexer_token paren;
            Exprs* arguments;
        } call;
    };
};

struct Expr* create_literal_expr(temp_allocator allocator, lexer_token_value value);
struct Expr* create_binary_expr(temp_allocator allocator, struct Expr* left, lexer_token operator, struct Expr* right);
struct Expr* create_unary_expr(temp_allocator allocator, lexer_token operator, struct Expr* right);
struct Expr* create_group_expr(temp_allocator allocator, struct Expr* expression);
struct Expr* create_variable_expr(temp_allocator allocator, lexer_token name);
struct Expr* create_assign_expr(temp_allocator allocator, lexer_token name, struct Expr* value);
struct Expr* create_logical_expr(temp_allocator allocator, struct Expr* left, lexer_token operator, struct Expr* right);
struct Expr* create_call_expr(temp_allocator allocator, struct Expr* calle, lexer_token paren, Exprs* arguments);

void print_indent(int indent_level);
void print_expression(struct Expr* expr, int indent_level);
void free_expr(struct Expr* expr);
