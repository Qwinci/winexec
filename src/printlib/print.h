#pragma once
#include <stdio.h>
#include <wchar.h>
#include "windef.h"

int vfwprintf_musl(FILE *restrict f, const wchar_t *restrict fmt, __ms_va_list ap);
int vfprintf_musl(FILE *restrict f, const char *restrict fmt, __ms_va_list ap);
int vsprintf_musl(char *restrict str, const char *restrict fmt, __ms_va_list ap);
int vswprintf_musl(char *restrict str, const wchar_t *restrict fmt, __ms_va_list ap);
