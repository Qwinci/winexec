#include "common.h"
#include "winnt.h"

static BOOL initialized = 0;

WINAPI HRESULT OleInitialize(LPVOID pv_reserved) {
	if (initialized) {
		return S_FALSE;
	}
	else {
		initialized = true;
		return S_OK;
	}
}

WINAPI void OleUninitialize() {
	initialized = false;
}

STUB(CoTaskMemFree)
STUB(CoCreateInstance)
STUB(IIDFromString)
STUB(StringFromGUID2)
STUB(CLSIDFromString)
