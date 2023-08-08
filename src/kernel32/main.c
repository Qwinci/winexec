#include "minwinbase.h"
#include "types.h"
#include "errhandlingapi.h"
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <string.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "common.h"
#include "kernel32.h"
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#define __USE_XOPEN_EXTENDED
#include <ftw.h>
#include <threads.h>
#include <glob.h>

STUB(DeleteCriticalSection)
STUB(EnterCriticalSection)
STUB(GetCurrentProcess)

typedef struct {
	DWORD n_length;
	LPVOID lp_security_descriptor;
	BOOL b_inherit_handle;
} SECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

WINAPI DWORD GetCurrentProcessId() {
	return getpid();
}

WINAPI DWORD GetCurrentThreadId() {
	return pthread_self();
}

#define ERROR_ALREADY_EXISTS 0xB7
#define ERROR_PATH_NOT_FOUND 0x3

static thread_local DWORD error = 0;

WINAPI DWORD GetLastError() {
	return error;
}

STUB(GetStartupInfoA)

static void unix_to_filetime(LPFILETIME filetime, const struct timespec* time) {
	u64 secs = time->tv_sec;
	u64 value = (secs * 10000000LL) + 116444736000000000LL + (time->tv_nsec / 100);
	filetime->dwLowDateTime = value & 0xFFFFFFFF;
	filetime->dwHighDateTime = value >> 32;
}

WINAPI void GetSystemTimeAsFileTime(LPFILETIME lpSystemTimeAsFileTime) {
	time_t t;
	time(&t);
	struct timespec s = {
		.tv_sec = t,
		.tv_nsec = 0
	};
	unix_to_filetime(lpSystemTimeAsFileTime, &s);
}

WINAPI DWORD GetTickCount() {
	struct sysinfo info;
	sysinfo(&info);
	return info.uptime * 1000;
}

WINAPI void InitializeCriticalSection(LPCRITICAL_SECTION lp_critical_section) {
	TODO;
}

STUB(LeaveCriticalSection)

WINAPI BOOL QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount) {
	struct timespec t = {};
	int status = clock_gettime(CLOCK_MONOTONIC, &t);
	lpPerformanceCount->QuadPart = (LONGLONG) (t.tv_sec * 1000000000ULL + t.tv_nsec);
	return status == 0;
}

STUB(RtlAddFunctionTable)
STUB(RtlCaptureContext)
STUB(RtlLookupFunctionEntry)
STUB(RtlVirtualUnwind)

static LPTOP_LEVEL_EXCEPTION_FILTER current_filter = NULL;

LPTOP_LEVEL_EXCEPTION_FILTER SetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter) {
	LPTOP_LEVEL_EXCEPTION_FILTER prev = current_filter;
	current_filter = lpTopLevelExceptionFilter;
	return prev;
}

STUB(Sleep)
STUB(TerminateProcess)
STUB(TlsGetValue)
STUB(UnhandledExceptionFilter)

WINAPI BOOL VirtualProtect(LPVOID lp_address, SIZE_T dw_size, DWORD fl_new_protect, LPDWORD lp_fp_old_protect) {
	TODO;
	return 1;
}

STUB(VirtualQuery)

STUB(GetACP)
STUB(GetExitCodeProcess)
STUB(LocalFree)

WINAPI BOOL CloseHandle(HANDLE h_object) {
	TODO;
	return 0;
}

STUB(SizeofResource)
STUB(VirtualFree)
STUB(GetFullPathNameW)

WINAPI void ExitProcess(UINT u_exit_code) {
	exit((int) u_exit_code);
}

#define HEAP_ZERO_MEMORY 0x8

WINAPI LPVOID HeapAlloc(HANDLE h_heap, DWORD dw_flags, SIZE_T dw_bytes) {
	TODO;
	if (dw_flags & HEAP_ZERO_MEMORY) {
		return calloc(1, dw_bytes);
	}
	else {
		return malloc(dw_bytes);
	}
}

STUB(GetCPInfoExW)
STUB(RtlUnwind)
STUB(GetCPInfo)
STUB(GetStdHandle)

#include "utils.h"
#include "../loaderlib/loaderlib.h"

