#include "common.h"
#include "winnt.h"

STUB(SelectObject)
STUB(SetBkMode)

#define LF_FACESIZE 32

typedef struct {
	LONG lf_height;
	LONG lf_width;
	LONG lf_escapement;
	LONG lf_orientation;
	LONG lf_weight;
	BYTE lf_italic;
	BYTE lf_underline;
	BYTE lf_strikeout;
	BYTE lf_charset;
	BYTE lf_out_precision;
	BYTE lf_clip_precision;
	BYTE lf_quality;
	BYTE lf_pitch_and_family;
	WCHAR lf_face_name[LF_FACESIZE];
} LOGFONTW;

WINAPI HFONT CreateFontIndirectW(const LOGFONTW* lplf) {
	TODO;
	return (HFONT) 1;
}

STUB(SetTextColor)

WINAPI BOOL DeleteObject(HGDIOBJ ho) {
	TODO;
	return 1;
}

WINAPI int GetDeviceCaps(HDC hdc, int index) {
	TODO;
	return 1;
}

STUB(CreateBrushIndirect)
STUB(SetBkColor)
