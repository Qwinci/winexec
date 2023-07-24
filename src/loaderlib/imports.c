#include "imports.h"
#include "headers.h"
#include "relocs.h"
#include "exports.h"
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <ctype.h>
#include <unistd.h>
#include "str.h"
#include "lib/common.h"
#include "loaderlib.h"
#include "tls.h"
#include "entry.h"
#include "loader.h"

typedef struct {
	uint32_t import_lookup_table_rva;
	uint32_t time_date_stamp;
	uint32_t forwarder_chain;
	uint32_t name_rva;
	uint32_t import_address_table_rva;
} ImportEntry;

#define IMAGE_ORDINAL_FLAG_64 (1ULL << 63)
#define IMAGE_ORDINAL_FLAG_32 (1ULL << 31)

Status process_imports(PeFile* file) { // NOLINT(misc-no-recursion)
	const DataDirectory* dir = file->x64 ?
	                           &file->hdr64->optional_header.data_directories[IMAGE_DIRECTORY_ENTRY_IMPORT] :
							   &file->hdr32->optional_header.data_directories[IMAGE_DIRECTORY_ENTRY_IMPORT];

	if (!dir->size) {
		return STATUS_SUCCESS;
	}

	const DataDirectory* iat_dir = file->x64 ?
	                               &file->hdr64->optional_header.data_directories[IMAGE_DIRECTORY_ENTRY_IAT] :
	                               &file->hdr32->optional_header.data_directories[IMAGE_DIRECTORY_ENTRY_IAT];

	ImportEntry* import = (ImportEntry*) ((uintptr_t) file->base + dir->virtual_address);

	usize size = iat_dir->size + 0x1000;
	usize base = iat_dir->virtual_address & ~(0x1000 - 1);
	for (usize i = 0; i < size; i += 0x1000) {
		mprotect((void*) ((uintptr_t) file->base + base + i), 0x1000, PROT_READ | PROT_WRITE);
	}

	Status status = STATUS_SUCCESS;

	for (; import->import_lookup_table_rva; ++import) {
		const char* library_name = (const char*) ((uintptr_t) file->base + import->name_rva);
		size_t library_name_len = strlen(library_name);
		const char* prefix = NULL;

		LoadedLib* loaded_lib = loaderlib_get_lib(library_name, library_name_len - 4);
		void* native_lib = NULL;
		if (!loaded_lib) {
			Status s = load_dll_library(&loaded_lib, library_name, library_name_len);
			if (s == STATUS_NOT_FOUND) {
				LoadedLib* lib;
				native_lib = loaderlib_native_open_lib(&lib, library_name, library_name_len);
			}
		}

		if (!loaded_lib && !native_lib) {
			fprintf(stderr, "library %s was not found\n", library_name);
			status = STATUS_NOT_FOUND;
			continue;
		}

		if (strcasecmp(library_name, "msvcrt.dll") == 0) {
			prefix = "win_";
			void (*set_args)(int argc, char** argv, char** envp) = (void (*)(int, char**, char**)) dlsym(native_lib, "set_args");
			set_args(LOADERLIB_ARGC, (char**) LOADERLIB_ARGV, (char**) LOADERLIB_ENVP);
		}
		else if (strcasecmp(library_name, "kernel32.dll") == 0) {
			*(const char**) dlsym(native_lib, "EXE_PATH") = file->exe_path;
		}

		if (file->x64) {
			uint64_t* entry = (uint64_t*) ((uintptr_t) file->base + import->import_address_table_rva);

			for (; *entry; ++entry) {
				void* fn = NULL;
				if (*entry & IMAGE_ORDINAL_FLAG_64) {
					uint16_t ordinal = *entry & 0xFFFF;
					if (native_lib) {
						Ordinals* ordinals = dlsym(native_lib, "g_ordinals");
						if (ordinals) {
							for (usize i = 0; i < ordinals->count; ++i) {
								Ordinal e = ordinals->ptr[i];
								if (e.num == ordinal) {
									fn = e.fn;
									break;
								}
							}
						}
						if (!fn) {
							fprintf(stderr, "error: attempting to import by ordinal from non dll file %s (%u)\n",
							        library_name, ordinal);
							return STATUS_INVALID;
						}
					}
					else {
						fn = pe_get_proc_addr((PeFile*) loaded_lib->handle, (const char*) (uintptr_t) ordinal);
					}
				}
				else {
					uint16_t hint = *(uint16_t*) ((uintptr_t) file->base + (*entry & 0xFFFFFFFF));
					const char* name = (const char*) file->base + (*entry & 0xFFFFFFFF) + 2;

					if (loaded_lib) {
						fn = pe_get_proc_addr_indexed((PeFile*) loaded_lib->handle, name, hint);
					}
					else {
						if (prefix) {
							usize len = strlen(name);
							String n = string_new_with_size(name, len, len + 4);
							string_prepend(&n, prefix, 4);
							fn = dlsym(native_lib, n.str);
							string_free(n);
						}
						else fn = dlsym(native_lib, name);
					}

					if (!fn) {
						fprintf(stderr, "info: import %s from %s\n", name, library_name);
						status = STATUS_NOT_FOUND;
					}
				}

				*entry = (uint64_t) fn;
			}
		}
		else {
			const uint32_t* orig_entry = (const uint32_t*) ((uintptr_t) file->base + import->import_lookup_table_rva);
			uint32_t* entry = (uint32_t*) ((uintptr_t) file->base + import->import_address_table_rva);

			for (; *orig_entry; ++orig_entry, ++entry) {
				void* fn = NULL;
				if (*orig_entry & IMAGE_ORDINAL_FLAG_32) {
					uint16_t ordinal = *entry & 0xFFFF;
					if (native_lib) {
						Ordinals* ordinals = dlsym(native_lib, "g_ordinals");
						if (ordinals) {
							for (usize i = 0; i < ordinals->count; ++i) {
								Ordinal e = ordinals->ptr[i];
								if (e.num == ordinal) {
									fn = e.fn;
									break;
								}
							}
						}

						if (!fn) {
							fprintf(stderr, "error: attempting to import by ordinal from non dll file %s (%u)\n",
							        library_name, ordinal);
							return STATUS_INVALID;
						}
					}
					else {
						fn = pe_get_proc_addr((PeFile*) loaded_lib->handle, (const char*) (uintptr_t) ordinal);
					}
				}
				else {
					uint16_t hint = *(uint16_t*) ((uintptr_t) file->base + (*orig_entry & 0xFFFFFFFF));
					const char* name = (const char*) file->base + (*orig_entry & 0xFFFFFFFF) + 2;

					if (loaded_lib) {
						fn = pe_get_proc_addr_indexed((PeFile*) loaded_lib->handle, name, hint);
					}
					else {
						if (prefix) {
							usize len = strlen(name);
							String n = string_new_with_size(name, len, len + 4);
							string_prepend(&n, prefix, 4);
							fn = dlsym(native_lib, n.str);
							string_free(n);
						}
						else {
							fn = dlsym(native_lib, name);
						}
					}

					if (!fn) {
						fprintf(stderr, "info: import %s from %s\n", name, library_name);
						status = STATUS_NOT_FOUND;
					}
				}

				*entry = (uint64_t) fn;
			}
		}
	}

	// todo
	/*for (usize i = 0; i < size; i += 0x1000) {
		mprotect((void*) ((uintptr_t) file->base + base + i), 0x1000, PROT_READ);
	}*/

	return status;
}