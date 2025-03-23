#include "vm.h"
#include "../libs/string.h"

#include <errno.h>

static void repl(VM* vm)
{
    char line[1024];
    for (;;) {
        printf("> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        interpret(vm, line);
    }
}

static void run_file(VM* vm, const char* path)
{
    string_builder source = sb_init(NULL);
    if (!sb_read_file(&source, path)) {
        fprintf(stderr, "Could not read file %s: %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    InterpretResult result = interpret(vm, source.items);
    sb_free(&source);

    if (result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}

int main(int argc, char** argv)
{
    VM vm = init_vm();

    if (argc == 1) {
        repl(&vm);
    } else if (argc == 2) {
        run_file(&vm, argv[1]);
    } else {
        fprintf(stderr, "Usage: vm.out [path]\n");
        exit(EXIT_FAILURE);
    }

    free_vm(&vm);
    return 0;
}