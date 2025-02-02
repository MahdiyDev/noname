#include "environment.h"
#include "libs/hash_table.h"

#define ht_insert_int(ht, key, value) ht_insert_generic_value(ht, key, int, value)
#define ht_search_int(ht, key)        ht_search_generic_value(ht, key, int)

char* string_view_to_char(string_view s)
{
    char* n = malloc(s.count + 1);
    ht_to_char(n, (void*)s.data, s.count + 1);
    return n;
}

struct Enviroment* env_init()
{
    struct Enviroment* env = malloc(sizeof(struct Enviroment));
    env->values = ht_init();
    return env;
}

void env_destroy(struct Enviroment* env)
{
    ht_free(env->values);
    free(env);
}

void env_define(struct Enviroment* env, string_view name, int value)
{
    char* n = string_view_to_char(name);

    ht_insert_int(env->values, n, value);
    free(n);
}

struct Error* env_get(struct Enviroment* env, lexer_token name, int* value)
{
    struct Error* result = NULL;

    char* n = string_view_to_char(name.lexeme);

    if (ht_has(env->values, n)) {
        *value = *ht_search_int(env->values, n);
        return_defer(NULL);
    }

    result = error_f("at %s:%zu:%zu Undefined variable %.*s", lex_loc_fmt(name), sv_fmt(name.lexeme));

defer:
    free(n);
    return result;
}

struct Error* env_assign(struct Enviroment* env, lexer_token name, int value)
{    
    struct Error* result = NULL;

    char* n = string_view_to_char(name.lexeme);

    if (ht_has(env->values, n)) {
        int* current_value = ht_search_int(env->values, n);
        *current_value = value; 
        return_defer(NULL);
    }

    result = error_f("at %s:%zu:%zu Undefined variable %.*s", lex_loc_fmt(name), sv_fmt(name.lexeme));

defer:
    free(n);
    return result;
}