WINAPI HMODULE GetModuleHandleW(LPCWSTR lp_module_name) {
	if (!lp_module_name) {
		// todo exe file handle
		return NULL;
	}

	String module_utf8 = utf16_to_str(lp_module_name);
	string_strip_postfix(&module_utf8, ".dll");
	string_strip_postfix(&module_utf8, ".DLL");

	LoadedLib* lib = loaderlib_get_lib(module_utf8.str, module_utf8.len);
	string_free(module_utf8);
	return lib;
}

WINAPI BOOL FreeLibrary(HMODULE h_lib_module) {
	TODO;
	return 1;
}

STUB(HeapDestroy)

typedef struct {
	ULONG_PTR internal;
	ULONG_PTR internal_high;
	union {
		struct {
			DWORD offset;
			DWORD offset_high;
		};
		PVOID pointer;
	};
	HANDLE h_event;
} OVERLAPPED, *LPOVERLAPPED;

WINAPI BOOL ReadFile(
	HANDLE h_file,
	LPVOID lp_buffer,
	DWORD n_number_of_bytes_to_read,
	LPDWORD lp_number_of_bytes_read,
	LPOVERLAPPED lp_overlapped) {
	int fd = (int) (uintptr_t) h_file;
	ssize_t r = read(fd, lp_buffer, n_number_of_bytes_to_read);
	if (r == -1) {
		return 0;
	}
	*lp_number_of_bytes_read = (DWORD) r;
	return 1;
}

STUB(CreateProcessW)

const char* EXE_PATH = NULL;

WINAPI DWORD GetModuleFileNameW(HMODULE h_module, LPWSTR lp_filename, DWORD n_size) {
	if (h_module == NULL) {
		usize len = n_size;
		usize total_written = 0;
		const char* s = EXE_PATH;
		while (true) {
			if (len == 1) {
				*lp_filename = 0;
				return n_size;
			}

			u8 count = 0;
			u32 cp = utf8_decode(s, &count);
			s += count;
			if (cp == '/') {
				cp = '\\';
			}
			u8 written = utf16_encode(lp_filename, cp, len > UCHAR_MAX ? UCHAR_MAX : len);
			len -= written;
			lp_filename += written;
			if (cp == 0) {
				break;
			}
			total_written += written;
			if (written == 0) {
				*lp_filename = 0;
				return n_size;
			}
		}
		return total_written;
	}

	LoadedLib* lib = (LoadedLib*) h_module;
	usize len = n_size;
	const char* s = lib->name.str;
	usize total_written = 0;
	while (true) {
		if (len == 1) {
			*lp_filename = 0;
			return n_size;
		}

		u8 count = 0;
		u32 cp = utf8_decode(s, &count);
		s += count;
		u8 written = utf16_encode(lp_filename, cp, len > UCHAR_MAX ? UCHAR_MAX : len);
		len -= written;
		lp_filename += written;
		if (cp == 0) {
			break;
		}
		total_written += written;
		if (written == 0) {
			*lp_filename = 0;
			return n_size;
		}
	}
	return total_written;
}

STUB(SetLastError)
STUB(FindResourceW)
STUB(CreateThread)
STUB(CompareStringW)
STUB(LoadLibraryA)
STUB(ResetEvent)

WINAPI DWORD GetVersion() {
	return 0x23F00206;
	//return 0x11 | 0x0 << 8;
}

STUB(RaiseException)
STUB(FormatMessageW)
STUB(SwitchToThread)
STUB(GetExitCodeThread)
STUB(GetCurrentThread)

#include "../loaderlib/loaderlib.h"

WINAPI HMODULE LoadLibraryExW(LPCWSTR lp_lib_file_name, HANDLE h_file, DWORD dw_flags) {
	String name = utf16_to_str(lp_lib_file_name);
	string_strip_prefix(&name, "C:\\");
	string_replace_all_c(&name, '\\', '/');
	string_strip_prefix(&name, "Windows/System32/");
	if (!string_ends_with(name, ".dll")) {
		string_append(&name, ".dll", 4);
	}

	const char* file_start = strrchr(name.str, '/');
	usize len;
	if (!file_start) {
		file_start = name.str;
		len = name.len;
	}
	else {
		file_start += 1;
		len = name.len - (file_start - name.str);
	}
	LoadedLib* lib = loaderlib_get_lib(file_start, len);
	if (!lib) {
		if (loaderlib_load_lib(&lib, name.str, name.len) == STATUS_SUCCESS) {
			string_free(name);
			return lib;
		}
		fprintf(stderr, "LoadLibraryExW: library %s not found\n", name.str);
	}

	string_free(name);
	return lib;
}

