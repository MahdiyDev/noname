#include <stdio.h>
#include <stdlib.h>
#include "libs/string.h"
#include "libs/temp_alloc.h"
#include "string.h"
#include "lexer.h"
#include "expression.h"

// Helper function to create a string from an expression
void accept(string_builder* sb, struct Expr* expr);

string_view parenthesize(string_builder* sb, string_view name, struct Expr* exprs, int exprs_count)
{
    sb_add_cstr(sb, "(");
    sb_add(sb, name);
    for (int i = 0; i < exprs_count; i++) {
        sb_add_cstr(sb, " ");
        accept(sb, &exprs[i]);
    }
    sb_add_cstr(sb, ")");

    return sb_to_sv(sb);
}

void accept(string_builder* sb, struct Expr* expr)
{
    switch (expr->type) {
        case EXPR_BINARY:
            parenthesize(sb, expr->binary.operator.lexeme, to_c_array(struct Expr, *expr->binary.left, *expr->binary.right), 2);
            break;
        case EXPR_UNARY:
            parenthesize(sb, expr->unary.operator.lexeme, to_c_array(struct Expr, *expr->unary.right), 1);
            break;
        case EXPR_GROUP:
            parenthesize(sb, sv_from_cstr("group"), to_c_array(struct Expr, *expr->group.expression), 1);
            break;
        case EXPR_LITERAL: {
            if (!expr->literal.value) {
                sb_add_cstr(sb, "nil");
            } else {
                sb_add(sb, sv_from_digit(expr->literal.value));
            }
            break;
        }
        default:
            sb_add_cstr(sb, "Unknown Expression");
            break;
    }
}

// Factory functions for creating expressions
struct Expr* create_binary_expr(struct Expr* left, lexer_token operator, struct Expr* right)
{
    struct Expr* expr = temp_alloc(sizeof(struct Expr));
    expr->type = EXPR_BINARY;
    expr->binary.left = left;
    expr->binary.operator = operator;
    expr->binary.right = right;
    return expr;
}

struct Expr* create_unary_expr(lexer_token operator, struct Expr* right)
{
    struct Expr* expr = temp_alloc(sizeof(struct Expr));
    expr->type = EXPR_UNARY;
    expr->unary.operator = operator;
    expr->unary.right = right;
    return expr;
}

struct Expr* create_literal_expr(int value)
{
    struct Expr* expr = temp_alloc(sizeof(struct Expr));
    expr->type = EXPR_LITERAL;
    expr->literal.value = value;
    return expr;
}

struct Expr* create_group_expr(struct Expr* expression)
{
    struct Expr* expr = temp_alloc(sizeof(struct Expr));
    expr->type = EXPR_GROUP;
    expr->group.expression = expression;
    return expr;
}

struct Expr* create_variable_expr(lexer_token name)
{
    struct Expr* expr = temp_alloc(sizeof(struct Expr));
    expr->type = EXPR_VAR;
    expr->variable.name = name;
    return expr;
}

struct Expr* create_assign_expr(lexer_token name, struct Expr* value)
{
    struct Expr* expr = temp_alloc(sizeof(struct Expr));
    expr->type = EXPR_ASSIGN;
    expr->assign.name = name;
    expr->assign.value = value;
    return expr;
}

struct Expr* create_logical_expr(struct Expr* left, lexer_token operator, struct Expr* right)
{
    struct Expr* expr = temp_alloc(sizeof(struct Expr));
    expr->type = EXPR_LOGICAL;
    expr->logical.left = left;
    expr->logical.operator = operator;
    expr->logical.right = right;
    return expr;
}

lexer_token create_operator(const char* sign)
{
    return (lexer_token){ .lexeme = sv_from_cstr(sign) };
}

// Free memory allocated for an expression
void free_expr(struct Expr* expr)
{
    if (!expr) return;
    switch (expr->type) {
    case EXPR_BINARY:
        free_expr(expr->binary.left);
        free_expr(expr->binary.right);
        break;
    case EXPR_UNARY:
        free_expr(expr->unary.right);
        break;
    case EXPR_GROUP:
        free_expr(expr->group.expression);
        break;
    case EXPR_ASSIGN:
        free_expr(expr->assign.value);
        break;
    case EXPR_LOGICAL:
        free_expr(expr->logical.left);
        free_expr(expr->logical.right);
        break;
    case EXPR_VAR:
        break; // No dynamic memory in variable
    case EXPR_LITERAL:
        break; // No dynamic memory in literals
    }
    temp_free(expr);
}

void print_expr(struct Expr* expr)
{
    string_builder* ast_sb = sb_init(NULL);
    accept(ast_sb, expr);

    string_view ast = sb_to_sv(ast_sb);

    printf("AST: %.*s\n", sv_fmt(ast));

    sb_free(ast_sb);
}
