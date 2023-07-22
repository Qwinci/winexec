#include "relocs.h"
#include "headers.h"

typedef struct {
	uint32_t page_rva;
	uint32_t block_size;
} BaseRelocEntry;

#define IMAGE_REL_ABSOLUTE 0
#define IMAGE_REL_BASED_HIGH 1
#define IMAGE_REL_BASED_LOW 2
#define IMAGE_REL_BASED_HIGHLOW 3
#define IMAGE_REL_BASED_HIGHADJ 4
#define IMAGE_REL_BASED_DIR64 10

void apply_relocations(PeFile* file) {
	const DataDirectory* base_reloc_dir;
	if (!file->x64) {
		base_reloc_dir = &file->hdr32->optional_header.data_directories[IMAGE_DIRECTORY_ENTRY_BASERELOC];
	}
	else {
		base_reloc_dir = &file->hdr64->optional_header.data_directories[IMAGE_DIRECTORY_ENTRY_BASERELOC];
	}
	const BaseRelocEntry* relocs = (const BaseRelocEntry*) ((uintptr_t) file->base + base_reloc_dir->virtual_address);
	uint32_t total_size = base_reloc_dir->size;
	uintptr_t delta = (uintptr_t) file->base -
			(file->x64 ? file->hdr64->optional_header.image_base : file->hdr32->optional_header.image_base);

	if (!delta) {
		return;
	}

	for (uint32_t i = 0; i < total_size; relocs = (const BaseRelocEntry*) ((uintptr_t) relocs + relocs->block_size)) {
		uint32_t size = relocs->block_size;
		i += size;
		usize block_start = relocs->page_rva;

		size -= sizeof(BaseRelocEntry);

		size /= sizeof(uint16_t);

		const uint16_t* entry = (const uint16_t*) ((uintptr_t) relocs + sizeof(BaseRelocEntry));

		for (uint32_t j = 0; j < size; ++j) {
			uint8_t type = (uint8_t) (entry[j] >> 12);
			uint16_t offset = entry[j] & 0xFFF;
			void* reloc_loc = (void*) ((uintptr_t) file->base + block_start + offset);

			switch (type) { // NOLINT(hicpp-multiway-paths-covered)
				case IMAGE_REL_BASED_LOW:
					*(uint16_t*) reloc_loc += (uint32_t) delta & 0xFFFF;
					break;
				case IMAGE_REL_BASED_HIGH:
					*(uint16_t*) reloc_loc += (uint32_t) delta >> 16;
					break;
				case IMAGE_REL_BASED_HIGHLOW:
					*(uint32_t*) reloc_loc += (int32_t) delta;
					break;
				case IMAGE_REL_BASED_DIR64:
					*(uint64_t*) reloc_loc += (i64) delta;
			}
		}
	}
}