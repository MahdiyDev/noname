#pragma once

#ifndef __FUNCTION_NAME__
    #ifdef WIN32
        #define __FUNCTION_NAME__   __FUNCTION__  
    #else
        #define __FUNCTION_NAME__   __func__ 
    #endif
#endif

#define MAX_ERROR_MSG   128
#define MAX_ERROR_TRACE 128

typedef struct {
    const char* file_path;
    const char* func_name;
    int line;
} stack_trace;

struct Error {
    char message[MAX_ERROR_MSG];
    int trace_index;
    stack_trace stack_trace[MAX_ERROR_TRACE];
};

#define _error_f(fmt, ...) ({ \
    static struct Error e; \
    snprintf(e.message, MAX_ERROR_MSG, fmt, __VA_ARGS__); \
    e.trace_index = 0; \
    e.stack_trace[e.trace_index] = (stack_trace) { __FILE__, __FUNCTION_NAME__, __LINE__ }; \
    &e; \
})
#define error_f(fmt, ...) _error_f(fmt, __VA_ARGS__)

#define error(message)({ \
    static struct Error e = { message }; \
    e.trace_index = 0; \
    e.stack_trace[e.trace_index] = (stack_trace) { __FILE__, __FUNCTION_NAME__, __LINE__ }; \
    &e; \
})

#define trace(expr) ({ \
    error = (expr); \
    if (error != NULL && error->trace_index < MAX_ERROR_TRACE) { \
        error->trace_index++; \
        error->stack_trace[error->trace_index] = (stack_trace) { __FILE__, __FUNCTION_NAME__, __LINE__ }; \
    } \
    error; \
})

#define has_error(expr) ({ error = (expr); error != NULL; })

#define print_error(error) \
    do { \
        if (error != NULL) { \
            fprintf(stderr, "ERROR: %s\n", error->message); \
            for (int i = error->trace_index; i >= 0; i--) { \
                stack_trace s = error->stack_trace[i]; \
                fprintf(stderr, "    in %s (%s:%d)\n", s.func_name, s.file_path, s.line); \
            } \
        } else {\
           fprintf(stderr, "ERROR: (NULL)\n"); /* for debugging */ \
        } \
    } while(0)

#define return_defer(result, value) ({result = value; goto defer;})
