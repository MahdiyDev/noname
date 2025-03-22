#pragma once

#include "dynamic_array.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct {
    char* items;
    size_t capacity;
    size_t count;
} string_builder;

typedef struct {
    const char* data;
    size_t count;
} string_view;

#define sv_fmt(sv) (int)sv.count, sv.data

string_view sv_from_cstr(const char* cstr);
string_view sv_from_parts(const char* data, size_t count);

string_view sb_to_sv(string_builder* sb);

string_view sv_split_cstr(string_view *sv, char* spliter);
string_view sv_split_c(string_view *sv, char spliter);
string_view sv_split_mul_cstr(string_view *sv, char* spliter, int n);
string_view sv_split_mul_c(string_view *sv, char spliter, int n);

bool sv_equal(string_view a, string_view b);
bool sv_equal_c(string_view a, const char b);
bool sv_equal_cstr(string_view a, const char* b);

bool sv_start_with(string_view sv, const char* cstr);
bool sv_end_with(string_view sv, const char *cstr);

bool sv_isdigit(const char sv);
size_t sv_to_digit(string_view sv);
string_view sv_from_digit(size_t n);

string_view sv_trim_left(string_view sv);
string_view sv_trim_right(string_view sv);
string_view sv_trim(string_view sv);

bool sv_in(string_view sv, const char** arr, int count);
bool sv_in_sv(string_view a, string_view b);
bool sv_in_cstr(string_view a, const char* cstr);
bool sv_in_c(string_view a, const char c);
#define sv_in_carr(sv, arr) sv_in(sv, arr, arr_count(arr))

string_builder* sb_init(const char* cstr);
void sb_free(string_builder* sb);

void sb_add(string_builder* sb, string_view sv);

void sb_add_cstr(string_builder* sb, const char* cstr);
void sb_add_first_cstr(string_builder* sb, const char* cstr);
void sb_delete_range_cstr(string_builder* sb, int start, int end);

void sb_add_c(string_builder* sb, const char c);
void sb_add_first_c(string_builder* sb, const char c);
void sb_delete_c(string_builder* sb, int index);

void sb_add_f(string_builder* sb, const char *format, ...);

void sb_clear(string_builder* sb);

// UTILS
bool sb_read_file(string_builder* sb, const char* file_path);
bool sb_read_file_from_fp(string_builder* sb, FILE* fp);
