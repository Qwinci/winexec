#pragma once
#include "mapper.h"
#include <stdint.h>

void* pe_get_proc_addr(PeFile* file, const char* name);
void* pe_get_proc_addr_indexed(PeFile* file, const char* name, uint16_t index);