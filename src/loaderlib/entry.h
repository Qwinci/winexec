#pragma once
#include "mapper.h"
#include <stdint.h>

#define DLL_PROCESS_ATTACH 1
#define DLL_PROCESS_DETACH 0
#define DLL_THREAD_ATTACH 2
#define DLL_THREAD_DETACH 3

Status invoke_entry(PeFile* file, uint16_t reason);