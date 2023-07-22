#pragma once
#include <stdint.h>
#include "basetsd.h"

#ifdef CONFIG_64BIT
#if __has_attribute(force_align_arg_pointer)
#define WINAPI __attribute__((ms_abi, force_align_arg_pointer))
#else
#define WINAPI __attribute__((ms_abi))
#endif
#define WINAPIV WINAPI
#else
#if __has_attribute(force_align_arg_pointer)
#define WINAPI __attribute__((stdcall, force_align_arg_pointer))
#define WINAPIV __attribute__((cdecl, force_align_arg_pointer))
#else
#define WINAPI __stdcall
#define WINAPIV __cdecl
#endif
#endif

#if CONFIG_64BIT
#define __ms_va_list __builtin_ms_va_list // NOLINT(bugprone-reserved-identifier)
#define __ms_va_start(ap, arg) __builtin_ms_va_start(ap, arg) // NOLINT(bugprone-reserved-identifier)
#define __ms_va_end(ap) __builtin_ms_va_end(ap) // NOLINT(bugprone-reserved-identifier)
#define __ms_va_copy(dest, src) __builtin_ms_va_copy(dest, src) // NOLINT(bugprone-reserved-identifier)
#else
#define __ms_va_list __builtin_va_list // NOLINT(bugprone-reserved-identifier)
#define __ms_va_start(ap, arg) __builtin_va_start(ap, arg) // NOLINT(bugprone-reserved-identifier)
#define __ms_va_end(ap) __builtin_va_end(ap) // NOLINT(bugprone-reserved-identifier)
#define __ms_va_copy(dest, src) __builtin_va_copy(dest, src) // NOLINT(bugprone-reserved-identifier)
#endif

#ifndef __WINDEF_NO_TYPES

typedef int BOOL;
typedef unsigned char BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint64_t QWORD;
typedef short SHORT;
typedef unsigned short USHORT;
typedef int LONG;
typedef int64_t LONGLONG;
typedef unsigned int ULONG;
typedef unsigned int UINT;
typedef uint64_t ULONGLONG;
typedef BYTE* LPBYTE;
typedef DWORD* DWORD_PTR;
typedef DWORD* LPDWORD;
typedef LONG* PLONG;
typedef void* LPVOID;
typedef const void* LPCVOID;
typedef LONG LSTATUS;
typedef LONG_PTR LRESULT;
typedef WORD ATOM;

typedef void* HANDLE;

#define INVALID_HANDLE_VALUE ((HANDLE) -1)

typedef HANDLE HINSTANCE;
typedef HANDLE HMODULE;
typedef HANDLE HICON;
typedef HANDLE HWND;
typedef HANDLE HGLOBAL;
typedef HANDLE HKEY;
typedef HKEY* PHKEY;
typedef HICON HCURSOR;
typedef HANDLE HBRUSH;
typedef HANDLE HMENU;
typedef HANDLE HDC;
typedef HANDLE HFONT;
typedef HANDLE HGDIOBJ;

#endif