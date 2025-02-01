#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdarg.h>

#include "lexer.h"

#define is_newline(c) (c == '\n' || c == '\r')

location create_location(lexer *l)
{
    return (location) {
        .file_path = l->file_path,
        .row = l->row + 1,
        .col = l->current - l->beginning_of_line + 1,
    };
}

bool lex_is_symbol(char c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
            c == '_';
}

bool lex_is_space(char c)
{
    return c == ' ' || c <= '\t';
}

bool lex_advance(lexer* l)
{
    if (l->current < l->source.count) {
        char current_c = l->source.data[l->current];
        l->current++;
        if (is_newline(current_c)) {
            l->row += 1;
            l->beginning_of_line = l->current;
        }
        return true;
    }
    return false;
}

void lex_advance_until(lexer *l, size_t n)
{
    while (n --> 0 && lex_advance(l));
}

void lex_advance_until_endline(lexer *l)
{
    while (l->current < l->source.count) {
        char x = l->source.data[l->current];
        lex_advance(l);
        if (x == '\n') break;
    }
}

void lex_advance_until_prefix(lexer *l, const char *prefix)
{
    string_view content = sv_from_parts(l->source.data + l->current, l->source.count);
    while (l->current < l->source.count && !sv_start_with(content, prefix)) {
        content = sv_from_parts(l->source.data + l->current, l->source.count);
        lex_advance(l);
    }
}

void lex_trim_left(lexer *l)
{
    while (l->current < l->source.count && lex_is_space(l->source.data[l->current])) {
        lex_advance(l);
    }
}

bool lex_get_token(lexer* l, lexer_token* t)
{
    string_view current_str = sv_from_parts(l->source.data + l->current, l->source.count);

another_trim_round:
    while (l->current < l->source.count) {
        lex_trim_left(l);
        current_str = sv_from_parts(l->source.data + l->current, l->source.count);

        // Single line comment
        for (size_t i = 0; i < l->sl_comments_count; ++i) {
            if (sv_start_with(current_str, l->sl_comments[i])) {
                lex_advance_until_endline(l);
                goto another_trim_round;
            }
        }

        // Multi line comment
        for (size_t i = 0; i < l->ml_comments_count; ++i) {
            const char *opening = l->ml_comments[i].opening;
            const char *closing = l->ml_comments[i].closing;
            if (sv_start_with(current_str, opening)) {
                lex_advance_until(l, strlen(opening));
                lex_advance_until_prefix(l, closing);
                lex_advance_until(l, strlen(closing));
                goto another_trim_round;
            }
        }

        break;
    }

    memset(t, 0, sizeof(*t));

    t->loc = create_location(l);

    if (l->current >= l->source.count) {
        t->id = LEXER_END;
        return false;
    }

    if (is_newline(current_str.data[0])) {
        t->id = LEXER_NEWLINE;
        lex_advance(l);
        return true;
    }

    // Puncts
    for (int i = 0; i < l->puncts_count; i++) {
        if (sv_start_with(current_str, l->puncts[i])) {
            t->id = LEXER_PUNCT;
            size_t n = strlen(l->puncts[i]);
            t->lexeme = sv_from_parts(current_str.data, n);
            lex_advance_until(l, n);
            return true;
        }
    }

    // Int
    if (sv_isdigit(l->source.data[l->current])) {
        size_t n = 0;
        while (l->current < l->source.count && sv_isdigit(l->source.data[l->current])) {
            t->int_value = t->int_value*10 + l->source.data[l->current] - '0';
            n += 1;
            lex_advance(l);
        }

        t->id = LEXER_INT;
        t->lexeme = sv_from_parts(l->source.data + l->current, n);
        
        return true;
    }

    // Symbol
    if (lex_is_symbol(l->source.data[l->current])) {
        t->id = LEXER_SYMBOL;
        size_t n = 0;
        size_t begin = l->current;
        while (l->current < l->source.count && (lex_is_symbol(l->source.data[l->current]) || sv_isdigit(l->source.data[l->current]))) {
            n += 1;
            lex_advance(l);
        }
        t->lexeme = sv_from_parts(l->source.data + begin, n);

        // Keyword
        for (int i = 0; i < l->keywords_count; i++) {
            if (sv_equal_cstr(t->lexeme, l->keywords[i])) {
                t->id = LEXER_KEYWORD;
                break;
            }
        }

        return true;
    }

    lex_advance(l);
    t->lexeme = sv_from_parts(l->source.data + l->current - 1, 1);

    return false;
}

bool lexer_expect(lexer_token t, lexer_token_kind kind)
{
    return t.id == kind;
}

lexer lexer_create(const char *file_path, string_view content)
{
    return (lexer) {
        .file_path = file_path,
        .source = content,
    };
}

const char *lexer_kind_names[LEXER_COUNT_KINDS] = {
    [LEXER_INVALID] = "INVALID",
    [LEXER_INT]     = "INT",
    [LEXER_END]     = "END",
    [LEXER_SYMBOL]  = "SYMBOL",
    [LEXER_KEYWORD] = "KEYWORD",
    [LEXER_PUNCT]   = "PUNCT",
    [LEXER_NEWLINE] = "NEWLINE",
};

void print_token_error(const lexer_token *t, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "Error: ");
    vfprintf(stderr, fmt, args);

    fprintf(stderr, " at %s:%zu:%zu (kind: %s, value: %d, lexeme: '%.*s')\n",
        t->loc.file_path,
        t->loc.row,
        t->loc.col,
        lexer_kind_names[t->id],
        t->int_value,
        sv_fmt(t->lexeme));

    va_end(args);
}

