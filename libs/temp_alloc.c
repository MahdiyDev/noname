#include "temp_alloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#define TEMP_CAPACITY   (1024 * 128)   // 1MB
#define TEMP_INIT_COUNT (100)          // 10MB

static uint8_t temp_memory[TEMP_INIT_COUNT][TEMP_CAPACITY] = {0};
static bool temp_used[TEMP_INIT_COUNT] = {0};

typedef struct {
    size_t size;
    bool used;
} block_header;

temp_allocator temp_init()
{
    for (size_t i = 0; i < TEMP_INIT_COUNT; i++) {
        if (!temp_used[i]) {  // Find an available allocator
            temp_used[i] = true;
            return (temp_allocator) { i, 0 };
        }
    }
    
    fprintf(stderr, "Cannot allocate memory.\n");
    exit(EXIT_FAILURE);
}


void temp_uninit(temp_allocator allocator)
{
    if (allocator.temp_index < TEMP_INIT_COUNT) {
        temp_used[allocator.temp_index] = false;  // Mark as free
    }
}

void* temp_alloc(temp_allocator allocator, size_t size)
{
    while (allocator.temp_count < TEMP_CAPACITY) {
        block_header *block = (block_header *)&temp_memory[allocator.temp_index][allocator.temp_count];

        if (!block->used && (allocator.temp_count + sizeof(block_header) + size <= TEMP_CAPACITY)) {
            block->size = size;
            block->used = true;
            void* data_ptr = &temp_memory[allocator.temp_index][allocator.temp_count + sizeof(block_header)];
            memset(data_ptr, 0, size); 
            return &temp_memory[allocator.temp_index][allocator.temp_count + sizeof(block_header)];
        }
        allocator.temp_count += sizeof(block_header) + block->size;
    }

    return NULL;  // No free space
}

void* temp_realloc(temp_allocator allocator, void* ptr, size_t new_size)
{
    if (ptr == NULL) {
        return temp_alloc(allocator, new_size);
    }

    block_header *header = (block_header *)((uint8_t *)ptr - sizeof(block_header));

    if (new_size <= header->size) {
        return ptr;  // No need to allocate a new block if it fits
    }

    void *new_ptr = temp_alloc(allocator, new_size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, header->size);  // Copy old data
        temp_free(ptr);
    }

    return new_ptr;
}

char* temp_strdup(temp_allocator allocator, const char *cstr)
{
    size_t n = strlen(cstr);
    char *result = temp_alloc(allocator, n + 1);
    if (result) {
        memcpy(result, cstr, n);
        result[n] = '\0';
    }
    return result;
}

void temp_reset(temp_allocator allocator)
{
    allocator.temp_count = 0;
}

size_t temp_save(temp_allocator allocator)
{
    return allocator.temp_count;
}

void temp_rewind(temp_allocator allocator, size_t checkpoint)
{
    allocator.temp_count = checkpoint;
}

void temp_free(void* ptr)
{
    if (ptr == NULL) return;

    block_header *header = (block_header *)((uint8_t *)ptr - sizeof(block_header));
    header->used = false;  // Mark as free
}

void print_memory(temp_allocator allocator, FILE* fp)
{
    size_t i = 0;
    fprintf(fp, "Memory Dump:\n");
    while (i < TEMP_CAPACITY) {
        block_header *block = (block_header *)&temp_memory[allocator.temp_index][i];
        fprintf(fp, "[%s | size: %lu]\n", block->used ? "USED" : "FREE", block->size);
        i += sizeof(block_header) + block->size;
        if (i >= TEMP_CAPACITY) break;
    }
}
