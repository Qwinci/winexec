#include <dlfcn.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include "loader.h"
#include "str.h"
#include "loaderlib.h"
#include "relocs.h"
#include "tls.h"
#include "imports.h"
#include "entry.h"

extern String* IMPORT_PATHS;
extern usize IMPORT_PATHS_LEN;

extern LoadedLib* add_loaded_lib(const LoadedLib* lib);

void* loaderlib_native_open_lib(LoadedLib** res, const char* lib_name, usize lib_name_len) {
	void* lib = NULL;
	for (size_t i = 0; i < IMPORT_PATHS_LEN; ++i) {
		String* path = &IMPORT_PATHS[i];
		size_t len = path->len;

#ifdef CONFIG_64BIT
		const usize total_len = lib_name_len + len + 2;
#else
		const usize total_len = lib_name_len + len + 5;
#endif
		String str = string_new_with_size(path->str, len, total_len);
		string_append(&str, "lib", 3);
		string_append(&str, lib_name, lib_name_len);
		string_strip_postfix(&str, ".dll");
#ifndef CONFIG_64BIT
		string_append(&str, "_w32", 4);
#endif
		string_append(&str, ".so", 3);

#ifdef CONFIG_64BIT
		const usize ignored_chars = 3;
#else
		const usize ignored_chars = 7;
#endif

		lib = dlopen(str.str, RTLD_NOW);
		if (!lib && isupper(lib_name[0])) {
			for (size_t i2 = len + 3; i2 < str.len - ignored_chars; ++i2) str.str[i2] = (char) tolower(str.str[i2]);
			lib = dlopen(str.str, RTLD_NOW);
		}
		else if (!lib && islower(lib_name[0])) {
			for (size_t i2 = len + 3; i2 < str.len - ignored_chars; ++i2) str.str[i2] = (char) toupper(str.str[i2]);
			lib = dlopen(str.str, RTLD_NOW);
		}

		string_free(str);

		if (lib) break;
	}

	if (lib) {
		LoadedLib loaded_lib = {
			.name = string_new(lib_name, lib_name_len),
			.handle = lib,
			.ref_count = 1,
			.type = LOADED_LIB_TYPE_NATIVE
		};
		*res = add_loaded_lib(&loaded_lib);
	}

	return lib;
}

extern LoadedLib* LOADERLIB_LOADED_LIBS;
extern usize LOADERLIB_LOADED_LIBS_LEN;
int LOADERLIB_ARGC = 0;
const char** LOADERLIB_ARGV = NULL;
const char** LOADERLIB_ENVP = NULL;

void loaderlib_init(int argc, const char** argv, const char** envp) {
	LOADERLIB_ARGC = argc;
	LOADERLIB_ARGV = argv;
	LOADERLIB_ENVP = envp;
}

Status load_dll_library(LoadedLib** res, const char* name, usize name_len) {
	int fd = 0;
	String str = string_new_with_size("", 0, name_len);
	for (size_t i = 0; i < IMPORT_PATHS_LEN; ++i) {
		String* dll_folder_path = &IMPORT_PATHS[i];

		string_clear(&str);
		string_append(&str, dll_folder_path->str, dll_folder_path->len);
		string_append(&str, name, name_len);

		fd = open(str.str, O_RDONLY);
		if (fd < 0 && isupper(name[0])) {
			for (size_t i2 = dll_folder_path->len; i2 < str.len - 3; ++i2) {
				str.str[i2] = (char) tolower(str.str[i2]);
			}
			fd = open(str.str, O_RDONLY);
		}
		if (fd < 0 && islower(name[0])) {
			for (size_t i2 = dll_folder_path->len; i2 < str.len - 3; ++i2) {
				str.str[i2] = (char) toupper(str.str[i2]);
			}
			fd = open(str.str, O_RDONLY);
		}

		if (fd >= 0) break;
	}

	if (fd < 0) {
		string_free(str);
		return STATUS_NOT_FOUND;
	}

	struct stat s;
	fstat(fd, &s);
	char* dll_file = (char*) mmap(NULL, s.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (!dll_file) {
		string_free(str);
		return STATUS_NO_MEM;
	}

	string_free(str);

	PeFile* dll = malloc(sizeof(PeFile));
	if (!dll) {
		munmap(dll_file, s.st_size);
		return STATUS_NO_MEM;
	}
	Status status = map_pe(dll_file, s.st_size, dll);
	munmap(dll_file, s.st_size);
	close(fd);
	if (status == STATUS_SUCCESS) {
		fprintf(stderr, "successfully loaded library %s\n", name);
		apply_relocations(dll);
		allocate_tls(dll);

		const char* file_start = strrchr(name, '/');
		usize len;
		if (!file_start) {
			file_start = name;
			len = name_len;
		}
		else {
			file_start += 1;
			len = name_len - (file_start - name);
		}
		LoadedLib loaded_lib = {
			.name = string_new(file_start, len),
			.handle = dll,
			.ref_count = 1,
			.type = LOADED_LIB_TYPE_PE
		};

		*res = add_loaded_lib(&loaded_lib);

		status = process_imports(dll);
		invoke_entry(dll, DLL_PROCESS_ATTACH);
	}

	return status;
}
