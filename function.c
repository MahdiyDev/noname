#include "function.h"
#include "interpreter.h"
#include "environment.h"
#include "lexer.h"
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

struct Error* native_print_fun(struct callable_value value, struct Interpreter* intp, Arguments* args, struct lexer_token_value* _)
{
    struct Error* error = NULL;

    for (int i = 0; i < args->count; i++) {
        struct lexer_token_value result = args->items[i];

        switch (result.type) {
        case VALUE_TYPE_INT:
            printf("%d", result.int_value);
            break;
        case VALUE_TYPE_INT_LONG_LONG:
            printf("%lld", result.int_long_long_value);
            break;
        case VALUE_TYPE_STRING: 
            printf("%.*s", sv_fmt(result.string_value));
            break;
        case VALUE_TYPE_CALLABLE: 
            printf("<native fun>");
            break;
        }
    }

    return NULL;
}

struct Error* native_println_fun(struct callable_value value, struct Interpreter* intp, Arguments* args, struct lexer_token_value* _)
{
    struct Error* error = NULL;

    for (int i = 0; i < args->count; i++) {
        struct lexer_token_value result = args->items[i];

        switch (result.type) {
        case VALUE_TYPE_INT:
            printf("%d\n", result.int_value);
            break;
        case VALUE_TYPE_INT_LONG_LONG:
            printf("%lld\n", result.int_long_long_value);
            break;
        case VALUE_TYPE_STRING: 
            printf("%.*s\n", sv_fmt(result.string_value));
            break;
        case VALUE_TYPE_CALLABLE: 
            printf("<native fun>\n");
            break;
        }
    }

    return NULL;
}

void init_native_functions(struct Interpreter* intp)
{
    struct lexer_token_value clock;
    clock.type = VALUE_TYPE_CALLABLE;
    clock.callable_value.arity = 0;
    clock.callable_value.call = native_clock_fun;
    
    env_define(intp->global_env, sv_from_cstr("clock"), clock);
    
    struct lexer_token_value println;
    println.type = VALUE_TYPE_CALLABLE;
    println.callable_value.arity = 1;
    println.callable_value.call = native_println_fun;
    
    env_define(intp->global_env, sv_from_cstr("println"), println);

    struct lexer_token_value print;
    print.type = VALUE_TYPE_CALLABLE;
    print.callable_value.arity = 1;
    print.callable_value.call = native_print_fun;

    env_define(intp->global_env, sv_from_cstr("print"), print);
}

struct Error* callable_function(struct callable_value value, struct Interpreter* intp, Arguments* args, struct lexer_token_value* return_value)
{
    struct Error* error = NULL;

    struct Enviroment* env = env_init(value.closure);
    for (int i = 0; i < value.declaration->function_stmt.params->count; i++) {
        env_define(env, value.declaration->function_stmt.params->items[i].lexeme, args->items[i]);
    }

    if (has_error(execute_block(intp, value.declaration->function_stmt.body, env, return_value))) {
        env_destroy(env);
        return trace(error);
    }

    env_destroy(env);
    return NULL;
}

struct lexer_token_value create_function(struct Stmt* declaration, struct Enviroment* closure)
{
    struct lexer_token_value func;
    func.type = VALUE_TYPE_CALLABLE;
    func.callable_value.call = callable_function;
    func.callable_value.declaration = declaration;
    func.callable_value.arity = declaration->function_stmt.params->count;
    func.callable_value.closure = closure;
    return func;
}