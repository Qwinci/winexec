#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include "mapper.h"
#include "headers.h"

#define offset(T, V, A) ((T) ((uintptr_t) V + A))

static inline Status map_pe64(const void* src, const PeHeader64* hdr, PeFile* dest) {
	char* buffer;
	size_t image_size = (hdr->optional_header.size_of_image + 0x1000 - 1) & ~(0x1000 - 1);
	if (hdr->coff_header.characteristics & IMAGE_FILE_RELOCS_STRIPPED) {
		buffer = mmap(
				(void*) hdr->optional_header.image_base,
				image_size,
				PROT_WRITE,
				MAP_FIXED_NOREPLACE | MAP_PRIVATE | MAP_ANONYMOUS,
				-1,
				0);
		if (buffer == MAP_FAILED) {
			fprintf(stderr, "error: failed to allocate %p bytes at addr %p\n", (void*) image_size, (void*) hdr->optional_header.image_base);
			*(volatile int*) 0 = 0;
		}
	}
	else {
		buffer = mmap(
				NULL,
				image_size,
				PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS,
				-1,
				0);
	}

	if (buffer == MAP_FAILED) {
		perror("failed to mmap memory");
		return STATUS_NO_MEM;
	}

	dest->base = buffer;

	if (hdr->optional_header.size_of_headers == 0 ||
		hdr->optional_header.size_of_headers > hdr->optional_header.size_of_image) {
		munmap(buffer, image_size);
		return STATUS_BAD_FORMAT;
	}

	memcpy(buffer, src, hdr->optional_header.size_of_headers);

	const PeSectionHeader* section_hdr =
			offset(const PeSectionHeader*, &hdr->optional_header, hdr->coff_header.size_of_optional_header);

	for (uint16_t i = 0; i < hdr->coff_header.number_of_sections; ++i) {
		int prot =
				(section_hdr->characteristics & IMAGE_SCN_READ ? PROT_READ : 0) |
				(section_hdr->characteristics & IMAGE_SCN_WRITE ? PROT_WRITE : 0) |
				(section_hdr->characteristics & IMAGE_SCN_EXECUTE ? PROT_EXEC : 0);
		void* dest_addr = buffer + section_hdr->virtual_address;
		const void* src_addr = offset(const void*, src, section_hdr->pointer_to_raw_data);

		void* aligned_dest_addr = (void*) ((uintptr_t) dest_addr & ~(0x1000 - 1));
		size_t dest_len = (section_hdr->virtual_size + 0x1000 - 1) & ~(0x1000 - 1);
		mprotect(aligned_dest_addr, dest_len, PROT_WRITE);

		memcpy(dest_addr, src_addr, section_hdr->size_of_raw_data);

		if (section_hdr->virtual_size > section_hdr->size_of_raw_data) {
			memset(
					offset(void*, dest_addr, section_hdr->size_of_raw_data),
					0,
					section_hdr->virtual_size - section_hdr->size_of_raw_data);
		}

		// todo
		//mprotect(aligned_dest_addr, dest_len, prot);
		mprotect(aligned_dest_addr, dest_len, PROT_READ | PROT_WRITE | PROT_EXEC);

		++section_hdr;
	}

	dest->x64 = true;
	dest->hdr64 = (const PeHeader64*) (buffer + ((void*) hdr - src));;

	dest->mapped_size = image_size;

	return STATUS_SUCCESS;
}

