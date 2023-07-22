#pragma once
#include "winnt.h"

typedef PRTL_CRITICAL_SECTION LPCRITICAL_SECTION;

typedef struct {
	DWORD dwLowDateTime;
	DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;