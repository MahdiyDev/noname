#include "../libs/dynamic_array.h"
#include <stdio.h>

#include "value.h"

void init_value_array(ValueArray* array)
{
    da_init(array);
}

void write_value_array(ValueArray* array, Value value)
{
    da_append(array, value);
}

void free_value_array(ValueArray* array)
{
    da_free(array);
}

void print_value(Value value)
{  
    switch (value.type) {
        case VAL_BOOL:
            printf(AS_BOOL(value) ? "true" : "false");
            break;
        case VAL_NIL: printf("nil"); break;
        case VAL_NUMBER: printf("%g", AS_NUMBER(value)); break;
    }
}
