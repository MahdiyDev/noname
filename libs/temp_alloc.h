#include <stddef.h>

void* temp_alloc(size_t size);
void* temp_realloc(void* ptr, size_t new_size);
void temp_free(void* ptr);

char* temp_strdup(const char *cstr);

size_t temp_save(void);
void temp_reset(void);
void temp_rewind(size_t checkpoint);