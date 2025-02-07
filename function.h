#pragma once

#include "lexer.h"

struct lexer_token_value create_function(struct Stmt* declaration);

struct Error* native_clock_fun(struct callable_value value, struct Interpreter* intp, Arguments* args, struct lexer_token_value* result);
