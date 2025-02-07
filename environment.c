#include "environment.h"
#include "libs/error.h"
#include "libs/hash_table.h"
#include "libs/temp_alloc.h"

#define ht_insert_lexer_token_value(ht, key, value) ht_insert_generic_value(ht, key, struct lexer_token_value, value)
#define ht_search_lexer_token_value(ht, key)        ht_search_generic_value(ht, key, struct lexer_token_value)

char* string_view_to_char(string_view s)
{
    static char n[256];
    ht_to_char(n, (void*)s.data, s.count + 1);
    return n;
}

struct Enviroment* env_init(struct Enviroment* enclosing)
{
    temp_allocator allocator = temp_init();
    struct Enviroment* env = temp_alloc(allocator, sizeof(struct Enviroment));

    env->allocator = allocator;
    env->values = ht_init_with_capacity(7);
    env->enclosing = enclosing;

    return env;
}

void env_destroy(struct Enviroment* env)
{
    ht_free(env->values);
    // if (env->enclosing != NULL) env_destroy(env->enclosing);
    temp_uninit(env->allocator);
    temp_free(env);
}

void env_define(struct Enviroment* env, string_view name, struct lexer_token_value value)
{
    char* n = string_view_to_char(name);

    ht_insert_lexer_token_value(env->values, n, value);
    // temp_free(n);
}

struct Error* env_get(struct Enviroment* env, lexer_token name, struct lexer_token_value* value)
{
    struct Error* error = NULL;

    char* n = string_view_to_char(name.lexeme);

    if (ht_has(env->values, n)) {
        *value = *ht_search_lexer_token_value(env->values, n);
        return_defer(error, NULL);
    }

    if (env->enclosing != NULL) return env_get(env->enclosing, name, value);

    return_defer(error, error_f("at %s:%zu:%zu Undefined variable %.*s", lex_loc_fmt(name), sv_fmt(name.lexeme)));

defer:
    // temp_free(n);
    return error;
}

struct Error* env_assign(struct Enviroment* env, lexer_token name, struct lexer_token_value value)
{    
    struct Error* error = NULL;

    char* n = string_view_to_char(name.lexeme);

    if (ht_has(env->values, n)) {
        struct lexer_token_value* current_value = ht_search_lexer_token_value(env->values, n);
        *current_value = value; 
        return_defer(error, NULL);
    }

    if (env->enclosing != NULL) return env_assign(env->enclosing, name, value);

    return_defer(error, error_f("at %s:%zu:%zu Undefined variable %.*s", lex_loc_fmt(name), sv_fmt(name.lexeme)));

defer:
    // temp_free(n);
    return error;
}
