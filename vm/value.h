#pragma once

typedef double Value;

typedef struct {
    int capacity;
    int count;
    Value* items;
} ValueArray;

void init_value_array(ValueArray* array);
void write_value_array(ValueArray* array, Value value);
void free_value_array(ValueArray* array);
void print_value(Value value);
