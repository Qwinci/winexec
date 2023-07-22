#pragma once
#include "types.h"
#include <assert.h>

#define IMAGE_NT_SIGNATURE 0x00004550
#define DOS_MAGIC 0x5A4D

typedef struct {
	u16 e_magic;
	u16 e_cblp;
	u16 e_cp;
	u16 e_crlc;
	u16 e_cparhdr;
	u16 e_minalloc;
	u16 e_maxalloc;
	u16 e_ss;
	u16 e_sp;
	u16 e_csum;
	u16 e_ip;
	u16 e_cs;
	u16 e_lfarlc;
	u16 e_ovno;
	u16 e_res[4];
	u16 e_oemid;
	u16 e_oeminfo;
	u16 e_res2[10];
	i32 e_lfanew;
} DosHeader;

static_assert(sizeof(DosHeader) == 64);

#define IMAGE_FILE_MACHINE_AMD64 0x8664
#define IMAGE_FILE_MACHINE_I386 0x14C

#define IMAGE_FILE_RELOCS_STRIPPED 0x0001
#define IMAGE_FILE_EXECUTABLE_IMAGE 0x0002
#define IMAGE_FILE_LARGE_ADDRESS_AWARE 0x0020
#define IMAGE_FILE_SYSTEM 0x1000
#define IMAGE_FILE_DLL 0x2000

typedef struct {
	u16 machine;
	u16 number_of_sections;
	u32 time_date_stamp;
	u32 pointer_to_symbol_table;
	u32 number_of_symbols;
	u16 size_of_optional_header;
	u16 characteristics;
} CoffHeader;

static_assert(sizeof(CoffHeader) == 20);

#define IMAGE_OPTIONAL_PE32_MAGIC 0x10B
#define IMAGE_OPTIONAL_PE64_MAGIC 0x20B

typedef struct {
	u16 magic;
	u8 major_linker_version;
	u8 minor_linker_version;
	u32 size_of_code;
	u32 size_of_initialized_data;
	u32 size_of_uninitialized_data;
	u32 address_of_entry_point;
	u32 base_of_code;
} PeStandardOptionalHeader;

static_assert(sizeof(PeStandardOptionalHeader) == 24);

#define IMAGE_DIRECTORY_ENTRY_EXPORT 0
#define IMAGE_DIRECTORY_ENTRY_IMPORT 1
#define IMAGE_DIRECTORY_ENTRY_RESOURCE 2
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION 3
#define IMAGE_DIRECTORY_ENTRY_CERTIFICATE 4
#define IMAGE_DIRECTORY_ENTRY_BASERELOC 5
#define IMAGE_DIRECTORY_ENTRY_DEBUG 6
#define IMAGE_DIRECTORY_ENTRY_GLOBAL_PTR 8
#define IMAGE_DIRECTORY_ENTRY_TLS 9
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG 10
#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT 11
#define IMAGE_DIRECTORY_ENTRY_IAT 12
#define IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT_DESCRIPTOR 13
#define IMAGE_DIRECTORY_ENTRY_CLR_RUNTIME_HEADER 14

typedef struct {
	u32 virtual_address;
	u32 size;
} DataDirectory;

#define IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA 0x0020
#define IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE 0x0040
#define IMAGE_DLLCHARACTERISTICS_NX_COMPAT 0x0100
#define IMAGE_DLLCHARACTERISTICS_NO_BIND 0x0800

typedef struct {
	PeStandardOptionalHeader header;
	u32 base_of_data;
	u32 image_base;
	u32 section_alignment;
	u32 file_alignment;
	u16 major_os_version;
	u16 minor_os_version;
	u16 major_image_version;
	u16 minor_image_version;
	u16 major_subsystem_version;
	u16 minor_subsystem_version;
	u32 win32_version_value;
	u32 size_of_image;
	u32 size_of_headers;
	u32 checksum;
	u16 subsystem;
	u16 dll_characteristics;
	u32 size_of_stack_reserve;
	u32 size_of_stack_commit;
	u32 size_of_heap_reserve;
	u32 size_of_heap_commit;
	u32 loader_flags;
	u32 num_of_rva_and_sizes;
	DataDirectory data_directories[];
} PeOptionalHeader32;

typedef struct {
	PeStandardOptionalHeader header;
	u64 image_base;
	u32 section_alignment;
	u32 file_alignment;
	u16 major_os_version;
	u16 minor_os_version;
	u16 major_image_version;
	u16 minor_image_version;
	u16 major_subsystem_version;
	u16 minor_subsystem_version;
	u32 win32_version_value;
	u32 size_of_image;
	u32 size_of_headers;
	u32 checksum;
	u16 subsystem;
	u16 dll_characteristics;
	u64 size_of_stack_reserve;
	u64 size_of_stack_commit;
	u64 size_of_heap_reserve;
	u64 size_of_heap_commit;
	u32 loader_flags;
	u32 num_of_rva_and_sizes;
	DataDirectory data_directories[];
} PeOptionalHeader64;

typedef struct {
	uint32_t signature;
	CoffHeader coff_header;
	PeStandardOptionalHeader optional_header;
} PeHeader;

typedef struct PeHeader64 {
	uint32_t signature;
	CoffHeader coff_header;
	PeOptionalHeader64 optional_header;
} PeHeader64;

typedef struct PeHeader32 {
	uint32_t signature;
	CoffHeader coff_header;
	PeOptionalHeader32 optional_header;
} PeHeader32;

#define IMAGE_SCN_CNT_CODE 0x20
#define IMAGE_SCN_CNT_INITIALIZED_DATA 0x40
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA 0x80
#define IMAGE_SCN_GPREF 0x8000
#define IMAGE_SCN_LNK_NRELOC_OVFL 0x1000000
#define IMAGE_SCN_MEM_DISCARDABLE 0x2000000
#define IMAGE_SCN_NOT_CACHED 0x4000000
#define IMAGE_SCN_NOT_PAGED 0x8000000
#define IMAGE_SCN_SHARED 0x10000000
#define IMAGE_SCN_EXECUTE 0x20000000
#define IMAGE_SCN_READ 0x40000000
#define IMAGE_SCN_WRITE 0x80000000

typedef struct {
	char name[8];
	u32 virtual_size;
	u32 virtual_address;
	u32 size_of_raw_data;
	u32 pointer_to_raw_data;
	u32 pointer_to_relocations;
	u32 pointer_to_line_numbers;
	u16 number_of_relocations;
	u16 number_of_line_numbers;
	u32 characteristics;
} PeSectionHeader;