STUB(LockResource)
STUB(VirtualQueryEx)

#define FILE_BEGIN 0
#define FILE_CURRENT 1
#define FILE_END 2

#define INVALID_SET_FILE_POINTER ((DWORD) -1)

WINAPI DWORD SetFilePointer(HANDLE h_file, LONG l_distance_to_move, PLONG lp_distance_to_move_high, DWORD dw_move_method) {
	i64 value = (i64) l_distance_to_move;
	if (lp_distance_to_move_high) {
		value |= (i64) *lp_distance_to_move_high << 32;
	}

	int fd = (int) (uintptr_t) h_file;

	off_t new_pos = lseek(fd, value, (int) dw_move_method);
	if (new_pos == (off_t) -1) {
		return INVALID_SET_FILE_POINTER;
	}

	if (lp_distance_to_move_high) {
		*lp_distance_to_move_high = (LONG) (new_pos >> 32);
	}
	return (DWORD) new_pos;
}

STUB(LoadResource)
STUB(SuspendThread)

WINAPI DWORD GetFileSize(HANDLE h_file, LPDWORD lp_file_size_high) {
	struct stat s;
	int status = fstat((int) (uintptr_t) h_file, &s);

	if (status != 0) {
		if (lp_file_size_high) {
			*lp_file_size_high = 0;
		}
		return 0;
	}

	if (s.st_size > UINT32_MAX) {
		*lp_file_size_high = s.st_size >> 32;
	}
	return (DWORD) s.st_size;
}

STUB(GetStartupInfoW)

#define INVALID_FILE_ATTRIBUTES (-1)
#define FILE_ATTRIBUTE_READONLY 1
#define FILE_ATTRIBUTE_SYSTEM 4
#define FILE_ATTRIBUTE_DIRECTORY 16
#define FILE_ATTRIBUTE_NORMAL 0x80

WINAPI DWORD GetFileAttributesW(LPCWSTR lp_filename) {
	String file = utf16_to_str(lp_filename);
	if (strcmp(file.str, "C:\\") == 0 || strcmp(file.str, "C:\\Windows") == 0) {
		string_free(file);
		return FILE_ATTRIBUTE_DIRECTORY;
	}
	string_strip_prefix(&file, "C:\\");
	string_replace_all_c(&file, '\\', '/');
	struct stat s;
	if (stat(file.str, &s) == 0) {
		string_free(file);
		if (S_ISDIR(s.st_mode)) {
			return FILE_ATTRIBUTE_DIRECTORY;
		}
		else {
			return FILE_ATTRIBUTE_NORMAL;
		}
	}
	TODO;
	string_free(file);
	return INVALID_FILE_ATTRIBUTES;
}

STUB(GetThreadPriority)
STUB(SetThreadPriority)
STUB(VirtualAlloc)
STUB(GetSystemInfo)

WINAPI LPWSTR GetCommandLineW() {
	TODO;
	return L"";
}

WINAPI void* GetProcAddress(HMODULE h_module, LPCSTR lp_proc_name) {
	void* res = loaderlib_get_proc_addr((LoadedLib*) h_module, lp_proc_name);
	if (!res) {
		LoadedLib* lib = (LoadedLib*) h_module;
		if (lp_proc_name >= (const char*) 0xFFFF) {
			fprintf(stderr, "import %s not found from library %s\n", lp_proc_name, lib->name.str);
		}
		else {
			u16 ordinal = (u16) (usize) lp_proc_name;
			fprintf(stderr, "ordinal import %u not found from library %s\n", ordinal, lib->name.str);
		}
	}
	return res;
}

STUB(ResumeThread)

typedef struct {
	DWORD dw_os_version_info_size;
	DWORD dw_major_version;
	DWORD dw_minor_version;
	DWORD dw_build_number;
	DWORD dw_platform_id;
	WCHAR sz_csd_version[128];
} OSVERSIONINFOW, *LPOSVERSIONINFOW;

