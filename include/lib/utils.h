#pragma once
#define UTF_IMPLEMENTATION
#include "../utf.h"
#include "../str.h"

static String utf16_to_str(LPCWSTR str) {
	String utf8 = string_new("", 0);
	while (true) {
		u8 count = 0;
		u32 cp = utf16_decode(str, &count);
		if (!count || cp == 0) {
			break;
		}
		str += count;
		char buf[4];
		u8 enc = utf8_encode(buf, cp, 4);
		string_append(&utf8, buf, enc);
	}
	return utf8;
}

static String utf16_to_str_with_len(LPCWSTR str, usize len) {
	String utf8 = string_new("", 0);
	for (; len; --len) {
		u8 count = 0;
		u32 cp = utf16_decode(str, &count);
		str += count;
		char buf[4];
		u8 enc = utf8_encode(buf, cp, 4);
		string_append(&utf8, buf, enc);
	}
	return utf8;
}

static void str_to_utf16(String str, LPWSTR res, size_t len) {
	const char* s = str.str;
	usize total_written = 0;
	while (true) {
		if (len == 1) {
			*res = 0;
			return;
		}

		u8 count = 0;
		u32 cp = utf8_decode(s, &count);
		s += count;
		u8 written = utf16_encode(res, cp, len > UCHAR_MAX ? UCHAR_MAX : len);
		len -= written;
		res += written;
		if (cp == 0) {
			break;
		}
		total_written += written;
		if (written == 0) {
			*res = 0;
			return;
		}
	}
}