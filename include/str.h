#pragma once

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "types.h"

typedef struct {
	char* str;
	usize len;
	usize max_len;
} String;

static inline String string_new(const char* str, usize len) {
	if (len == (usize) -1) len = strlen(str);
	usize max_len = len < 8 ? 8 : len;
	char* buffer = malloc(max_len + 1);
	memcpy(buffer, str, len);
	buffer[len] = 0;
	return (String) {.str = buffer, .len = len, .max_len = max_len};
}

static inline String string_new_with_size(const char* str, usize len, usize max_len) {
	if (len == (usize) -1) len = strlen(str);
	if (max_len < len) max_len = len;
	if (max_len < 8) max_len = 8;
	char* buffer = malloc(max_len + 1);
	memcpy(buffer, str, len);
	return (String) {.str = buffer, .len = len, .max_len = len};
}

static inline void string_append(String* str1, const char* str2, usize len) {
	if (len == (usize) -1) len = strlen(str2);

	if (str1->len + len > str1->max_len) {
		usize new_max_len = str1->max_len * 2;
		if (new_max_len < (str1->len + len)) new_max_len = str1->len + len;
		str1->str = realloc(str1->str, new_max_len + 1);
		str1->max_len = new_max_len;
	}

	memcpy(str1->str + str1->len, str2, len);
	str1->len += len;
	str1->str[str1->len] = 0;
}

static inline void string_free(String str) {
	free(str.str);
	str.str = NULL;
	str.len = 0;
	str.max_len = 0;
}

static inline void string_clear(String* str) {
	str->len = 0;
}

static inline void string_prepend(String* str1, const char* str2, usize len) {
	if (len == (usize) -1) len = strlen(str2);
	if (str1->len + len > str1->max_len) {
		usize new_max_len = str1->max_len * 2;
		if (new_max_len < (str1->len + len)) new_max_len = str1->len + len;
		str1->str = realloc(str1->str, new_max_len + 1);
		str1->max_len = new_max_len;
	}
	memmove(str1->str + len, str1->str, str1->len);
	memcpy(str1->str, str2, len);
	str1->len += len;
	str1->str[str1->len] = 0;
}

static inline bool string_starts_with(String str, const char* str2) {
	return strncmp(str.str, str2, strlen(str2)) == 0;
}

static inline bool string_ends_with(String str, const char* str2) {
	usize len = strlen(str2);
	if (len > str.len) return false;
	return strncmp(str.str + str.len - len, str2, len) == 0;
}

static inline void string_strip_postfix(String* str, const char* postfix) {
	usize len = strlen(postfix);
	if (string_ends_with(*str, postfix)) {
		str->len -= len;
		str->str[str->len] = 0;
	}
}

static inline bool string_strip_prefix(String* str, const char* prefix) {
	usize len = strlen(prefix);
	if (string_starts_with(*str, prefix)) {
		memmove(str->str, str->str + len, str->len - len + 1);
		str->len -= len;
		return true;
	}
	return false;
}

static inline void string_replace_all_c(String* str, char replace, char with) {
	for (usize i = 0; i < str->len; ++i) {
		if (str->str[i] == replace) {
			str->str[i] = with;
		}
	}
}