typedef struct {
	DWORD dw_os_version_info_size;
	DWORD dw_major_version;
	DWORD dw_minor_version;
	DWORD dw_build_number;
	DWORD dw_platform_id;
	WCHAR sz_csd_version[128];
	WORD w_service_pack_major;
	WORD w_service_pack_minor;
	WORD w_suite_mask;
	BYTE w_product_type;
	BYTE w_reserved;
} OSVERSIONINFOEXW, *LPOSVERSIONINFOEXW;

#define VER_PLATFORM_WIN32_NT 2
#define VER_NT_WORKSTATION 1

WINAPI BOOL GetVersionExW(LPOSVERSIONINFOW lp_version_information) {
	if (lp_version_information->dw_os_version_info_size == sizeof(OSVERSIONINFOW)) {
		LPOSVERSIONINFOW ver = lp_version_information;
		ver->dw_major_version = 10;
		ver->dw_minor_version = 0;
		ver->dw_platform_id = VER_PLATFORM_WIN32_NT;
		ver->dw_build_number = 19045;
		ver->sz_csd_version[0] = 0;
	}
	else if (lp_version_information->dw_os_version_info_size == sizeof(OSVERSIONINFOEXW)) {
		LPOSVERSIONINFOEXW ver = (LPOSVERSIONINFOEXW) lp_version_information;
		ver->dw_major_version = 10;
		ver->dw_minor_version = 0;
		ver->dw_platform_id = VER_PLATFORM_WIN32_NT;
		ver->dw_build_number = 19045;
		ver->sz_csd_version[0] = 0;
		ver->w_service_pack_major = 0;
		ver->w_service_pack_minor = 0;
		ver->w_suite_mask = 0;
		ver->w_product_type = VER_NT_WORKSTATION;
	}
	else {
		return 0;
	}

	return 1;
}

STUB(VerifyVersionInfoW)
STUB(HeapCreate)

WINAPI UINT GetWindowsDirectoryW(LPWSTR lp_buffer, UINT u_size) {
	WCHAR path[] = L"C:\\Windows\\";
	const usize path_size = sizeof(path) / sizeof(*path);
	if (u_size >= path_size) {
		memcpy(lp_buffer, path, sizeof(path));
		return path_size - 1;
	}
	return path_size;
}

STUB(VerSetConditionMask)
STUB(GetDiskFreeSpaceW)

WINAPI int lstrlenW(LPCWSTR lp_string) {
	int len = 0;
	while (*lp_string++) ++len;
	return len;
}

typedef struct {
	DWORD dw_file_attribs;
	FILETIME ft_creation_time;
	FILETIME ft_last_access_time;
	FILETIME ft_last_write_time;
	DWORD n_file_size_high;
	DWORD n_file_size_low;
	DWORD dw_reserved0;
	DWORD dw_reserved1;
	WCHAR c_filename[MAX_PATH];
	WCHAR c_alternate_filename[14];
	DWORD dw_file_type;
	DWORD dw_creator_type;
	WORD w_finder_flags;
} WIN32_FIND_DATAW, *LPWIN32_FIND_DATAW;

typedef struct {
	glob_t buf;
	size_t index;
	bool is_c;
} FindFileData;

WINAPI HANDLE FindFirstFileW(LPCWSTR lp_filename, LPWIN32_FIND_DATAW lp_find_file_data) {
	FindFileData* data = calloc(1, sizeof(FindFileData));
	if (!data) {
		return INVALID_HANDLE_VALUE;
	}
	data->index = 1;

	String name = utf16_to_str(lp_filename);
	bool is_c = string_strip_prefix(&name, "C:\\");
	string_replace_all_c(&name, '\\', '/');

	data->is_c = is_c;

	if (glob(name.str, 0, NULL, &data->buf) != 0) {
		globfree(&data->buf);
		free(data);
		return INVALID_HANDLE_VALUE;
	}

	struct stat s;
	if (stat(data->buf.gl_pathv[0], &s) == 0) {
		DWORD attribs = 0;
		string_free(name);

		if (S_ISDIR(s.st_mode)) {
			attribs |= FILE_ATTRIBUTE_DIRECTORY;
		}
		else {
			attribs = FILE_ATTRIBUTE_NORMAL;
		}

		*lp_find_file_data = (WIN32_FIND_DATAW) {
			.dw_file_attribs = attribs,
			.n_file_size_high = s.st_size >> 32,
			.n_file_size_low = (DWORD) s.st_size
		};
		unix_to_filetime(&lp_find_file_data->ft_creation_time, &s.st_ctim);
		unix_to_filetime(&lp_find_file_data->ft_last_access_time, &s.st_atim);
		unix_to_filetime(&lp_find_file_data->ft_last_write_time, &s.st_mtim);
		String tmp = string_new(data->buf.gl_pathv[0], strlen(data->buf.gl_pathv[0]));
		if (is_c) {
			string_prepend(&tmp, "C:\\", 3);
		}
		string_replace_all_c(&tmp, '/', '\\');
		str_to_utf16(tmp, lp_find_file_data->c_filename, 260);
		string_free(tmp);

		return data;
	}

	string_free(name);
	return data;
}

