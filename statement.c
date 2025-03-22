#include "libs/temp_alloc.h"
#include "statement.h"
#include "expression.h"

struct Stmt* create_expression_stmt(temp_allocator allocator, struct Expr* expression)
{
    struct Stmt* stmt = temp_alloc(allocator, sizeof(struct Stmt));
    stmt->type = STMT_EXPRESSION;
    stmt->expression.expression = expression;
    return stmt;
}

struct Stmt* create_variable_stmt(temp_allocator allocator, lexer_token name, struct Expr* initializer)
{
    struct Stmt* stmt = temp_alloc(allocator, sizeof(struct Stmt));
    stmt->type = STMT_VAR;
    stmt->variable.name = name;
    stmt->variable.initializer = initializer;
    return stmt;
}

struct Stmt* create_block_stmt(temp_allocator allocator, Stmts* statements)
{
    struct Stmt* stmt = temp_alloc(allocator, sizeof(struct Stmt));
    stmt->type = STMT_BLOCK;
    stmt->block.statements = statements;
    return stmt;
}

struct Stmt* create_if_stmt(temp_allocator allocator, struct Expr* condition, struct Stmt* then_branch, struct Stmt* else_branch)
{
    struct Stmt* stmt = temp_alloc(allocator, sizeof(struct Stmt));
    stmt->type = STMT_IF;
    stmt->if_stmt.condition = condition;
    stmt->if_stmt.then_branch = then_branch;
    stmt->if_stmt.else_branch = else_branch;
    return stmt;
}

struct Stmt* create_while_stmt(temp_allocator allocator, struct Expr* condition, struct Stmt* body)
{
    struct Stmt* stmt = temp_alloc(allocator, sizeof(struct Stmt));
    stmt->type = STMT_WHILE;
    stmt->while_stmt.condition = condition;
    stmt->while_stmt.body = body;
    return stmt;
}

struct Stmt* create_function_stmt(temp_allocator allocator, lexer_token name, LexerTokens* params, Stmts* body)
{
    struct Stmt* stmt = temp_alloc(allocator, sizeof(struct Stmt));
    stmt->type = STMT_FUNCTION;
    stmt->function_stmt.name = name;
    stmt->function_stmt.params = params;
    stmt->function_stmt.body = body;
    return stmt;
}

struct Stmt* create_return_stmt(temp_allocator allocator, lexer_token keyword, struct Expr* value)
{
    struct Stmt* stmt = temp_alloc(allocator, sizeof(struct Stmt));
    stmt->type = STMT_RETURN;
    stmt->return_stmt.keyword = keyword;
    stmt->return_stmt.value = value;
    return stmt;
}

void free_stmt(struct Stmt* stmt)
{
    if (!stmt) return;

    switch (stmt->type) {
    case STMT_EXPRESSION:
        free_expr(stmt->expression.expression);
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
    case STMT_FUNCTION:
        da_free(stmt->function_stmt.params);
        da_free(stmt->function_stmt.body);
        break;
    case STMT_RETURN:
        free_expr(stmt->return_stmt.value);
        break;
    }

    temp_free(stmt);
}

void print_statement(struct Stmt* stmt, int indent_level)
{
    if (!stmt) return;
    
    print_indent(indent_level);
    
    switch (stmt->type) {
    case STMT_EXPRESSION:
        printf("Expression Statement:\n");
        print_expression(stmt->expression.expression, indent_level + 1);
        break;
    case STMT_VAR:
        printf("Variable Declaration: %.*s\n", sv_fmt(stmt->variable.name.lexeme));
        if (stmt->variable.initializer) {
            print_indent(indent_level + 1);
            printf("Initializer:\n");
            print_expression(stmt->variable.initializer, indent_level + 2);
        }
        break;
    case STMT_BLOCK:
        printf("Block Statement:\n");
        for (size_t i = 0; i < stmt->block.statements->count; i++) {
            print_statement(stmt->block.statements->items[i], indent_level + 1);
        }
        break;
    case STMT_IF:
        printf("If Statement:\n");
        print_indent(indent_level + 1);
        printf("Condition:\n");
        print_expression(stmt->if_stmt.condition, indent_level + 2);
        print_indent(indent_level + 1);
        printf("Then Branch:\n");
        print_statement(stmt->if_stmt.then_branch, indent_level + 2);
        if (stmt->if_stmt.else_branch) {
            print_indent(indent_level + 1);
            printf("Else Branch:\n");
            print_statement(stmt->if_stmt.else_branch, indent_level + 2);
        }
        break;
    case STMT_WHILE:
        printf("While Statement:\n");
        print_indent(indent_level + 1);
        printf("Condition:\n");
        print_expression(stmt->while_stmt.condition, indent_level + 2);
        print_indent(indent_level + 1);
        printf("Body:\n");
        print_statement(stmt->while_stmt.body, indent_level + 2);
        break;
    case STMT_FUNCTION:
        printf("Function Statement: %.*s\n", sv_fmt(stmt->function_stmt.name.lexeme));
        print_indent(indent_level + 1);
        printf("Parameters: ");
        for (int i = 0; i < stmt->function_stmt.params->count; i++) {
            printf("(%d): %.*s ", i, sv_fmt(stmt->function_stmt.params->items[i].lexeme));
        }
        printf("\n");
        print_indent(indent_level + 1);
        printf("Body:\n");
        for (int i = 0; i < stmt->function_stmt.body->count; i++) {
            print_statement(stmt->function_stmt.body->items[i], indent_level + 2);
        }
        break;
    case STMT_RETURN:
        printf("Return Statement\n");
        print_expression(stmt->return_stmt.value, indent_level + 2);
        break;
    default:
        printf("Unknown Statement Type: %d\n", stmt->type);
        break;
    }
}
