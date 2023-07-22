#pragma once
#include "types.h"
#include "loaderlib.h"

void* loaderlib_native_open_lib(LoadedLib** res, const char* lib_name, usize lib_name_len);
Status load_dll_library(LoadedLib** res, const char* name, usize name_len);
