#pragma once

#include "statement.h"

struct Interpreter {
    struct Enviroment* global_env;
    struct Enviroment* env;
};

struct Interpreter* interpreter_init();
void interpreter_destroy(struct Interpreter* intp);

struct Error* interpret(struct Interpreter* intp, Stmts* stmts);

struct Error* execute_block(struct Interpreter* intp, Stmts* stmts, struct Enviroment* env);
