#include "lexer.h"
#include "libs/hash_table.h"
#include "libs/string.h"
#include "libs/error.h"
#include "libs/temp_alloc.h"

struct Enviroment {
    hash_table* values;
    temp_allocator allocator;

    struct Enviroment* enclosing;
};

struct Enviroment* env_init(struct Enviroment* enclosing);
void env_destroy(struct Enviroment* env);

void env_define(struct Enviroment* env, string_view name, struct lexer_token_value value);
struct Error* env_get(struct Enviroment* env, lexer_token name, struct lexer_token_value* value);
struct Error* env_assign(struct Enviroment* env, lexer_token name, struct lexer_token_value value);
