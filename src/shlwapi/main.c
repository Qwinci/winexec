#include "common.h"

#define OS_WOW6432 30

WINAPI BOOL IsOS(DWORD dw_os) {
	if (dw_os == OS_WOW6432) {
		return 1;
	}
	TODO;
	return 0;
}

static Ordinal ORDINALS[] = {
	{
		.num = 437,
		.fn = IsOS
	}
};

__attribute__((used)) Ordinals g_ordinals = {
	.ptr = ORDINALS,
	.count = 1
};