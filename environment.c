#include "environment.h"
#include "libs/hash_table.h"

#define ht_insert_int(ht, key, value) ht_insert_generic_value(ht, key, int, value)
#define ht_search_int(ht, key)        ht_search_generic_value(ht, key, int)

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
    char* n = malloc(name.count + 1);
    ht_to_char(n, (void*)name.data, name.count + 1);

    ht_insert_int(env->values, n, value);
    free(n);
}

struct Error* env_get(struct Enviroment* env, lexer_token name, int* value)
{
    struct Error* result = NULL;

    char* n = malloc(name.lexeme.count + 1);
    ht_to_char(n, (void*)name.lexeme.data, name.lexeme.count + 1);

    if (ht_has(env->values, n)) {
        *value = *ht_search_int(env->values, n);
        return_defer(NULL);
    }

    result = error_f("Undefined variable %.*s", sv_fmt(name.lexeme));

defer:
    free(n);
    return result;
}
