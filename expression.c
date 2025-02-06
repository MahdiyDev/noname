#include <stdio.h>
#include "libs/string.h"
#include "libs/temp_alloc.h"
#include "lexer.h"
#include "expression.h"

// Factory functions for creating expressions
struct Expr* create_binary_expr(temp_allocator allocator, struct Expr* left, lexer_token operator, struct Expr* right)
{
    struct Expr* expr = temp_alloc(allocator, sizeof(struct Expr));
    expr->type = EXPR_BINARY;
    expr->binary.left = left;
    expr->binary.operator = operator;
    expr->binary.right = right;
    return expr;
}

struct Expr* create_unary_expr(temp_allocator allocator, lexer_token operator, struct Expr* right)
{
    struct Expr* expr = temp_alloc(allocator, sizeof(struct Expr));
    expr->type = EXPR_UNARY;
    expr->unary.operator = operator;
    expr->unary.right = right;
    return expr;
}

struct Expr* create_literal_expr(temp_allocator allocator, lexer_token_value value)
{
    struct Expr* expr = temp_alloc(allocator, sizeof(struct Expr));
    expr->type = EXPR_LITERAL;
    expr->literal.value = value;
    return expr;
}

struct Expr* create_group_expr(temp_allocator allocator, struct Expr* expression)
{
    struct Expr* expr = temp_alloc(allocator, sizeof(struct Expr));
    expr->type = EXPR_GROUP;
    expr->group.expression = expression;
    return expr;
}

struct Expr* create_variable_expr(temp_allocator allocator, lexer_token name)
{
    struct Expr* expr = temp_alloc(allocator, sizeof(struct Expr));
    expr->type = EXPR_VAR;
    expr->variable.name = name;
    return expr;
}

struct Expr* create_assign_expr(temp_allocator allocator, lexer_token name, struct Expr* value)
{
    struct Expr* expr = temp_alloc(allocator, sizeof(struct Expr));
    expr->type = EXPR_ASSIGN;
    expr->assign.name = name;
    expr->assign.value = value;
    return expr;
}

struct Expr* create_logical_expr(temp_allocator allocator, struct Expr* left, lexer_token operator, struct Expr* right)
{
    struct Expr* expr = temp_alloc(allocator, sizeof(struct Expr));
    expr->type = EXPR_LOGICAL;
    expr->logical.left = left;
    expr->logical.operator = operator;
    expr->logical.right = right;
    return expr;
}

struct Expr* create_call_expr(temp_allocator allocator, struct Expr* calle, lexer_token paren, Exprs* arguments)
{
    struct Expr* expr = temp_alloc(allocator, sizeof(struct Expr));
    expr->type = EXPR_CALL;
    expr->call.calle = calle;
    expr->call.paren = paren;
    expr->call.arguments = arguments;
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
    case EXPR_CALL:
        free_expr(expr->call.calle);
        da_free(expr->call.arguments);
        break;
    case EXPR_VAR:
        break; // No dynamic memory in variable
    case EXPR_LITERAL:
        break; // No dynamic memory in literals
    }
    temp_free(expr);
}

void print_indent(int indent_level)
{
    for (int i = 0; i < indent_level; i++) {
        printf("  ");
    }
}

void print_expression(struct Expr* expr, int indent_level)
{
    if (!expr) return;
    print_indent(indent_level);
    
    switch (expr->type) {
    case EXPR_BINARY:
        printf("Binary Expression: %.*s\n", sv_fmt(expr->binary.operator.lexeme));
        print_expression(expr->binary.left, indent_level + 1);
        print_expression(expr->binary.right, indent_level + 1);
        break;
    case EXPR_UNARY:
        printf("Unary Expression: %.*s\n", sv_fmt(expr->unary.operator.lexeme));
        print_expression(expr->unary.right, indent_level + 1);
        break;
    case EXPR_GROUP:
        printf("Grouping Expression:\n");
        print_expression(expr->group.expression, indent_level + 1);
        break;
    case EXPR_LITERAL:
        printf("Literal: ");
        switch (expr->literal.value.type) {
        case VALUE_TYPE_INT:
            printf("%d\n", expr->literal.value.int_value);
            break;
        case VALUE_TYPE_STRING:
            printf("\"%.*s\"\n", sv_fmt(expr->literal.value.string_value));
            break;
        }
        break;
    case EXPR_VAR:
        printf("Variable: %.*s\n",sv_fmt(expr->variable.name.lexeme));
        break;
    case EXPR_ASSIGN:
        printf("Assignment: %.*s\n", sv_fmt(expr->assign.name.lexeme));
        print_expression(expr->assign.value, indent_level + 1);
        break;
    case EXPR_LOGICAL:
        printf("Logical Expression: %.*s\n", sv_fmt(expr->logical.operator.lexeme));
        print_expression(expr->logical.left, indent_level + 1);
        print_expression(expr->logical.right, indent_level + 1);
        break;
    case EXPR_CALL:
        printf("Call Expression: %.*s\n", sv_fmt(expr->call.paren.lexeme));
        print_expression(expr->call.calle, indent_level + 1);
        for (size_t i = 0; i < expr->call.arguments->count; i++) {
            print_expression(expr->call.arguments->items[i], indent_level + 1);
        }
    default:
        printf("Unknown Expression\n");
        break;
    }
}
