#include <sys/mman.h>
#include "entry.h"
#include "headers.h"
#include "windef.h"
#include "IntSafe.h"

Status invoke_entry(PeFile* file, uint16_t reason) {
	union {
		void* p;
		BOOL (WINAPI* dll_entry)(HINSTANCE instance, DWORD reason, LPVOID reserved);
		void (WINAPI* exe_entry)(void);
	} u;

#ifdef CONFIG_64BIT
	if (file->x64) {
		u.p = (void*) ((uintptr_t) file->base + file->hdr64->optional_header.header.address_of_entry_point);
		if (file->hdr64->coff_header.characteristics & IMAGE_FILE_DLL) {
			if (!u.dll_entry(file->base, reason, NULL)) return STATUS_LOAD_FAILED;
		}
		else {
			u.exe_entry();
		}
	}
	else {
		return STATUS_BAD_ARCH;
	}
#else
	if (!file->x64) {
		u.p = (void*) ((uintptr_t) file->base + file->hdr32->optional_header.header.address_of_entry_point);
		if (file->hdr32->coff_header.characteristics & IMAGE_FILE_DLL) {
			if (!u.dll_entry(file->base, reason, NULL)) {
				return STATUS_LOAD_FAILED;
			}
		}
		else {
			u.exe_entry();
		}
	}
	else {
		return STATUS_BAD_ARCH;
	}
#endif
	return STATUS_SUCCESS;
}