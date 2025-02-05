#pragma once

#include <stddef.h>
#include "temp_alloc.h"

typedef struct {
    char* key;
    void* value;
    int value_size;
} ht_item;

typedef struct {
    size_t capacity;
    size_t count;
    ht_item** items;

    size_t base_capacity;

    temp_allocator temp;
} hash_table;

hash_table* ht_init();
hash_table* ht_init_with_capacity(const int base_capacity);
void ht_free(hash_table* ht);

void  ht_insert(hash_table* ht, const char* key, void* value, int value_size);
void* ht_search(hash_table* ht, const char* key);
void  ht_delete(hash_table* ht, const char* key);
bool  ht_has(hash_table* ht, const char* key);

void ht_to_char(char* cstr, void* data, int data_size);

#define ht_insert_generic_value(ht, key, value_type, value) do { value_type _v = value; ht_insert(ht, key, &_v, sizeof(value_type)); } while(0)
#define ht_search_generic_value(ht, key, value_type) (value_type*)ht_search(ht, key)

#define ht_insert_generic_key(ht, key_type, key, value_type, value) do { \
    value_type _v = value; \
    key_type _vk = key; \
    char _key[sizeof(key_type)]; \
    ht_to_char(_key, &_vk, sizeof(_vk)); \
    ht_insert(ht, _key, &_v, sizeof(value_type)); \
} while(0)
#define ht_search_generic_key(ht, key_type, key, value_type) ({ \
    key_type _vk = key; \
    char _key[sizeof(key_type)]; \
    ht_to_char(_key, &_vk, sizeof(_vk)); \
    (value_type*)ht_search(ht, _key); \
})
#define ht_has_generic_key(ht, key_type, key) ({ \
    key_type _vk = key; \
    char _key[sizeof(key_type)]; \
    ht_to_char(_key, &_vk, sizeof(_vk)); \
    ht_has(ht, _key); \
})
#define ht_delete_generic_key(ht, key_type, key) do { \
    key_type _vk = key; \
    char _key[sizeof(key_type)]; \
    ht_to_char(_key, &_vk, sizeof(_vk)); \
    ht_delete(ht, _key); \
} while(0)
