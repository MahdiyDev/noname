#include "environment.h"
#include "libs/hash_table.h"
#include "libs/temp_alloc.h"

#define ht_insert_int(ht, key, value) ht_insert_generic_value(ht, key, int, value)
#define ht_search_int(ht, key)        ht_search_generic_value(ht, key, int)

char* string_view_to_char(string_view s)
{
    static char n[256];
    ht_to_char(n, (void*)s.data, s.count + 1);
    return n;
}

struct Enviroment* env_init(struct Enviroment* enclosing)
{
    struct Enviroment* env = temp_alloc(sizeof(struct Enviroment));
    env->temp_index = temp_save();
    env->values = ht_init_with_capacity(10);
    env->enclosing = enclosing;
    return env;
}

void env_destroy(struct Enviroment* env)
{
    ht_free(env->values);
    // if (env->enclosing != NULL) env_destroy(env->enclosing);
    temp_rewind(env->temp_index);
}

void env_define(struct Enviroment* env, string_view name, int value)
{
    char* n = string_view_to_char(name);

    ht_insert_int(env->values, n, value);
}

struct Error* env_get(struct Enviroment* env, lexer_token name, int* value)
{
    struct Error* error = NULL;

    char* n = string_view_to_char(name.lexeme);

    if (ht_has(env->values, n)) {
        *value = *ht_search_int(env->values, n);
        return NULL;
    }

    if (env->enclosing != NULL) return env_get(env->enclosing, name, value);

    return error_f("at %s:%zu:%zu Undefined variable %.*s", lex_loc_fmt(name), sv_fmt(name.lexeme));
}

struct Error* env_assign(struct Enviroment* env, lexer_token name, int value)
{    
    struct Error* error = NULL;

    char* n = string_view_to_char(name.lexeme);

    if (ht_has(env->values, n)) {
        int* current_value = ht_search_int(env->values, n);
        *current_value = value; 
        return NULL;
    }

    if (env->enclosing != NULL) return env_assign(env->enclosing, name, value);

    return error_f("at %s:%zu:%zu Undefined variable %.*s", lex_loc_fmt(name), sv_fmt(name.lexeme));
}
