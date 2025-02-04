#include <stddef.h>

void* temp_alloc(size_t size);
void temp_reset(void);
size_t temp_save(void);
void temp_rewind(size_t checkpoint);