WINAPI LANGID GetUserDefaultUILanguage() {
	// en-US
	return 0x0409;
}

STUB(SetEndOfFile)

WINAPI BOOL HeapFree(HANDLE h_heap, DWORD dw_flags, LPVOID lp_mem) {
	TODO;
	free(lp_mem);
	return 1;
}

static usize wstrlen(LPCWSTR str) {
	usize len = 0;
	while (*str++) ++len;
	return len;
}

#define CP_ACP 0

WINAPI int WideCharToMultiByte(UINT code_page,
							   DWORD dw_flags,
							   LPCWCH lp_wide_char_str,
							   int cch_wide_char,
							   LPSTR lp_multibyte_str,
							   int cb_multibyte,
							   LPCCH lp_default_char,
							   LPBOOL lp_used_default_char) {
	if (cch_wide_char == 0) {
		return 0;
	}

	// ansi
	if (code_page == CP_ACP && dw_flags == 0) {
		int lp_wide_len = cch_wide_char;
		if (lp_wide_len == -1) {
			lp_wide_len = (int) wstrlen(lp_wide_char_str) + 1;
		}
		String res = utf16_to_str_with_len(lp_wide_char_str, lp_wide_len);
		int len = (int) res.len;
		if (cb_multibyte == 0) {
			string_free(res);
			return len;
		}
		if (lp_used_default_char) {
			*lp_used_default_char = 0;
		}
		memcpy(lp_multibyte_str, res.str, res.len);
		string_free(res);
		return len;
	}

	TODO;
	return 0;
}

WINAPI BOOL FindClose(HANDLE h_find_file) {
	globfree(&((FindFileData*) h_find_file)->buf);
	free(h_find_file);
	return 0;
}

STUB(MultiByteToWideChar)

WINAPI HMODULE LoadLibraryW(LPCWSTR lp_lib_file_name) {
	String name = utf16_to_str(lp_lib_file_name);
	string_strip_prefix(&name, "C:\\");
	string_replace_all_c(&name, '\\', '/');
	string_strip_prefix(&name, "Windows/System32/");
	if (!string_ends_with(name, ".dll")) {
		string_append(&name, ".dll", 4);
	}

	const char* file_start = strrchr(name.str, '/');
	usize len;
	if (!file_start) {
		file_start = name.str;
		len = name.len;
	}
	else {
		file_start += 1;
		len = name.len - (file_start - name.str);
	}
	LoadedLib* lib = loaderlib_get_lib(file_start, len);
	if (!lib) {
		if (loaderlib_load_lib(&lib, name.str, name.len) == STATUS_SUCCESS) {
			string_free(name);
			return lib;
		}
		fprintf(stderr, "LoadLibraryW: library %s not found\n", name.str);
	}

	string_free(name);
	return lib;
}

STUB(SetEvent)

#define GENERIC_READ 0x80000000
#define GENERIC_WRITE 0x40000000
#define GENERIC_EXECUTE 0x20000000
#define GENERIC_ALL 0x10000000

#define CREATE_ALWAYS 2
#define CREATE_NEW 1
#define OPEN_ALWAYS 4
#define OPEN_EXISTING 3
#define TRUNCATE_EXISTING 5

