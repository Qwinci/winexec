#include "common.h"
#include "winnt.h"

STUB(SHGetSpecialFolderLocation)
STUB(SHGetPathFromIDListW)
STUB(SHBrowseForFolderW)

typedef struct {
	HICON h_icon;
	int i_icon;
	DWORD dw_attribs;
	WCHAR sz_display_name[MAX_PATH];
	WCHAR sz_type_name[80];
} SHFILEINFOW;

#include "utils.h"

#define SHGFI_PIDL 0x8
#define SHGFI_USEFILEATTRIBUTES 0x10

WINAPI DWORD_PTR SHGetFileInfoW(LPCWSTR psz_path, DWORD dw_file_attribs, SHFILEINFOW* psfi, UINT cb_file_info, UINT u_flags) {
	if (u_flags & SHGFI_PIDL) {
		TODO;
		return NULL;
	}
	else if (u_flags & SHGFI_USEFILEATTRIBUTES) {
		TODO;
		return NULL;
	}

	if (*psz_path == 0) {
		return NULL;
	}

	String path = utf16_to_str(psz_path);

	string_free(path);
	TODO;
	return NULL;
}

STUB(ShellExecuteW)
STUB(SHFileOperationW)
STUB(ShellExecuteExW)

WINAPI BOOL IsUserAnAdmin() {
	return 1;
}

static Ordinal ORDINALS[] = {
	{
		.num = 680,
		.fn = IsUserAnAdmin
	}
};

__attribute__((used)) Ordinals g_ordinals = {
	.ptr = ORDINALS,
	.count = 1
};
