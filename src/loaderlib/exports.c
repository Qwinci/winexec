#include <string.h>
#include "exports.h"
#include "headers.h"

typedef struct {
	u32 flags;
	u32 time_date_stamp;
	u16 major_version;
	u16 minor_version;
	u32 name;
	u32 ordinal_base;
	u32 number_of_functions;
	u32 number_of_names;
	u32 address_of_functions;
	u32 address_of_names;
	u32 address_of_name_ordinals;
} ExportDirectory;

void* pe_get_proc_addr(PeFile* file, const char* name) {
#ifdef CONFIG_64BIT
	u32 addr = file->hdr64->optional_header.data_directories[IMAGE_DIRECTORY_ENTRY_EXPORT].virtual_address;
#else
	u32 addr = file->hdr32->optional_header.data_directories[IMAGE_DIRECTORY_ENTRY_EXPORT].virtual_address;
#endif
	const ExportDirectory* export = (const ExportDirectory*) ((usize) file->base + addr);

	if ((uintptr_t) name < 0xFFFF) {
		uint16_t ordinal = (uint16_t) (uintptr_t) name;
		const uint32_t* functions = (const uint32_t*) ((uintptr_t) file->base + export->address_of_functions);
		if (ordinal < export->ordinal_base) return NULL;
		uint16_t offset = (uint32_t) ordinal - export->ordinal_base;
		if (offset >= export->number_of_functions) return NULL;
		return (void*) ((uintptr_t) file->base + functions[offset]);
	}

	const uint32_t* names = (const uint32_t*) ((uintptr_t) file->base + export->address_of_names);
	const uint16_t* name_ordinals = (const uint16_t*) ((uintptr_t) file->base + export->address_of_name_ordinals);
	const uint32_t* functions = (const uint32_t*) ((uintptr_t) file->base + export->address_of_functions);

	for (uint32_t i = 0; i < export->number_of_names; ++i) {
		const char* current_name = (const char*) ((uintptr_t) file->base + names[i]);

		if (strcmp(current_name, name) == 0) {
			uint16_t offset = (uint32_t) name_ordinals[i];
			return (void*) ((uintptr_t) file->base + functions[offset]);
		}
	}
	return NULL;
}

void* pe_get_proc_addr_indexed(PeFile* file, const char* name, uint16_t index) {
#ifdef CONFIG_64BIT
	u32 addr = file->hdr64->optional_header.data_directories[IMAGE_DIRECTORY_ENTRY_EXPORT].virtual_address;
#else
	u32 addr = file->hdr32->optional_header.data_directories[IMAGE_DIRECTORY_ENTRY_EXPORT].virtual_address;
#endif
	const ExportDirectory* export = (const ExportDirectory*) ((usize) file->base + addr);

	const uint32_t* names = (const uint32_t*) ((uintptr_t) file->base + export->address_of_names);
	const uint16_t* name_ordinals = (const uint16_t*) ((uintptr_t) file->base + export->address_of_name_ordinals);
	const uint32_t* functions = (const uint32_t*) ((uintptr_t) file->base + export->address_of_functions);

	const char* current_name = (const char*) ((uintptr_t) file->base + names[index]);
	if (strcmp(current_name, name) == 0) {
		uint16_t offset = (uint32_t) name_ordinals[index] - export->ordinal_base;
		return (void*) ((uintptr_t) file->base + functions[offset]);
	}

	return pe_get_proc_addr(file, name);
}