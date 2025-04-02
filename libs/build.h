#pragma once

#include <stdbool.h>
#include "string.h"

typedef struct {
    int count;
    int capacity;
    string_view* items;
} Files;

typedef struct {
    string_view filename;
    Files dependencies;
    string_view obj_file;
    string_view src_file;
    string_builder buffer;
} FileDep;

typedef struct {
    int count;
    int capacity;
    FileDep* items;
} FileDeps;

typedef struct {
    string_view executable;
    FileDeps object_files;
    const char* output_dir;
} Builder;

bool cmd(const char *command);
void builder_add_executable(Builder* builder, const char* executable_name);
void builder_add_source_file(Builder* builder, const char* filename);
void builder_build(Builder* builder);
void builder_free(Builder* builder);

#ifdef BUILD_IMPLEMENTATION
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "dynamic_array.h"

void free_dependencies(FileDep *deps)
{
    sb_free(&deps->buffer);
    da_free(&deps->dependencies);
}

FileDep get_dependencies(const char *filename)
{
    FileDep deps = {0};
    da_init(&deps.dependencies);

    string_builder command = sb_init(NULL);
    sb_add_f(&command, "gcc -MM %s", filename);
    sb_add_c(&command, '\0');

    FILE *fp = popen(sb_to_sv(&command).data, "r");
    if (fp == NULL) {
        perror("popen failed");
        sb_free(&command);
        exit(EXIT_FAILURE);
    }
    sb_free(&command);

    deps.buffer = sb_init(NULL);
    string_view sv;

    if (!sb_read_file_from_fp(&deps.buffer, fp)) {
        perror("Error reading from gcc");
        pclose(fp);
        sb_free(&deps.buffer);
        exit(EXIT_FAILURE);
    }

    sv = sb_to_sv(&deps.buffer);

    string_view obj_file = sv_split_c(&sv, ':');
    string_view src_file = sv_split_mul_cstr(&sv, " ", 2);

    deps.obj_file = sv_trim(obj_file);
    deps.src_file = sv_trim(src_file);

    while (sv.count > 0) {
        string_view dep = sv_split_c(&sv, ' ');
        sv = sv_trim(sv);
        if (dep.count > 0 && !sv_equal_c(dep, '\\')) {
            da_append(&deps.dependencies, dep);
        }
    }

    pclose(fp);

    return deps;
}

bool cmd(const char *command)
{
    int status = system(command);
    if (status == -1) {
        perror("system");
        return false;
    }

    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        fprintf(stderr, "[ERROR] Command failed with exit status %d\n", WEXITSTATUS(status));
        return false;
    }

    return true;
}

void builder_add_executable(Builder* builder, const char* executable_name)
{
    builder->executable = sv_from_cstr(executable_name);
    da_init(&builder->object_files);
}

void builder_add_source_file(Builder* builder, const char* filename)
{
    FileDep dep = get_dependencies(filename);
    da_append(&builder->object_files, dep);
}

void builder_build(Builder* builder)
{
    if (builder->object_files.count == 0) {
        fprintf(stderr, "[ERROR] No source files to build.\n");
        return;
    }

    bool needs_relink = false;

    for (int i = 0; i < builder->object_files.count; i++) {
        char object_file[256], src_file[256];
        snprintf(object_file, sizeof(object_file), "%s/%.*s",
                 builder->output_dir, sv_fmt(builder->object_files.items[i].obj_file));
        snprintf(src_file, sizeof(src_file), "%.*s", sv_fmt(builder->object_files.items[i].src_file));

        struct stat obj_stat, src_stat;
        int obj_exists = (stat(object_file, &obj_stat) == 0);
        int src_exists = (stat(src_file, &src_stat) == 0);

        if (!src_exists) {
            fprintf(stderr, "[ERROR] Source file '%s' not found.\n", src_file);
            exit(EXIT_FAILURE);
        }

        bool needs_recompile = !obj_exists || obj_stat.st_mtime < src_stat.st_mtime;

        // Check if any dependency (header file) has changed
        for (int j = 0; j < builder->object_files.items[i].dependencies.count; j++) {
            char header_file[256];
            snprintf(header_file, sizeof(header_file), "%.*s",
                     sv_fmt(builder->object_files.items[i].dependencies.items[j]));

            struct stat header_stat;
            if (stat(header_file, &header_stat) == 0) {
                if (!obj_exists || obj_stat.st_mtime < header_stat.st_mtime) {
                    printf("[INFO] Header file '%s' modified. Recompiling '%s'\n", header_file, src_file);
                    needs_recompile = true;
                    break;
                }
            }
        }

        if (needs_recompile) {
            printf("[INFO] Compiling '%s' -> '%s'\n", src_file, object_file);
            char command[256];
            snprintf(command, sizeof(command), "gcc -g -O0 -c -o %s %s", object_file, src_file);
            printf("[CMD] %s\n", command);
            if (!cmd(command)) {
                fprintf(stderr, "[ERROR] Compilation failed for '%s'\n", src_file);
                exit(EXIT_FAILURE);
            }
            needs_relink = true;
        } else {
            printf("[INFO] Skipping compilation for '%s' (up to date)\n", src_file);
        }
    }

    // Only relink if at least one object file was updated
    if (needs_relink) {
        string_builder command = sb_init(NULL);
        sb_add_f(&command, "gcc -g -O0 -o %.*s", sv_fmt(builder->executable));

        for (int i = 0; i < builder->object_files.count; i++) {
            sb_add_f(&command, " %s/%.*s", builder->output_dir, sv_fmt(builder->object_files.items[i].obj_file));
        }

        sb_add_c(&command, '\0');
        printf("[CMD] %s\n", sb_to_sv(&command).data);
        if (!cmd(sb_to_sv(&command).data)) {
            fprintf(stderr, "[ERROR] Linking failed for '%.*s'\n", sv_fmt(builder->executable));
            exit(EXIT_FAILURE);
        }
        sb_free(&command);
    } else {
        printf("[INFO] No object files changed, skipping linking.\n");
    }
}

void builder_free(Builder* builder)
{
    for (int i = 0; i < builder->object_files.count; i++) {
        free_dependencies(&builder->object_files.items[i]);
    }
    da_free(&builder->object_files);
}

#endif // BUILD_IMPLEMENTATION
