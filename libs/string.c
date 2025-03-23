#include "string.h"
#include <stdarg.h>
#include <ctype.h>

string_view sv_from_parts(const char* data, size_t count)
{
    string_view sv;
    sv.count = count;
    sv.data = data;
    return sv;
}

string_view sv_from_cstr(const char* cstr)
{
    return sv_from_parts(cstr, strlen(cstr));
}

string_view sv_split_cstr(string_view *sv, char* spliter)
{
	size_t len = strlen(spliter);
	string_view result = sv_split_c(sv, spliter[len - 1]);

	for (int i = len - 1; i >= 0; i--) {
		result = sv_split_c(&result, spliter[i]);
	}

	return result;
}

string_view sv_split_c(string_view *sv, char spliter)
{
    size_t i = 0;
    while (i < sv->count && sv->data[i] != spliter) {
        i += 1;
    }

    string_view result = sv_from_parts(sv->data, i);

    if (i < sv->count) {
        sv->count -= i + 1;
        sv->data  += i + 1;
    } else {
        sv->count -= i;
        sv->data  += i;
    }

    return result;
}

string_view sv_split_mul_cstr(string_view *sv, char* spliter, int n)
{
	string_view result;
	for (int i = 0; i < n; i++) {
		result = sv_split_cstr(sv, spliter);
	}
	return result;
}

string_view sv_split_mul_c(string_view *sv, char spliter, int n)
{
	string_view result;
	for (int i = 0; i < n; i++) {
		result = sv_split_c(sv, spliter);
		result = sv_split_c(&result, spliter);
	}
	return result;
}

bool sv_equal(string_view a, string_view b)
{
    if (a.count != b.count) {
        return false;
    } else {
        return memcmp(a.data, b.data, a.count) == 0;
    }
}

bool sv_equal_c(string_view a, const char b)
{
	if (a.count <= 0) return false;
	return a.data[0] == b;
}

bool sv_equal_cstr(string_view a, const char* b)
{
	return sv_equal(a, sv_from_cstr(b));
}

bool sv_start_with(string_view sv, const char* cstr)
{
	size_t cstr_count = strlen(cstr);
	if (sv.count < cstr_count) return false;

	return memcmp(sv.data, cstr, cstr_count) == 0;
}

bool sv_end_with(string_view sv, const char *cstr)
{
    size_t cstr_count = strlen(cstr);
    if (sv.count >= cstr_count) {
        size_t ending_start = sv.count - cstr_count;
        string_view sv_ending = sv_from_parts(sv.data + ending_start, cstr_count);
        return sv_equal_cstr(sv_ending, cstr);
    }
    return false;
}

bool sv_isdigit(const char c)
{
	return c >= '0' && c <= '9';
}

size_t sv_to_digit(string_view sv)
{
	size_t value = 0;
	int n;
	for (int i = 0; i < sv.count; i++) {
		if (isdigit(sv.data[i]) == 0) break;
		n = sv.data[i] - '0';
		if (n > 9) return value;
		if (i == 0) {
			value = value + n;
		} else {
			value = value * 10 + n;
		}
	}
	return value;
}

string_view sv_from_digit(size_t n) {
    static char buffer[21];
    size_t i = 20;
    buffer[i] = '\0';

    do {
        buffer[--i] = '0' + (n % 10);
        n /= 10;
    } while (n > 0);

	string_view sv = { &buffer[i], 20 - i };
    return sv;
}

string_view sv_trim_left(string_view sv)
{
    size_t i = 0;
    while (i < sv.count && isspace(sv.data[i])) {
        i += 1;
    }

    return sv_from_parts(sv.data + i, sv.count - i);
}

string_view sv_trim_right(string_view sv)
{
    size_t i = 0;
    while (i < sv.count && isspace(sv.data[sv.count - 1 - i])) {
        i += 1;
    }

    return sv_from_parts(sv.data, sv.count - i);
}

string_view sv_trim(string_view sv)
{
    return sv_trim_right(sv_trim_left(sv));
}

bool sv_in(string_view sv, const char** arr, int count)
{
    for (int i = 0; i < count; i++) {
        if (sv_equal(sv_from_cstr(arr[i]), sv)) {
            return true;
        }
    }
    return false;
}

bool sv_in_sv(string_view a, string_view b)
{
	if (a.count < b.count) return false;
	for (int i = 0; i < a.count; i++) {
		if (sv_equal(sv_from_parts(a.data + i, b.count), b)) return true;
	}
	return false;
}

bool sv_in_cstr(string_view a, const char* cstr)
{
	return sv_in_sv(a, sv_from_cstr(cstr));
}

bool sv_in_c(string_view a, const char c)
{
	for (int i = 0; i < a.count; i++) {
		if (a.data[i] == c) return true;
	}
	return false;
}

string_view sb_to_sv(string_builder* sb)
{
    return sv_from_parts((sb)->items, (sb)->count);
}

string_builder sb_init(const char* cstr)
{
    string_builder sb = {0};
    da_init(&sb);
    if (cstr != NULL) {
        sb_add_cstr(&sb, cstr);
    }
    return sb;
}

void sb_free(string_builder* sb)
{
    da_free(sb);
}

void sb_add(string_builder* sb, string_view sv)
{
    da_append_many(sb, sv.data, sv.count);
}

void sb_add_cstr(string_builder* sb, const char* cstr)
{
    da_append_many(sb, cstr, strlen(cstr));
}

void sb_add_first_cstr(string_builder* sb, const char* cstr)
{
    da_prepend_many(sb, cstr, strlen(cstr));
}

void sb_delete_range_cstr(string_builder* sb, int start_index, int end_index)
{
    da_delete_range(sb, start_index, end_index);
}

void sb_clear(string_builder* sb)
{
	da_clear(sb);
}

void sb_add_c(string_builder* sb, const char c)
{
    da_append(sb, c);
}

void sb_add_first_c(string_builder* sb, const char c)
{
    da_prepend(sb, c);
}

void sb_delete_c(string_builder* sb, int index)
{
    da_delete(sb, index);
}

void sb_add_f(string_builder* sb, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int n = vsnprintf(NULL, 0, format, args);
    va_end(args);

    DA_ASSERT(n >= 0);
    char* temp = (char*)malloc(n + 1);
    DA_ASSERT(temp != NULL && "Failed to allocate memory\n");

    va_start(args, format);
    vsnprintf(temp, n + 1, format, args);
    va_end(args);

	sb_add_cstr(sb, temp);
	free(temp);
}

// UTILS
bool sb_read_file(string_builder* sb, const char* file_path)
{
    FILE *file = fopen(file_path, "rb");
    if (file == NULL) return false;

    bool result = sb_read_file_from_fp(sb, file);
    if (!result) fclose(file);

    return result;
}

bool sb_read_file_from_fp(string_builder* sb, FILE* fp)
{
    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        sb_add_c(sb, ch);
    }

    if (ferror(fp)) {
        return false;
    }

    return true;
}
