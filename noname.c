#include "libs/error.h"
#include "interpreter.h"
#include "parser.h"
#include "statement.h"

const char* puncts[] = {
    "(", ")", "{", "}", 
    ",", ".", ";",
    "-", "+", "*", "/",
    ">=", ">", "<=", "<", // Order is matter
};

const char *sl_comments[] = {
    "//",
};

const multi_line_comments ml_comments[] = {
    {"/*", "*/"},
};

int main(int argc, char** argv)
{
    int exit_code = EXIT_SUCCESS;

    if (argc < 2) {
        fprintf(stderr, "usage: %s file\n", argv[0]);
        return EXIT_FAILURE;
    }
    const char* file_path = argv[1];

    string_builder* sb = sb_init(NULL);
    if (!sb_read_file(sb, file_path)) return_defer(exit_code, EXIT_FAILURE);

    lexer l = lexer_create(file_path, sb_to_sv(sb));

    l.puncts = puncts;
    l.puncts_count = arr_count(puncts);
    l.sl_comments = sl_comments;
    l.sl_comments_count = arr_count(sl_comments);
    l.ml_comments = ml_comments;
    l.ml_comments_count = arr_count(ml_comments);

    lexer_token t = {0};

    struct Error* error = NULL;

    Stmts* stmts = NULL;
    da_init(stmts);

    temp_allocator allocator = temp_init();

    struct Parser parser;

    parser.lexer = &l;
    parser.token = &t;
    parser.allocator = allocator;

    if (has_error(parse(&parser, stmts))) {
        print_error(error);

        for (size_t i = 0; i < stmts->count; i++) {
            free_stmt(stmts->items[i]);
        }

        da_free(stmts);
        return_defer(exit_code, EXIT_FAILURE);
    }

    // for (int i = 0; i < stmts->count; i++) {
    //     print_statement(stmts->items[i], 0);
    // }

    struct Interpreter* intp = interpreter_init();

    if (has_error(interpret(intp, stmts))) {
        print_error(error);

        interpreter_destroy(intp);
        da_free(stmts);
        return_defer(exit_code, EXIT_FAILURE);
    }

    interpreter_destroy(intp);

    for (size_t i = 0; i < stmts->count; i++) {
        free_stmt(stmts->items[i]);
    }

    da_free(stmts);

defer:
    sb_free(sb);
    return exit_code;
}