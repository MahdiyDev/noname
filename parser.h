#pragma once

#include "libs/temp_alloc.h"
#include "statement.h"
#include "lexer.h"

struct Parser {
    temp_allocator allocator;
    lexer* lexer;
    lexer_token* token;
};

struct Error* parse(struct Parser* parser, Stmts* result);
