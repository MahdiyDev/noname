#pragma once

#include <stddef.h>
#include <stdio.h>

typedef struct {
    size_t temp_index;
    size_t temp_count;
} temp_allocator;

temp_allocator temp_init();
void temp_uninit(temp_allocator allocator);

void* temp_alloc(temp_allocator allocator, size_t size);
void* temp_realloc(temp_allocator allocator, void* ptr, size_t new_size);
void temp_free(void* ptr);

char* temp_strdup(temp_allocator allocator, const char *cstr);

void temp_reset(temp_allocator allocator);
size_t temp_save(temp_allocator allocator);
void temp_rewind(temp_allocator allocator, size_t checkpoint);

void print_memory(temp_allocator allocator, FILE* fp);
