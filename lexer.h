#pragma once

#include "libs/string.h"

#define lex_loc_fmt(t)      t.loc.file_path, t.loc.row, t.loc.col
#define lex_loc_fmt_ptr(t)  t->loc.file_path, t->loc.row, t->loc.col

struct Interpreter;

typedef struct {
    size_t count;
    size_t capacity;
    struct lexer_token_value* items;
} Arguments;

typedef struct {
    const char *opening;
    const char *closing;
} multi_line_comments;

typedef struct {
    const char *file_path;
    size_t row, col;
} location;

typedef enum {
    LEXER_INVALID,
    LEXER_END,
    LEXER_VALUE,
    LEXER_SYMBOL,
    LEXER_KEYWORD,
    LEXER_PUNCT,
    LEXER_NEWLINE,
    LEXER_COUNT_KINDS,
} lexer_token_kind;

typedef enum {
    VALUE_TYPE_INT,
    VALUE_TYPE_INT_LONG_LONG,
    VALUE_TYPE_STRING,
    VALUE_TYPE_CALLABLE,
} lexer_token_value_type;

struct callable_value {
    int arity;
    struct Error* (*call)(struct callable_value value, struct Interpreter* intp, Arguments* args, struct lexer_token_value* result);
    struct Stmt* declaration;
    struct Enviroment* closure;
};

struct lexer_token_value {
    lexer_token_value_type type;
    union {
        int int_value;
        long long int_long_long_value;
        string_view string_value;
        struct callable_value callable_value;
    };
};

typedef struct {
    lexer_token_kind id;
    string_view lexeme;
    struct lexer_token_value value;
    location loc;
} lexer_token;

typedef struct {
    size_t count;
    size_t capacity;
    lexer_token* items;
} LexerTokens;

typedef struct {
    string_view source;
    size_t current, beginning_of_line, row;
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

// Function declarations
lexer lexer_create(const char *file_path, string_view content);
bool lex_get_token(lexer *l, lexer_token *t);
bool lexer_expect(lexer_token t, lexer_token_kind kind);
void print_token_error(const lexer_token *t, const char *fmt, ...);
