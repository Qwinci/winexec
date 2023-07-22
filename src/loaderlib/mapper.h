#pragma once
#include <stddef.h>
#include "str.h"
#include "../kernel32/kernel32.h"

typedef enum {
	STATUS_SUCCESS,
	STATUS_BAD_FORMAT,
	STATUS_BAD_ARCH,
	STATUS_NO_MEM,
	STATUS_INVALID,
	STATUS_UNSUPPORTED,
	STATUS_NOT_FOUND,
	STATUS_LOAD_FAILED,
	STATUS_IMPORT_NOT_FOUND
} Status;

static inline const char* status_to_str(Status status) {
	switch (status) {
		case STATUS_SUCCESS:
			return "STATUS_SUCCESS";
		case STATUS_BAD_FORMAT:
			return "STATUS_BAD_FORMAT";
		case STATUS_BAD_ARCH:
			return "STATUS_BAD_ARCH";
		case STATUS_NO_MEM:
			return "STATUS_NO_MEM";
		case STATUS_INVALID:
			return "STATUS_INVALID";
		case STATUS_UNSUPPORTED:
			return "STATUS_UNSUPPORTED";
		case STATUS_NOT_FOUND:
			return "STATUS_NOT_FOUND";
		case STATUS_LOAD_FAILED:
			return "STATUS_LOAD_FAILED";
		case STATUS_IMPORT_NOT_FOUND:
			return "STATUS_IMPORT_NOT_FOUND";
			break;
	}
}

typedef struct {
	void* base;
	size_t mapped_size;
	bool x64;
	union {
		const struct PeHeader32* hdr32;
		const struct PeHeader64* hdr64;
	};
	const char* exe_path;
} PeFile;

Status map_pe(const void* data, size_t data_size, PeFile* dest);
void free_pe(PeFile* file);
