#include "common.h"
#include "winnt.h"
#include "utils.h"

#define CURRENTVERSION_KEY ((HKEY) -2)

#define REG_SZ 1

WINAPI LSTATUS RegQueryValueExW(HKEY h_key, LPCWSTR lp_value_name, LPDWORD lp_reserved, LPDWORD lp_type, LPBYTE lp_data, LPDWORD lpcb_data) {
	if (h_key == CURRENTVERSION_KEY) {
		if (lp_value_name) {
			String name = utf16_to_str(lp_value_name);
			if (strcmp(name.str, "ProgramFilesDir") == 0) {
				if (lp_type) {
					*lp_type = REG_SZ;
				}
				if (lp_data) {
					const WCHAR dir[] = L"C:\\Program Files";
					memcpy(lp_data, dir, sizeof(dir));
					*lpcb_data = sizeof(dir);
				}
				return 1;
			}

			string_free(name);
			TODO;
		}
		else {
			TODO;
		}
	}

	TODO;
	return 1;
}

STUB(AdjustTokenPrivileges)
STUB(LookupPrivilegeValueW)

WINAPI LSTATUS RegCloseKey(HKEY key) {
	TODO;
	return 0;
}

STUB(OpenProcessToken)

#define HKEY_CLASSES_ROOT (HKEY) (-2147483648)
#define HKEY_CURRENT_CONFIG (HKEY) (-2147483643)
#define HKEY_CURRENT_USER (HKEY) (-2147483647)
#define HKEY_CURRENT_USER_LOCAL_SETTINGS (HKEY) (-2147483641)
#define HKEY_LOCAL_MACHINE (HKEY) (-2147483646)

WINAPI static BOOL is_predefined(HKEY key) {
	return key == HKEY_CLASSES_ROOT || key == HKEY_CURRENT_CONFIG ||
		key == HKEY_CURRENT_USER || key == HKEY_CURRENT_USER_LOCAL_SETTINGS ||
		key == HKEY_LOCAL_MACHINE;
}

WINAPI LSTATUS RegOpenKeyExW(HKEY h_key, LPCWSTR lp_sub_key, DWORD ul_options, REGSAM sam_desired, PHKEY phk_result) {
	if ((lp_sub_key == NULL || *lp_sub_key == 0) && is_predefined(h_key)) {
		*phk_result = h_key;
		return 0;
	}

	String sub_key = utf16_to_str(lp_sub_key);
	*phk_result = INVALID_HANDLE_VALUE;
	const char current_version[] = "Software\\Microsoft\\Windows\\CurrentVersion";
	if (sub_key.len == sizeof(current_version) - 1 && strncmp(sub_key.str, current_version, sub_key.len) == 0) {
		*phk_result = CURRENTVERSION_KEY;
		string_free(sub_key);
		return 0;
	}
	string_free(sub_key);
	TODO;
	return 1;
}

STUB(RegDeleteKeyW)

typedef DWORD SECURITY_INFORMATION;
typedef WORD SECURITY_DESCRIPTOR_CONTROL;

typedef struct {
	BYTE value[6];
} SID_IDENTIFIER_AUTHORITY;

typedef struct {
	BYTE revision;
	BYTE sub_auth_count;
	SID_IDENTIFIER_AUTHORITY identifier_authority;
	DWORD* sub_auth[];
} SID, *PSID;

typedef struct {
	BYTE acl_revision;
	BYTE sbz1;
	WORD acl_size;
	WORD ace_count;
	WORD sbz2;
} ACL, *PACL;

typedef struct {
	BYTE revision;
	BYTE sbz1;
	SECURITY_DESCRIPTOR_CONTROL control;
	PSID owner;
	PSID group;
	PACL sacl;
	PACL dacl;
} SECURITY_DESTRIPTOR, *PSECURITY_DESCRIPTOR;

WINAPI BOOL SetFileSecurityW(LPCWSTR lp_filename, SECURITY_INFORMATION security_information, PSECURITY_DESCRIPTOR p_security_descriptor) {
	return 1;
}

STUB(RegEnumValueW)
STUB(RegDeleteValueW)
STUB(RegCreateKeyExW)
STUB(RegSetValueExW)
STUB(RegEnumKeyW)
