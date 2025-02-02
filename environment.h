#include "lexer.h"
#include "libs/hash_table.h"
#include "libs/string.h"
#include "libs/error.h"

struct Enviroment {
    hash_table* values;

    struct Enviroment* enclosing;
};

struct Enviroment* env_init(struct Enviroment* enclosing);
void env_destroy(struct Enviroment* env);

void env_define(struct Enviroment* env, string_view name, int value);
struct Error* env_get(struct Enviroment* env, lexer_token name, int* value);
struct Error* env_assign(struct Enviroment* env, lexer_token name, int value);
