#pragma once
#include "expression.h"
#include "statement.h"

struct Interpreter {
    struct Enviroment* env;
};

struct Interpreter* interpreter_init();
void interpreter_destroy(struct Interpreter* intp);

void interpret(struct Interpreter* intp, Stmts* stmts);