WINAPI HANDLE CreateFileW(
	LPCWSTR lp_filename,
	DWORD dw_desired_access,
	DWORD dw_share_mode,
	LPSECURITY_ATTRIBUTES lp_security_attribs,
	DWORD dw_creation_disposition,
	DWORD dw_flags_and_attribs,
	HANDLE h_template_file) {
	String name = utf16_to_str(lp_filename);
	string_strip_prefix(&name, "C:\\");
	string_replace_all_c(&name, '\\', '/');

	int acc = 0;
	if (dw_desired_access == GENERIC_READ) {
		acc = O_RDONLY;
	}
	else {
		acc = O_RDWR;
	}

	int flags = 0;
	if (dw_creation_disposition == CREATE_ALWAYS || dw_creation_disposition == TRUNCATE_EXISTING) {
		flags = O_CREAT | O_TRUNC;
	}
	else if (dw_creation_disposition == CREATE_NEW || dw_creation_disposition == OPEN_ALWAYS) {
		flags = O_CREAT;
	}

	int fd = open(name.str, acc | flags, 0755);
	string_free(name);

	// todo proper flags above

	if (fd < 0) {
		return INVALID_HANDLE_VALUE;
	}

	return (HANDLE) (uintptr_t) fd;
}

STUB(GetLocaleInfoW)

WINAPI UINT GetSystemDirectoryW(LPWSTR lp_buffer, UINT u_size) {
	const WCHAR path[] = L"C:\\Windows\\System32";
	const usize length = sizeof(path) / sizeof(*path);
	if (u_size >= length) {
		memcpy(lp_buffer, path, sizeof(path));
		return length - 1;
	}
	return length;
}

WINAPI BOOL DeleteFileW(LPCWSTR lp_file_name) {
	String a = utf16_to_str(lp_file_name);
	string_strip_prefix(&a, "C:\\");
	string_replace_all_c(&a, '\\', '/');
	if (remove(a.str) != 0) {
		return 0;
	}
	return 1;
}

STUB(GetLocalTime)
STUB(GetEnvironmentVariableW)
STUB(WaitForSingleObject)

WINAPI BOOL WriteFile(HANDLE h_file,
					  LPCVOID lp_buffer,
					  DWORD n_number_of_bytes_to_write,
					  LPDWORD lp_number_of_bytes_written,
					  LPOVERLAPPED lp_overlapped) {
	int fd = (int) (uintptr_t) h_file;
	ssize_t written = write(fd, lp_buffer, n_number_of_bytes_to_write);
	if (written == -1) {
		return 0;
	}

	if (!lp_overlapped) {
		*lp_number_of_bytes_written = (DWORD) written;
	}

	return 1;
}

STUB(ExitThread)
STUB(GetDateFormatW)

static UINT err_mode = 0;

WINAPI UINT SetErrorMode(UINT mode) {
	UINT old = err_mode;
	err_mode = mode;
	return old;
}

STUB(IsValidLocale)
STUB(TlsSetValue)

WINAPI BOOL CreateDirectoryW(LPCWSTR lp_path_name, LPSECURITY_ATTRIBUTES lp_security_attribs) {
	String path = utf16_to_str(lp_path_name);
	string_strip_prefix(&path, "C:\\");
	string_replace_all_c(&path, '\\', '/');
	int status = mkdir(path.str, 0755);
	string_free(path);

	if (status != 0) {
		return 0;
	}
	return 1;
}

STUB(GetSystemDefaultUILanguage)
STUB(EnumCalendarInfoW)
STUB(LocalAlloc)
STUB(GetUserDefaultLangID)

static int unlink_callback(const char* path, const struct stat* s, int type_flag, struct FTW* ftw_buf) {
	return remove(path);
}

WINAPI BOOL RemoveDirectoryW(LPCWSTR lp_pathname) {
	TODO;

	String path = utf16_to_str(lp_pathname);
	string_strip_prefix(&path, "C:\\");
	string_replace_all_c(&path, '\\', '/');

	if (nftw(path.str, unlink_callback, 64, FTW_DEPTH | FTW_PHYS) != 0) {
		string_free(path);
		return 0;
	}
	string_free(path);
	return 1;
}

STUB(CreateEventW)
STUB(SetThreadLocale)
STUB(GetThreadLocale)
STUB(SetCurrentDirectoryW)
STUB(MoveFileW)

WINAPI BOOL SetFileAttributesW(LPCWSTR lp_filename, DWORD dw_file_attribs) {
	TODO;
	return 1;
}

STUB(CopyFileW)

