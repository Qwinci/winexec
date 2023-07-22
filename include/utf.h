#pragma once
#include "types.h"
#include <limits.h>

#define UTF_IMPLEMENTATION

#ifndef UTF_IMPLEMENTATION
u32 utf16_decode(const u16* str, u8* count);
u32 utf8_decode(const char* str, u8* count);
u8 utf16_encode(u16* str, u32 codepoint, u8 str_len);
u8 utf8_encode(char* str, u32 codepoint, u8 str_len);
#else
u32 utf16_decode(const u16* str, u8* count) {
	if (*str < 0xFFFF) {
		if (count) *count = 1;
		return (u32) *str;
	}
	u32 high = ((u32) *str - 0xD800) * 0x400;
	u32 low = (u32) *str - 0xDC00;
	u32 codepoint = high + low + 0x10000;
	if (count) *count = 2;
	return codepoint;
}

u32 utf8_decode(const char* str, u8* count) {
	if (*str >= 0x0) {
		if (count) *count = 1;
		return (u32) *str;
	}
	else if ((*str & 0b111 << 5) == 0b110 << 5) {
		u32 codepoint = (*str++ & 0b11111) << 6;
		codepoint |= (*str & 0b111111);
		if (count) *count = 2;
		return codepoint;
	}
	else if ((*str & 0b1111 << 4) == 0b1110 << 4) {
		u32 codepoint = (*str++ & 0b1111) << 12;
		codepoint |= (*str++ & 0b111111) << 6;
		codepoint |= (*str & 0b111111);
		if (count) *count = 3;
		return codepoint;
	}
	else if ((*str & 0b11111 << 3) == 0b11110 << 3) {
		u32 codepoint = (*str++ & 0b111) << 18;
		codepoint |= (*str++ & 0b111111) << 12;
		codepoint |= (*str++ & 0b111111) << 6;
		codepoint |= (*str & 0b111111);
		if (count) *count = 4;
		return codepoint;
	}
	else {
		if (count) *count = 0;
		return 0;
	}
}

u8 utf16_encode(u16* str, u32 codepoint, u8 str_len) {
	if (codepoint < 0xFFFF) {
		if (str_len < 1) return 0;
		*str = (u16) codepoint;
		return 1;
	}
	else if (codepoint <= 0x10FFFF) {
		if (str_len < 2) return 0;
		codepoint -= 0x10000;
		*str++ = 0xDC00 + (codepoint & 0x3FF);
		*str = 0xD800 + (codepoint >> 10);
		return 2;
	}
	else return 0;
}

u8 utf8_encode(char* str, u32 codepoint, u8 str_len) {
	if (codepoint <= 0x7F) {
		if (str_len < 1) return 0;
		*str = (char) codepoint;
		return 1;
	}
	else if (codepoint <= 0x7FF) {
		if (str_len < 2) return 0;
		*str++ = (char) ((0b110 << 5) | (codepoint >> 6 & 0b11111));
		*str = (char) ((0b10 << 6) | (codepoint & 0b111111));
		return 2;
	}
	else if (codepoint <= 0xFFFF) {
		if (str_len < 3) return 0;
		*str++ = (char) ((0b1110 << 4) | (codepoint >> 12 & 0b1111));
		*str++ = (char) ((0b10 << 6) | (codepoint >> 6 & 0b111111));
		*str = (char) ((0b10 << 6) | (codepoint & 0b111111));
		return 3;
	}
	else if (codepoint <= 0x10FFFF) {
		if (str_len < 4) return 0;
		*str++ = (char) ((0b11110 << 3) | (codepoint >> 18 & 0b111));
		*str++ = (char) ((0b10 << 6) | (codepoint >> 12 & 0b111111));
		*str++ = (char) ((0b10 << 6) | (codepoint >> 6 & 0b111111));
		*str = (char) ((0b10 << 6) | (codepoint & 0b111111));
		return 4;
	}
	else return 0;
}

#endif