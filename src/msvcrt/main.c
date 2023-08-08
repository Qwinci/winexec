#include "windef.h"
#include "processthreadapi.h"
#include "common.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

STUB(win___C_specific_handler)

static int i_argc;
static char** i_argv;
static char** i_envp;

char* win__acmdln;
char*** win___initenv;

static void onexit_handler();

void set_args(int argc, char** argv, char** envp) {
	i_argc = argc;
	i_argv = argv;
	i_envp = envp;
	size_t len = 0;
	for (int i = 0; i < argc; ++i) {
		len += strlen(argv[i]);
	}
	win__acmdln = malloc(len + 1);
	len = 0;
	for (int i = 0; i < argc; ++i) {
		size_t a_len = strlen(argv[i]);
		memcpy(win__acmdln + len, argv[i], a_len);
		len += a_len;
	}
	win__acmdln[len] = 0;

	win___initenv = &i_envp;
	atexit(onexit_handler);
}

WINAPI int win___getmainargs(int* argc, char*** argv, char*** env, int doWildCard, STARTUPINFOA* startInfo) {
	*argc = i_argc;
	*argv = i_argv;
	*env = i_envp;
	return 0;
}

STUB(win___iob_func)

WINAPI int win___lconv_init() {
	// todo charmax
	return 0;
}

WINAPI void win___set_app_type(int at) {
	return;
	// todo
}

STUB(win___setusermatherr)

STUB(win__amsg_exit)
STUB(win__cexit)

#define O_TEXT 0x4000
#define O_BINARY 0x8000

int win__fmode;

typedef void (WINAPI* PVFV)();

WINAPI void win__initterm(const PVFV* ppfn, const PVFV* end) {
	do {
		PVFV pfn = *++ppfn;
		if (pfn) pfn();
	} while (ppfn < end);
}

typedef int (WINAPI* PIFV)();

WINAPI int win__initterm_e(const PIFV* ppfn, const PIFV* end) {
	do {
		PIFV pfn = *++ppfn;
		if (pfn) {
			int err = pfn();
			if (err) return err;
		}
	} while (ppfn < end);

	return 0;
}

typedef int (*WINAPI onexit_t)(void);

static onexit_t onexit_fns[32];
static int onexit_count = 0;

static void onexit_handler() {
	for (int i = 0; i < onexit_count; ++i) {
		onexit_fns[i]();
	}
}

WINAPI onexit_t win__onexit(onexit_t function) {
	if (onexit_count == 32) return NULL;
	onexit_fns[onexit_count++] = function;
	return function;
}

STUB(win_abort)
STUB(win_calloc)

[[noreturn]] WINAPI void win_exit(int status) {
	exit(status);
}

STUB(win_fprintf)
STUB(win_free)
STUB(win_fwrite)

WINAPI int win_getchar() {
	return getchar();
}

WINAPI void* win_malloc(size_t size) {
	return malloc(size);
}

WINAPI void* win_memcpy(void* dest, const void* src, size_t count) {
	return memcpy(dest, src, count);
}

extern int vfprintf_musl(FILE *restrict f, const char *restrict fmt, __ms_va_list ap);

WINAPIV int win_printf(const char* format, ...) {
	__ms_va_list ap;
	__ms_va_start(ap, format);

	int status = vfprintf_musl(stdout, format, ap);

	__ms_va_end(ap);
	return status;
}

STUB(win_signal)

WINAPI size_t win_strlen(const char* str) {
	return strlen(str);
}

WINAPI int win_strncmp(const char* str1, const char* str2, size_t count) {
	for (; count; --count, ++str1, ++str2) {
		int res = *str1 - *str2;
		if (res != 0) {
			return res;
		}
		else if (!*str1) {
			return 0;
		}
	}
	return 0;
}

STUB(win_vfprintf)
STUB(win_iswspace)
STUB(win__msize)
STUB(win_iswalpha)
STUB(win__snprintf)
STUB(win_swscanf)
STUB(win_iswcntrl)
STUB(win__beginthreadex)
STUB(win_time)
STUB(win_srand)
STUB(win_rand)
STUB(win_iswalnum)
STUB(win___wgetmainargs)
STUB(win_wcsncpy)
STUB(win__wcsnicmp)
STUB(win_wcschr)
STUB(win__wcsicmp)
STUB(win__snwprintf)
STUB(win_memset)

WINAPI void* win_operator_new(size_t size) {
	return malloc(size);
}

WINAPI void win_operator_delete(void* ptr) {
	free(ptr);
}
