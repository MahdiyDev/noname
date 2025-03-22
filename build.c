#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libs/string.h"
#include "string.h"

void get_dependencies(const char *filename) {
    string_builder *command = sb_init(NULL);
    sb_add_f(command, "gcc -MM %s", filename);

    FILE *fp = popen(sb_to_sv(command).data, "r");
    if (fp == NULL) {
        perror("popen failed");
        sb_free(command);
        exit(EXIT_FAILURE);
    }
    sb_free(command);

    string_builder *buffer = sb_init(NULL);
    string_view sv;

    if (!sb_read_file_from_fp(buffer, fp)) {
        perror("Error reading from gcc");
        pclose(fp);
        sb_free(buffer);
        exit(EXIT_FAILURE);
    }

    sv = sb_to_sv(buffer);

    string_view obj_file = sv_split_c(&sv, ':');
    string_view src_file = sv_split_mul_cstr(&sv, " ", 2);

    printf("Object: %.*s\n", sv_fmt(obj_file));
    printf("File: %.*s\n", sv_fmt(src_file));
    printf("Deps:\n");

    while (sv.count > 0) {
        string_view dep = sv_split_c(&sv, ' ');
        sv = sv_trim(sv);
        if (dep.count > 0 && !sv_equal_c(dep, '\\')) {
            printf("%.*s\n", sv_fmt(dep));
        }
    }

    pclose(fp);
    sb_free(buffer);
}

// int main(int argc, char *argv[]) {
//     if (argc != 2) {
//         fprintf(stderr, "Usage: %s <source_file>\n", argv[0]);
//         return EXIT_FAILURE;
//     }

//     get_dependencies(argv[1]);
//     return EXIT_SUCCESS;
// }


#include <stdio.h>
#include <dlfcn.h>

int main(int argc, char *argv[]) {
    void *handle;
    int (*main_func)(int, char**);
    void* (*lib_malloc)(size_t);

    // Open the dynamic library
    handle = dlopen("./noname.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Error: %s\n", dlerror());
        return 1;
    }

    // Get function pointer
    main_func = (int (*)(int, char**)) dlsym(handle, "main");
    if (!main_func) {
        fprintf(stderr, "Error: %s\n", dlerror());
        dlclose(handle);
        return 1;
    }
    lib_malloc = (void* (*)(size_t)) dlsym(handle, "malloc");
    if (!lib_malloc) {
        fprintf(stderr, "Error: %s\n", dlerror());
        return 1;
    }

    printf("main_func:  %p\n", main_func);
    printf("lib_malloc: %p\n", lib_malloc);

    // // Call the function
    main_func(argc, argv);

    // Close the library
    dlclose(handle);

    return 0;
}
