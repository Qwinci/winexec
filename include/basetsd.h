#pragma once
#include <stdint.h>
#include <stddef.h>

#ifdef CONFIG_64BIT
typedef uint64_t ULONG_PTR;
typedef int64_t LONG_PTR;
typedef int64_t INT_PTR;
#else
typedef unsigned int ULONG_PTR;
typedef int LONG_PTR;
typedef int INT_PTR;
#endif

typedef size_t SIZE_T;
typedef LONG_PTR LPARAM;
typedef ULONG_PTR WPARAM;

typedef uint64_t DWORD64;
