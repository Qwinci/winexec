#pragma once
#include "windef.h"
#include "types.h"
#include <stdio.h>
#if __STDC_VERSION__ < 202000L || (!defined(__clang__) && __GNUC__ < 13)
#include <stdbool.h>
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-attributes"
#endif

typedef struct {
	u16 num;
	void* fn;
} Ordinal;

typedef struct {
	Ordinal* ptr;
	usize count;
} Ordinals;

#define STUB(Name) WINAPI void Name() {fprintf(stderr, "todo: %s:%d: %s\n", __FILE__, __LINE__, #Name);}
#define TODO fprintf(stderr, "part_todo: %s:%d: %s\n", __FILE__, __LINE__, __func__)
