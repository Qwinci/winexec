#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include "mapper.h"
#include "imports.h"
#include "relocs.h"
#include "tls.h"
#include "entry.h"
#include "exports.h"
#include "loaderlib.h"
#include "loader.h"
#include "common.h"

LoadedLib* LOADERLIB_LOADED_LIBS = NULL;
usize LOADERLIB_LOADED_LIBS_LEN = 0;
static usize LOADERLIB_LOADED_LIBS_CAP = 0;

String* IMPORT_PATHS = NULL;
usize IMPORT_PATHS_LEN = 0;
static usize IMPORT_PATHS_CAP = 0;

LoadedLib* add_loaded_lib(const LoadedLib* lib) {
	if (LOADERLIB_LOADED_LIBS_LEN == LOADERLIB_LOADED_LIBS_CAP) {
		LOADERLIB_LOADED_LIBS_CAP = LOADERLIB_LOADED_LIBS_CAP < 8 ? 8 : (LOADERLIB_LOADED_LIBS_CAP * 2);
		LOADERLIB_LOADED_LIBS = realloc(LOADERLIB_LOADED_LIBS, LOADERLIB_LOADED_LIBS_CAP * sizeof(LoadedLib));
	}

	LOADERLIB_LOADED_LIBS[LOADERLIB_LOADED_LIBS_LEN++] = *lib;
	return &LOADERLIB_LOADED_LIBS[LOADERLIB_LOADED_LIBS_LEN - 1];
}

void loaderlib_add_import_path(const char* path) {
	if (IMPORT_PATHS_LEN == IMPORT_PATHS_CAP) {
		IMPORT_PATHS_CAP = IMPORT_PATHS_CAP < 8 ? 8 : (IMPORT_PATHS_CAP * 2);
		IMPORT_PATHS = realloc(IMPORT_PATHS, IMPORT_PATHS_CAP * sizeof(String));
	}

	IMPORT_PATHS[IMPORT_PATHS_LEN++] = string_new(path, strlen(path));
}

LoadedLib* loaderlib_get_lib(const char* name, usize name_len) {
	for (usize i = 0; i < LOADERLIB_LOADED_LIBS_LEN; ++i) {
		LoadedLib* lib = &LOADERLIB_LOADED_LIBS[i];
		if (lib->name.len == name_len && strncasecmp(name, lib->name.str, name_len) == 0) {
			return lib;
		}
	}
	return NULL;
}

HANDLE loaderlib_get_proc_addr(LoadedLib* lib, const char* name) {
	if (lib->type == LOADED_LIB_TYPE_PE) {
		return pe_get_proc_addr((PeFile*) lib->handle, name);
	}
	else {
		if (name < (const char*) 0xFFFF) {
			u16 ordinal = (u16) (usize) name;
			Ordinals* ordinals = dlsym(lib->handle, "g_ordinals");
			if (ordinals) {
				for (usize i = 0; i < ordinals->count; ++i) {
					Ordinal e = ordinals->ptr[i];
					if (e.num == ordinal) {
						return e.fn;
					}
				}
			}
			fprintf(stderr, "import ordinal %u from %s\n", ordinal, lib->name.str);
			return NULL;
		}

		return dlsym(lib->handle, name);
	}
}

Status loaderlib_load_lib(LoadedLib** res, const char* name, usize name_len) {
	Status s = load_dll_library(res, name, name_len);
	if (s == STATUS_NOT_FOUND) {
		if (loaderlib_native_open_lib(res, name, name_len)) {
			return STATUS_SUCCESS;
		}
	}

	return s;
}

void loaderlib_unload_lib(LoadedLib* lib) {
	// todo
}

Status loaderlib_execute(PeFile* out,
						 const char* name,
						 int argc,
						 const char** argv,
						 const char** envp) {
	loaderlib_init(argc, argv, envp);

	int fd = open(name, O_RDONLY);
	if (fd < 0) {
		return STATUS_NOT_FOUND;
	}
	struct stat s;
	fstat(fd, &s);
	char* file = (char*) mmap(NULL, s.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (!file) {
		return STATUS_NO_MEM;
	}

	Status status = map_pe(file, s.st_size, out);
	out->exe_path = name;
	if (status == STATUS_SUCCESS) {
		apply_relocations(out);
		allocate_tls(out);
		status = process_imports(out);
		if (status != STATUS_SUCCESS) {
			return STATUS_IMPORT_NOT_FOUND;
		}

		invoke_entry(out, DLL_PROCESS_ATTACH);
		return STATUS_SUCCESS;
	}
	else {
		return status;
	}
}
