#pragma once

#include "statement.h"

typedef enum {
    ERROR_RETURN,
} ErrorType;

struct Interpreter {
    struct Enviroment* global_env;
    struct Enviroment* env;
    temp_allocator allocator;
};

struct Interpreter* interpreter_init();
void interpreter_destroy(struct Interpreter* intp);

struct Error* interpret(struct Interpreter* intp, Stmts* stmts);

struct Error* evaluate(struct Interpreter* intp, struct Expr* expr, struct lexer_token_value* result);
struct Error* execute_block(struct Interpreter* intp, Stmts* stmts, struct Enviroment* env, struct lexer_token_value* return_value);
