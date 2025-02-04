#include "temp_alloc.h"

#define TEMP_CAPACITY (8*1024*1024)

static size_t temp_count = 0;
static void* temp_env[TEMP_CAPACITY] = {0};

void* temp_alloc(size_t size)
{
    if (temp_count + size > TEMP_CAPACITY) return NULL;
    void *result = &temp_env[temp_count];
    temp_count += size;
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