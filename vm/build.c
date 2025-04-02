#define BUILD_IMPLEMENTATION
#include "../libs/build.h"

int main(int argc, char** argv)
{
    Builder builder = { .output_dir = "./build" };

    builder_add_executable(&builder, "vm.out");

    builder_add_source_file(&builder, "vm.c");
    builder_add_source_file(&builder, "main.c");
    builder_add_source_file(&builder, "chunk.c");
    builder_add_source_file(&builder, "debug.c");
    builder_add_source_file(&builder, "value.c");
    builder_add_source_file(&builder, "compiler.c");
    builder_add_source_file(&builder, "scanner.c");
    builder_add_source_file(&builder, "../libs/string.c");

    builder_build(&builder);

    builder_free(&builder);

    return 0;
}
