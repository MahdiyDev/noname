#pragma once

#include "libs/string.h"
#include <stdio.h>

#define lex_loc_fmt(t) t->loc.file_path, t->loc.row, t->loc.col

typedef struct {
    const char *opening;
    const char *closing;
} multi_line_comments;

typedef struct {
    const char* file_path;
    size_t row;
    size_t col;
} location;

typedef enum {
    LEXER_INVALID,
    LEXER_END,
    LEXER_INT,
    LEXER_SYMBOL,
    LEXER_KEYWORD,
    LEXER_PUNCT,
    LEXER_NEWLINE,
    LEXER_COUNT_KINDS,
} lexer_token_kind;

typedef struct {
    lexer_token_kind id;
    string_view lexeme;

    union {
        int int_value;
    };

    location loc;
} lexer_token;

typedef struct {
    string_view source;

    size_t current;
    size_t beginning_of_line;
    size_t row;

    const char **puncts;
    size_t puncts_count;
    const char **keywords;
    size_t keywords_count;
    const char **sl_comments;
    size_t sl_comments_count;
    const multi_line_comments *ml_comments;
    size_t ml_comments_count;

    const char *file_path;

    lexer_token temp_token; 
} lexer;

lexer lexer_create(const char *file_path, string_view content);

bool lex_get_token(lexer* l, lexer_token* t);
bool lexer_expect(lexer_token t, lexer_token_kind kind);
void print_token_error(const lexer_token* t, const char* fmt, ...);
