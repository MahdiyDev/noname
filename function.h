#pragma once

#include "lexer.h"

struct lexer_token_value create_function(struct Stmt* declaration, struct Enviroment* closure);
void init_native_functions(struct Interpreter* intp);

struct Error* native_clock_fun(struct callable_value value, struct Interpreter* intp, Arguments* args, struct lexer_token_value* result);
struct Error* native_print_fun(struct callable_value value, struct Interpreter* intp, Arguments* args, struct lexer_token_value* _);
struct Error* native_println_fun(struct callable_value value, struct Interpreter* intp, Arguments* args, struct lexer_token_value* _);