static inline Status map_pe32(const void* src, const PeHeader32* hdr, PeFile* dest) {
	char* buffer;
	size_t image_size = (hdr->optional_header.size_of_image + 0x1000 - 1) & ~(0x1000 - 1);
	if (hdr->coff_header.characteristics & IMAGE_FILE_RELOCS_STRIPPED) {
		buffer = mmap(
				(void*) (usize) hdr->optional_header.image_base,
				image_size,
				PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS,
				-1,
				0);
		if (!buffer) return STATUS_NO_MEM;
		else if (buffer != (void*) (usize) hdr->optional_header.image_base) {
			munmap(buffer, image_size);
			return STATUS_NO_MEM;
		}
	}
	else {
		buffer = mmap(
				NULL,
				image_size,
				PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS,
				-1,
				0);
	}

	if (!buffer) return STATUS_NO_MEM;
	dest->base = buffer;

	if (hdr->optional_header.size_of_headers == 0 ||
	    hdr->optional_header.size_of_headers > hdr->optional_header.size_of_image) {
		munmap(buffer, image_size);
		return STATUS_BAD_FORMAT;
	}

	memcpy(buffer, src, hdr->optional_header.size_of_headers);

	const PeSectionHeader* section_hdr =
			offset(const PeSectionHeader*, &hdr->optional_header, hdr->coff_header.size_of_optional_header);

	for (uint16_t i = 0; i < hdr->coff_header.number_of_sections; ++i) {
		int prot =
				(section_hdr->characteristics & IMAGE_SCN_READ ? PROT_READ : 0) |
				(section_hdr->characteristics & IMAGE_SCN_WRITE ? PROT_WRITE : 0) |
				(section_hdr->characteristics & IMAGE_SCN_EXECUTE ? PROT_EXEC : 0);
		void* dest_addr = buffer + section_hdr->virtual_address;
		const void* src_addr = offset(const void*, src, section_hdr->pointer_to_raw_data);

		void* aligned_dest_addr = (void*) ((uintptr_t) dest_addr & ~(0x1000 - 1));
		size_t dest_len = (section_hdr->virtual_size + 0x1000 - 1) & ~(0x1000 - 1);
		mprotect(aligned_dest_addr, dest_len, PROT_WRITE);

		memcpy(dest_addr, src_addr, section_hdr->size_of_raw_data);

		if (section_hdr->virtual_size > section_hdr->size_of_raw_data) {
			memset(
					offset(void*, dest_addr, section_hdr->size_of_raw_data),
					0,
					section_hdr->virtual_size - section_hdr->size_of_raw_data);
		}

		// todo
		//mprotect(aligned_dest_addr, dest_len, prot);
		mprotect(aligned_dest_addr, dest_len, PROT_READ | PROT_WRITE | PROT_EXEC);

		++section_hdr;
	}

	dest->x64 = false;
	dest->hdr32 = (const PeHeader32*) (buffer + ((void*) hdr - src));

	dest->mapped_size = image_size;

	return STATUS_SUCCESS;
}

void free_pe(PeFile* file) {
	munmap(file->base, file->mapped_size);
}

Status map_pe(const void* data, size_t data_size, PeFile* dest) {
	*dest = (PeFile) {};

	if (!data || data_size < sizeof(DosHeader)) return STATUS_BAD_FORMAT;

	DosHeader* dos_hdr = (DosHeader*) data;
	if (dos_hdr->e_magic != DOS_MAGIC || (u32) dos_hdr->e_lfanew > data_size) return STATUS_BAD_FORMAT;

	const PeHeader* pe_hdr = offset(const PeHeader*, data, dos_hdr->e_lfanew);
	if (pe_hdr->signature != IMAGE_NT_SIGNATURE) return STATUS_BAD_FORMAT;

	if (pe_hdr->coff_header.machine != IMAGE_FILE_MACHINE_AMD64 &&
		pe_hdr->coff_header.machine != IMAGE_FILE_MACHINE_I386) return STATUS_BAD_ARCH;

	if (pe_hdr->optional_header.magic == IMAGE_OPTIONAL_PE64_MAGIC) {
#ifdef CONFIG_64BIT
		return map_pe64(data, (const PeHeader64*) pe_hdr, dest);
#else
		return STATUS_BAD_ARCH;
#endif
	}
	else if (pe_hdr->optional_header.magic == IMAGE_OPTIONAL_PE32_MAGIC) {
#ifdef CONFIG_64BIT
		return STATUS_BAD_ARCH;
#else
		return map_pe32(data, (const PeHeader32 *) pe_hdr, dest);
#endif
	}
	else return STATUS_BAD_FORMAT;
}