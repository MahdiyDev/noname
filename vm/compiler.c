#include "compiler.h"
#include "scanner.h"
#include "common.h"

void compile(const char* source)
{
    Scanner scanner = init_scanner(source);

    int line = -1;
    for(;;) {
        Token token = scan_token(&scanner);
        if (token.line != line) {
            printf("%4d ", token.line);
            line = token.line;
        } else {
            printf("   | ");
        }
        printf("%2d '%.*s'\n", token.type, token.length, token.start);

        if (token.type == TOKEN_EOF) break;
    }
}
