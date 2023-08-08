#include <stdarg.h>
#include <string.h>
#include "common.h"
#include "windef.h"
#include "winnt.h"
#include "utils.h"

WINAPI HWND CreateWindowExW(
		DWORD dw_ex_style,
		LPCWSTR lp_class_name,
		LPCWSTR lp_window_name,
		DWORD dw_style,
		int x,
		int y,
		int n_width,
		int n_height,
		HWND hwnd_parent,
		HMENU h_menu,
		HINSTANCE h_instance,
		LPVOID lp_param) {
	TODO;
	return INVALID_HANDLE_VALUE;
}

STUB(TranslateMessage)
STUB(CharLowerBuffW)
STUB(CallWindowProcW)
STUB(CharUpperW)
STUB(PeekMessageW)
STUB(GetSystemMetrics)

WINAPI LONG SetWindowLongW(HWND hwnd, int n_index, LONG dw_new_long) {
	TODO;
	return 1;
}

STUB(MessageBoxW)

WINAPI BOOL DestroyWindow(HWND hwnd) {
	TODO;
	return 1;
}

STUB(CharUpperBuffW)

WINAPI LPWSTR CharNextW(LPCWSTR lp_sz) {
	if (*lp_sz == 0) {
		return (LPWSTR) lp_sz;
	}
	return (LPWSTR) lp_sz + 1;
}

STUB(MsgWaitForMultipleObjects)
STUB(LoadStringW)
STUB(ExitWindowsEx)
STUB(DispatchMessageW)

WINAPI HMENU GetSystemMenu(HWND h_wnd, BOOL b_revert) {
	TODO;
	return (HMENU) 1;
}


WINAPI DWORD SetClassLongW(HWND h_wnd, int n_index, LONG dw_new_long) {
	TODO;
	return 1;
}

STUB(IsWindowEnabled)

WINAPI BOOL EnableMenuItem(HMENU h_menu, UINT u_id_enable_item, UINT u_enable) {
	TODO;
	return 1;
}

STUB(SetWindowPos)
STUB(GetSysColor)
STUB(GetWindowLongW)
STUB(SetCursor)
STUB(LoadCursorW)
STUB(CheckDlgButton)
STUB(GetMessagePos)
STUB(LoadBitmapW)
STUB(IsWindowVisible)
STUB(CloseClipboard)
STUB(SetClipboardData)
STUB(EmptyClipboard)
STUB(OpenClipboard)

static void ws_write(LPWSTR str, LPCWSTR src, usize len) {
	memcpy(str, src, len * sizeof(WCHAR));
}

static void ws_write8(LPWSTR str, LPCSTR src, usize len) {
	for (; len; --len) {
		*str++ = (WCHAR) *src++;
	}
}

#define PAD_RIGHT (1U << (L'-' - L'#'))
#define PREFIX (1U << (L'#' - L'#'))
#define PAD_ZERO (1U << (L'0' - L'#'))
#define FLAG_MASK (PAD_RIGHT | PREFIX | PAD_ZERO)

static int wstrlen_internal(LPCWSTR str) {
	int len = 0;
	while (*str++) ++len;
	return len;
}

WINAPIV int wsprintfW(LPWSTR str, LPCWSTR fmt, ...) {
	__ms_va_list ap;
	__ms_va_start(ap, fmt);
	LPWSTR orig = str;

	int written = 0;
	while (true) {
		LPCWSTR start = fmt;
		int len = 0;
		for (; *fmt && *fmt != L'%'; ++fmt) ++len;
		for (; fmt[0] == L'%' && fmt[1] == L'%'; fmt += 2) ++len;
		ws_write(str, start, len);
		written += len;
		str += len;

		if (!*fmt) {
			break;
		}

		// %
		++fmt;

		u32 flags = 0;
		for (; (*fmt - '#') < 32 && (1U << (*fmt - '#') & FLAG_MASK); ++fmt) flags |= 1U << (*fmt - '#');

		if (*fmt >= L'0' && *fmt <= L'9') {
			fprintf(stderr, "todo: printf precision and width\n");
		}

		switch (*fmt) {
			case L's':
			{
				LPCWSTR value = va_arg(ap, LPCWSTR);
				int l = wstrlen_internal(value);
				ws_write(str, value, l);
				str += l;
				written += l;
				break;
			}
			case L'S':
			{
				LPCSTR value = va_arg(ap, LPCSTR);
				size_t l = strlen(value);
				ws_write8(str, value, l);
				str += l;
				written += (int) l;
				break;
			}
			case L'd':
			case L'i':
			{
				int value = va_arg(ap, int);
				WCHAR buf[11];
				buf[10] = 0;
				WCHAR* ptr = buf + 10;
				do {
					WCHAR c = L'0' + (value % 10);
					*--ptr = c;
					value /= 10;
				} while (value);
				size_t l = 10 - (ptr - buf);
				ws_write(str, ptr, l);
				str += l;
				written += (int) l;
				break;
			}
			default:
				fprintf(stderr, "wsprintfW: unsupported format specifier '%c'\n", (char) *fmt);
				break;
		}
		++fmt;
	}

	*str = 0;
	__ms_va_end(ap);
	return written;
}

