#include "function.h"
#include "interpreter.h"
#include "environment.h"
#include <sys/time.h>

long long current_time_millis()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)(tv.tv_sec) * 1000 + (tv.tv_usec / 1000);
}

struct Error* native_clock_fun(struct callable_value value, struct Interpreter* intp, Arguments* args, struct lexer_token_value* result)
{
    *result = (struct lexer_token_value) {
        .type = VALUE_TYPE_INT_LONG_LONG,
        .int_long_long_value = current_time_millis() / 1000,
    };
    return NULL;
}

struct Error* callable_function(struct callable_value value, struct Interpreter* intp, Arguments* args, struct lexer_token_value* result)
{
    struct Error* error = NULL;

    struct Enviroment* env = env_init(intp->global_env);
    for (int i = 0; i < value.declaration->function_stmt.params->count; i++) {
        env_define(env, value.declaration->function_stmt.params->items[i].lexeme, args->items[i]);
    }

    if (has_error(execute_block(intp, value.declaration->function_stmt.body, env))) {
        return trace(error);
    }

    env_destroy(env);
    return NULL;
}

struct lexer_token_value create_function(struct Stmt* declaration)
{
    struct lexer_token_value func;
    func.type = VALUE_TYPE_CALLABLE;
    func.callable_value.call = callable_function;
    func.callable_value.declaration = declaration;
    func.callable_value.arity = declaration->function_stmt.params->count;
    return func;
}