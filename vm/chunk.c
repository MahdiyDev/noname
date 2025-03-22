#include "../libs/dynamic_array.h"
#include "value.h"

#include "chunk.h"

void init_chunk(Chunk* chunk)
{
    da_init(chunk);
    chunk->lines = NULL;
    init_value_array(&chunk->constants);
}

void write_chunk(Chunk* chunk, uint8_t byte, int line)
{
    da_append(chunk, byte);
    da_grow(chunk->lines, chunk->count, chunk->capacity);
    chunk->lines[chunk->count - 1] = line;
}

int add_constant(Chunk* chunk, Value value)
{
    write_value_array(&chunk->constants, value);
    return chunk->constants.count - 1;
}

void free_chunk(Chunk* chunk)
{
    free(chunk->lines);
    free_value_array(&chunk->constants);
    da_free(chunk);
}