STUB(ScreenToClient)

typedef struct {
	LONG left;
	LONG top;
	LONG right;
	LONG bottom;
} RECT, *LPRECT;

WINAPI BOOL GetWindowRect(HWND hwnd, LPRECT lp_rect) {
	TODO;
	return 1;
}

WINAPI BOOL SetDlgItemTextW(HWND h_dlg, int n_id_dlg_item, LPCWSTR lp_string) {
	String utf8 = utf16_to_str(lp_string);
	string_free(utf8);
	TODO;
	return 1;
}

STUB(GetDlgItemTextW)

typedef struct {
	LONG x;
	LONG y;
} POINT, *LPPOINT;

typedef struct {
	UINT cb_size;
	int i_context_type;
	int i_ctrl_id;
	HANDLE h_item_handle;
	DWORD_PTR dw_context_id;
	POINT mouse_pos;
} HELPINFO, *LPHELPINFO;

typedef VOID (WINAPI *MSGBOXCALLBACK)(LPHELPINFO lp_help_info);

typedef struct {
	UINT cb_size;
	HWND hwnd_owner;
	HINSTANCE h_instance;
	LPCWSTR lpsz_text;
	LPCWSTR lpsz_caption;
	DWORD dw_style;
	LPCWSTR lpsz_icon;
	DWORD_PTR dw_context_help_id;
	MSGBOXCALLBACK lpfn_msg_box_callback;
	DWORD dw_language_id;
} MSGBOXPARAMSW;

#include <SDL.h>

WINAPI int MessageBoxIndirectW(const MSGBOXPARAMSW* lp_mbp) {
	LPCWSTR caption = lp_mbp->lpsz_caption;
	LPCWSTR text = lp_mbp->lpsz_text;
	String capt_utf8 = string_new("", 0);
	String text_utf8 = string_new("", 0);
	while (true) {
		u8 count = 0;
		u32 cp = utf16_decode(caption, &count);
		if (!count || cp == 0) {
			break;
		}
		caption += count;
		char buf[4];
		u8 enc = utf8_encode(buf, cp, 4);
		string_append(&capt_utf8, buf, enc);
	}
	while (true) {
		u8 count = 0;
		u32 cp = utf16_decode(text, &count);
		if (!count || cp == 0) {
			break;
		}
		text += count;
		char buf[4];
		u8 enc = utf8_encode(buf, cp, 4);
		string_append(&text_utf8, buf, enc);
	}

	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, capt_utf8.str, text_utf8.str, NULL);
	string_free(capt_utf8);
	string_free(text_utf8);
	TODO;
	return 0;
}

WINAPI LPWSTR CharPrevW(LPCWSTR lp_sz_start, LPCWSTR lp_sz_current) {
	if (lp_sz_current == lp_sz_start) {
		return (LPWSTR) lp_sz_current;
	}
	return (LPWSTR) (lp_sz_current - 1);
}

STUB(CharNextA)
STUB(wsprintfA)

WINAPI HDC GetDC(HWND h_wnd) {
	TODO;
	return (HDC) 1;
}

WINAPI int ReleaseDC(HWND h_wnd, HDC hdc) {
	TODO;
	return 1;
}

WINAPI BOOL EnableWindow(HWND hwnd, BOOL b_enable) {
	TODO;
	return 1;
}

WINAPI BOOL InvalidateRect(HWND hwnd, const RECT* lp_rect, BOOL b_erase) {
	TODO;
	return 1;
}

WINAPI LRESULT SendMessageW(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param) {
	TODO;
	return 1;
}

