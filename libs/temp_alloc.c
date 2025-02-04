#include "temp_alloc.h"
#include <stdio.h>
#include <string.h>

#define TEMP_CAPACITY (8*1024*1024)

static size_t temp_count = 0;
static void* temp_memory[TEMP_CAPACITY] = {0};

void* temp_alloc(size_t size)
{
    if (temp_count + size > TEMP_CAPACITY) return NULL;
    void *result = &temp_memory[temp_count];
    temp_count += size;
    return result;
}

void* temp_realloc(void* ptr, size_t new_size)
{
    if (new_size == 0) {
        return NULL;
    }

    if (ptr == NULL) {
        return temp_alloc(new_size);
    }

    if (new_size <= temp_count) {
        return ptr;
    }

    if (temp_count + new_size > TEMP_CAPACITY) {
        return NULL;
    }

    void *new_ptr = &temp_memory[temp_count];
    temp_count += new_size;

    return new_ptr;
}

char* temp_strdup(const char *cstr)
{
    size_t n = strlen(cstr);
    char *result = temp_alloc(n + 1);
    memcpy(result, cstr, n);
    result[n] = '\0';
    return result;
}

void temp_reset(void)
{
    temp_count = 0;
}

size_t temp_save(void)
{
    return temp_count;
}

void temp_rewind(size_t checkpoint)
{
    temp_count = checkpoint;
}

void temp_free(void* ptr)
{
    if (ptr == NULL) return;

    if (ptr >= (void*)(&temp_memory[0]) && ptr < (void*)(&temp_memory[temp_count])) {
        temp_count = (size_t)(ptr - (void*)temp_memory);
    }
}