WINAPI BOOL SetEnvironmentVariableW(LPCWSTR lp_name, LPCWSTR lp_value) {
	TODO;
	return 0;
}

WINAPI DWORD GetTempPathW(DWORD n_buffer_length, LPWSTR lp_buffer) {
	const WCHAR path[] = L"C:\\Windows\\Temp\\";
	const usize len = sizeof(path) / sizeof(*path);
	if (len > n_buffer_length) {
		return len;
	}
	else {
		memcpy(lp_buffer, path, sizeof(path));
		return len - 1;
	}
}

STUB(CompareFileTime)
STUB(GlobalUnlock)
STUB(GlobalLock)
STUB(lstrcmpiA)

WINAPI UINT GetTempFileNameW(LPCWSTR lp_path_name, LPCWSTR lp_prefix_string, UINT u_unique, LPWSTR lp_temp_file_name) {
	String path = utf16_to_str(lp_path_name);
	String prefix = utf16_to_str(lp_prefix_string);
	if (!string_ends_with(path, "\\")) {
		string_append(&path, "\\", 1);
	}

	string_append(&path, prefix.str, prefix.len);

	if (u_unique == 0) {
		u_unique = (UINT) time(NULL);
	}

	char buf[11 + 4];
	usize buf_len = snprintf(buf, 11 + 4, "%u", u_unique);
	string_append(&path, buf, buf_len);

	usize len = PATH_MAX;
	const char* s = path.str;
	while (true) {
		u8 count = 0;
		u32 cp = utf8_decode(s, &count);
		s += count;
		u8 written = utf16_encode(lp_temp_file_name, cp, len > UCHAR_MAX ? UCHAR_MAX : len);
		lp_temp_file_name += written;
		len -= written;
		if (cp == 0) {
			break;
		}
	}

	string_free(path);

	return u_unique;
}

STUB(lstrcpyA)

WINAPI LPWSTR lstrcpyW(LPWSTR lp_string1, LPCWSTR lp_string2) {
	memcpy(lp_string1, lp_string2, (wstrlen(lp_string2) + 1) * sizeof(WCHAR));
	return lp_string1;
}

STUB(MoveFileExW)

WINAPI LPWSTR lstrcatW(LPWSTR lp_string1, LPCWSTR lp_string2) {
	usize len = wstrlen(lp_string1);
	memcpy(lp_string1 + len, lp_string2, (wstrlen(lp_string2) + 1) * sizeof(WCHAR));
	return lp_string1;
}

WINAPI HMODULE GetModuleHandleA(LPCSTR lp_module_name) {
	usize len = strlen(lp_module_name);
	if (len >= 4 && strncasecmp(lp_module_name + len - 4, ".dll", 4) == 0) {
		len -= 4;
	}
	return loaderlib_get_lib(lp_module_name, len);
}

#define GHND 0x42
#define GMEM_FIXED 0
#define GMEM_MOVEABLE 2
#define GMEM_ZEROINIT 0x40

WINAPI HGLOBAL GlobalFree(HGLOBAL h_mem) {
	free(h_mem);
	return NULL;
}

WINAPI HGLOBAL GlobalAlloc(UINT u_flags, SIZE_T dw_bytes) {
	if (u_flags & GMEM_ZEROINIT) {
		return calloc(1, dw_bytes);
	}
	else {
		return malloc(dw_bytes);
	}
}

STUB(GetShortPathNameW)
STUB(SearchPathW)

WINAPI int lstrcmpiW(LPCWSTR lp_string1, LPCWSTR lp_string2) {
	while (true) {
		// todo
		WCHAR a = *lp_string1;
		if (a >= L'A' && a <= L'Z') a = L'a' + a - L'A';
		WCHAR b = *lp_string2;
		if (b >= L'A' && b <= L'Z') b = L'a' + b - L'A';
		int res = a - b;
		if (res != 0) {
			return res;
		}
		else if (!*lp_string1) {
			return 0;
		}
		lp_string1 += 1;
		lp_string2 += 1;
	}
}

WINAPI BOOL SetFileTime(HANDLE h_file, const FILETIME* lp_creation_time, const FILETIME* lp_last_access_time, const FILETIME* lp_last_write_time) {
	TODO;
	return 1;
}

STUB(ExpandEnvironmentStringsW)
STUB(lstrcmpW)