STUB(DefWindowProcW)
STUB(BeginPaint)

WINAPI BOOL GetClientRect(HWND hwnd, LPRECT lp_rect) {
	TODO;
	return 1;
}

STUB(FillRect)
STUB(EndDialog)

typedef LRESULT (WINAPI *WNDPROC)(HWND param1, UINT param2, WPARAM param3, LPARAM param4);

typedef struct {
	UINT style;
	WNDPROC lp_fn_wnd_proc;
	int cb_cls_extra;
	int cb_wnd_extra;
	HINSTANCE h_instance;
	HICON h_icon;
	HCURSOR h_cursor;
	HBRUSH hbr_background;
	LPCWSTR lpsz_menu_name;
	LPCWSTR lpsz_class_name;
} WNDCLASSW, *LPWNDCLASSW;

WINAPI ATOM RegisterClassW(const WNDCLASSW* lp_wnd_class) {
	TODO;
	return 0;
}

STUB(SystemParametersInfoW)

WINAPI BOOL GetClassInfoW(HINSTANCE h_instance, LPCWSTR lp_class_name, LPWNDCLASSW lp_wnd_class) {
	TODO;
	return 0;
}

typedef INT_PTR (WINAPI *DLGPROC)(HWND param1, UINT param2, WPARAM param3, LPARAM param4);

#define WM_INITDIALOG 0x0110

#define TMP_DIALOG_HANDLE (HWND) (-3)

WINAPI INT_PTR DialogBoxParamW(HINSTANCE h_instance, LPCWSTR lp_template_name, HWND h_wnd_parent, DLGPROC lp_dialog_func, LPARAM dw_init_param) {
	lp_dialog_func(TMP_DIALOG_HANDLE, WM_INITDIALOG, 0, dw_init_param);
	TODO;
	return 0;
}

WINAPI HANDLE LoadImageW(HINSTANCE h_inst, LPCWSTR name, UINT type, int cx, int cy, UINT fu_load) {
	TODO;
	return INVALID_HANDLE_VALUE;
}

STUB(SetTimer)

WINAPI BOOL SetWindowTextW(HWND hwnd, LPCWSTR lp_string) {
	TODO;
	String text = utf16_to_str(lp_string);
	string_free(text);
	return 1;
}

STUB(PostQuitMessage)

WINAPI BOOL ShowWindow(HWND hwnd, int n_cmd_show) {
	TODO;
	return 0;
}


WINAPI HWND GetDlgItem(HWND h_dlg, int n_idd_lg_item) {
	TODO;
	return NULL;
}

STUB(IsWindow)

WINAPI HWND FindWindowExW(HWND hwnd_parent, HWND hwnd_child_after, LPCWSTR lpsz_class, LPCWSTR lpsz_window) {
	return NULL;
}

STUB(TrackPopupMenu)
STUB(AppendMenuW)
STUB(CreatePopupMenu)
STUB(DrawTextW)
STUB(EndPaint)

typedef INT_PTR (WINAPI *DLGPROC)(HWND param1, UINT param2, WPARAM param3, LPARAM param4);

WINAPI HWND CreateDialogParamW(HINSTANCE h_instance,
							   LPCWSTR lp_template_name,
							   HWND hwnd_parent,
							   DLGPROC lp_dialog_func,
							   LPARAM dw_init_param) {
	return NULL;
}

STUB(SendMessageTimeoutW)

WINAPI BOOL SetForegroundWindow(HWND hwnd) {
	TODO;
	return 1;
}

STUB(GetAsyncKeyState)
STUB(IsDlgButtonChecked)

WINAPI int wvsprintfW(LPWSTR out, LPCWSTR fmt, __ms_va_list ap) {
	TODO;
	return 0;
}

STUB(GetPropW)
STUB(DrawFocusRect)

WINAPI BOOL MapDialogRect(HWND h_dlg, LPRECT lp_rect) {
	TODO;
	return 1;
}

WINAPI BOOL SetPropW(HWND hwnd, LPCWSTR lp_string, HANDLE h_data) {
	TODO;
	return 1;
}

STUB(KillTimer)
STUB(GetMessageW)
STUB(IsDialogMessageW)

WINAPI int MapWindowPoints(HWND hwnd_from, HWND hwnd_to, LPPOINT lp_points, UINT c_points) {
	TODO;
	return 0;
}

STUB(RemovePropW)
STUB(GetWindowTextW)
