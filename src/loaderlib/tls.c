#include <stdio.h>
#include "tls.h"
#include "headers.h"

typedef struct {
	usize raw_data_start_va;
	usize raw_data_end_va;
	usize addr_of_index;
	usize addr_of_callbacks;
	u32 size_of_zero_fill;
	u32 characteristics;
} TlsDir;

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
	if (tls->raw_data_end_va - tls->raw_data_start_va + tls->size_of_zero_fill > 0) {
		fprintf(stderr, "todo: implement tls\n");
		exit(EXIT_FAILURE);
	}
}