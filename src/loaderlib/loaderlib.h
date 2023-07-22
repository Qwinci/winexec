#pragma once
#include "mapper.h"

typedef enum {
	LOADED_LIB_TYPE_PE,
	LOADED_LIB_TYPE_NATIVE
} LoadedLibType;

typedef struct {
	String name;
	HANDLE handle;
	usize ref_count;
	LoadedLibType type;
} LoadedLib;

extern int LOADERLIB_ARGC;
extern const char** LOADERLIB_ARGV;
extern const char** LOADERLIB_ENVP;

void loaderlib_init(int argc, const char** argv, const char** envp);
/// Gets a loaded library based on name, name doesn't include .dll
LoadedLib* loaderlib_get_lib(const char* name, usize name_len);
HANDLE loaderlib_get_proc_addr(LoadedLib* lib, const char* name);
Status loaderlib_load_lib(LoadedLib** res, const char* name, usize name_len);
void loaderlib_unload_lib(LoadedLib* lib);
void loaderlib_add_import_path(const char* path);

Status loaderlib_execute(PeFile* out,
                         const char* name,
                         int argc,
                         const char** argv,
                         const char** envp);
