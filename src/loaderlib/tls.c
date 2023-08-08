#include <stdio.h>
#include "tls.h"
#include "headers.h"
#include "winternl.h"
#include "entry.h"

#ifdef __linux__
#include <sys/syscall.h>
#include <asm/prctl.h>
#include <unistd.h>
#include <sys/mman.h>

#endif

typedef struct {
	usize raw_data_start_va;
	usize raw_data_end_va;
	usize addr_of_index;
	usize addr_of_callbacks;
	u32 size_of_zero_fill;
	u32 characteristics;
} TlsDir;

static usize TLS_INDEX = 0;

typedef VOID (WINAPI* PIMAGE_TLS_CALLBACK)(PVOID dll_handle, DWORD reason, PVOID reserved);

void allocate_tls(PeFile* file) {
	const DataDirectory* tls_dir;
#ifdef CONFIG_64BIT
	tls_dir = &file->hdr64->optional_header.data_directories[IMAGE_DIRECTORY_ENTRY_TLS];
#else
	tls_dir = &file->hdr32->optional_header.data_directories[IMAGE_DIRECTORY_ENTRY_TLS];
#endif
	if (!tls_dir->size) {
		return;
	}

	const TlsDir* tls = (const TlsDir*) ((usize) file->base + tls_dir->virtual_address);
	void* start = (void*) tls->raw_data_start_va;
	void* end = (void*) tls->raw_data_end_va;

	TIB* tib;
#ifdef __linux__
	syscall(SYS_arch_prctl, ARCH_GET_GS, &tib);
#endif

	assert(TLS_INDEX < 64);

	TEB* teb = tib->teb;

	usize size = tls->raw_data_end_va - tls->raw_data_start_va + tls->size_of_zero_fill;

	void* memory = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
	assert(memory);

	usize index = TLS_INDEX++;

	teb->TlsSlots[index] = memory;

	memcpy(memory, (const void*) tls->raw_data_start_va, tls->raw_data_end_va - tls->raw_data_start_va);

	*(DWORD*) tls->addr_of_index = index;

	PIMAGE_TLS_CALLBACK* callback = (PIMAGE_TLS_CALLBACK*) tls->addr_of_callbacks;
	for (; *callback; ++callback) {
		(*callback)(file->base, DLL_PROCESS_ATTACH, NULL);
	}
}