WINAPI LPWSTR lstrcpynW(LPWSTR lp_string1, LPCWSTR lp_string2, int i_max_length) {
	if (!i_max_length) {
		return lp_string1;
	}

	LPWSTR orig = lp_string1;
	while (true) {
		if (i_max_length-- == 1) {
			*lp_string1 = 0;
			break;
		}
		*lp_string1++ = *lp_string2++;
		if (!lp_string2[-1]) {
			break;
		}
	}

	return orig;
}

WINAPI BOOL FindNextFileW(HANDLE h_find_file, LPWIN32_FIND_DATAW lp_find_file_data) {
	FindFileData* data = (FindFileData*) h_find_file;

	struct stat s;
	if (data->index < data->buf.gl_pathc && stat(data->buf.gl_pathv[data->index], &s) == 0) {
		DWORD attribs = 0;

		if (S_ISDIR(s.st_mode)) {
			attribs |= FILE_ATTRIBUTE_DIRECTORY;
		}
		else {
			attribs = FILE_ATTRIBUTE_NORMAL;
		}

		*lp_find_file_data = (WIN32_FIND_DATAW) {
				.dw_file_attribs = attribs,
				.n_file_size_high = s.st_size >> 32,
				.n_file_size_low = (DWORD) s.st_size
		};
		unix_to_filetime(&lp_find_file_data->ft_creation_time, &s.st_ctim);
		unix_to_filetime(&lp_find_file_data->ft_last_access_time, &s.st_atim);
		unix_to_filetime(&lp_find_file_data->ft_last_write_time, &s.st_mtim);
		String tmp = string_new(data->buf.gl_pathv[data->index], strlen(data->buf.gl_pathv[data->index]));
		if (data->is_c) {
			string_prepend(&tmp, "C:\\", 3);
		}
		string_replace_all_c(&tmp, '/', '\\');
		str_to_utf16(tmp, lp_find_file_data->c_filename, 260);
		string_free(tmp);

		data->index += 1;
		return 1;
	}

	data->index += 1;
	return 0;
}

WINAPI int MulDiv(int n_number, int n_numerator, int n_denominator) {
	TODO;
	i64 res = n_number * n_numerator;
	return (int) (res / n_denominator);
}

WINAPI int lstrlenA(LPCSTR lp_string) {
	int len = 0;
	while (*lp_string++) ++len;
	return len;
}

STUB(GetPrivateProfileStringW)
STUB(WritePrivateProfileStringW)

#define LOAD_LIBRARY_SEARCH_APPLICATION_DIR 0x200
#define LOAD_LIBRARY_SEARCH_DEFAULT_DIRS 0x1000
#define LOAD_LIBRARY_SEARCH_SYSTEM32 0x800
#define LOAD_LIBRARY_SEARCH_USER_DIRS 0x400

WINAPI BOOL SetDefaultDllDirectories(DWORD directory_flags) {
	TODO;
	return 1;
}

STUB(WriteConsoleW)
STUB(GetConsoleMode)
STUB(GlobalSize)
STUB(GetCurrentDirectoryW)

static void* process_heap = NULL;

WINAPI HANDLE GetProcessHeap() {
	TODO;
	return &process_heap;
}

#define HEAP_REALLOC_IN_PLACE_ONLY 0x10

WINAPI LPVOID HeapReAlloc(HANDLE h_heap, DWORD dw_flags, LPVOID lp_mem, SIZE_T dw_bytes) {
	TODO;
	if (dw_flags & HEAP_REALLOC_IN_PLACE_ONLY) {
		return NULL;
	}

	void* mem;
	if (dw_flags & HEAP_ZERO_MEMORY) {
		fprintf(stderr, "heap zeroing in HeapReAlloc\n");
	}
	mem = realloc(lp_mem, dw_bytes);
	return mem;
}

WINAPI HANDLE CreateMutexW(LPSECURITY_ATTRIBUTES lp_mutex_attribs, BOOL b_initial_owner, LPCWSTR lp_name) {
	mtx_t* mutex = (mtx_t*) malloc(sizeof(mtx_t));
	if (!mutex) {
		return NULL;
	}

	mtx_init(mutex, mtx_plain);
	if (b_initial_owner) {
		mtx_lock(mutex);
	}

	return mutex;
}
