#pragma once

#include <stddef.h>
#include "windef.h"
#include "ntdef.h"
#include "basetsd.h"

#define MAX_PATH 260

#define VOID void
typedef void* PVOID;
typedef PVOID HANDLE;

typedef char CHAR;
typedef CHAR* LPSTR;
typedef const CHAR* LPCSTR;
typedef wchar_t WCHAR;
typedef WCHAR* PWSTR;
typedef WCHAR* LPWSTR;
typedef const WCHAR* LPCWSTR;
typedef LONG HRESULT;
typedef WORD LANGID;
typedef const WCHAR* LPCWCH;
typedef const char* LPCCH;
typedef BOOL* LPBOOL;

typedef DWORD ACCESS_MASK;
typedef ACCESS_MASK REGSAM;

#define S_OK 0
#define S_FALSE 1
#define E_FAIL 0x80004005

typedef union {
	struct {
		DWORD LowPart;
		LONG HighPart;
	};
	LONGLONG QuadPart;
} LARGE_INTEGER;

typedef struct RTL_CRITICAL_SECTION* PRTL_CRITICAL_SECTION;

typedef struct {
	WORD Type;
	WORD CreatorBackTraceIndex;
	PRTL_CRITICAL_SECTION CriticalSection;
	LIST_ENTRY ProcessLocksList;
	ULONG EntryCount;
	ULONG ContentionCount;
	ULONG Flags;
	WORD CreatorBackTraceIndexHigh;
	WORD SpareShort;
} RTL_CRITICAL_SECTION_DEBUG, *PRTL_CRITICAL_SECTION_DEBUG;

typedef struct RTL_CRITICAL_SECTION {
	PRTL_CRITICAL_SECTION_DEBUG DebugInfo;
	LONG LockCount;
	LONG RecursionCount;
	HANDLE OwningThread;
	HANDLE LockSemaphore;
	ULONG_PTR SpinCount;
} RTL_CRITICAL_SECTION, *PRTL_CRITICAL_SECTION;

#define EXCEPTION_MAXIMUM_PARAMETERS 15

typedef struct EXCEPTION_RECORD {
	DWORD ExceptionCode;
	DWORD ExceptionFlags;
	struct EXCEPTION_RECORD* ExceptionRecord;
	PVOID ExceptionAddress;
	DWORD NumberParameters;
	ULONG_PTR ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
} EXCEPTION_RECORD, *PEXCEPTION_RECORD;

typedef struct {
	ULONGLONG Low;
	LONGLONG High;
} M128A, *PM128A;

typedef struct [[gnu::aligned(16)]] {
	WORD ControlWord;
	WORD StatusWord;
	BYTE TagWord;
	BYTE Reserved1;
	WORD ErrorOpcode;
	DWORD ErrorOffset;
	WORD ErrorSelector;
	WORD Reserved2;
	DWORD DataOffset;
	WORD DataSelector;
	WORD Reserved3;
	DWORD MxCsr;
	DWORD MxCsr_Mask;
	M128A FloatRegisters[8];

#if defined(_WIN64)
	M128A XmmRegisters[16];
	BYTE Reserved4[96];
#else
	M128A XmmRegisters[8];
	BYTE Reserved[224];
#endif
} XSAVE_FORMAT;

typedef XSAVE_FORMAT XMM_SAVE_AREA32, *PXMM_SAVE_AREA32;

typedef struct {
	ULONGLONG Low;
	LONGLONG High;
} NEON128, *PNEON128;

typedef struct {
	DWORD64 P1Home;
	DWORD64 P2Home;
	DWORD64 P3Home;
	DWORD64 P4Home;
	DWORD64 P5Home;
	DWORD64 P6Home;
	DWORD   ContextFlags;
	DWORD   MxCsr;
	WORD    SegCs;
	WORD    SegDs;
	WORD    SegEs;
	WORD    SegFs;
	WORD    SegGs;
	WORD    SegSs;
	DWORD   EFlags;
	DWORD64 Dr0;
	DWORD64 Dr1;
	DWORD64 Dr2;
	DWORD64 Dr3;
	DWORD64 Dr6;
	DWORD64 Dr7;
	DWORD64 Rax;
	DWORD64 Rcx;
	DWORD64 Rdx;
	DWORD64 Rbx;
	DWORD64 Rsp;
	DWORD64 Rbp;
	DWORD64 Rsi;
	DWORD64 Rdi;
	DWORD64 R8;
	DWORD64 R9;
	DWORD64 R10;
	DWORD64 R11;
	DWORD64 R12;
	DWORD64 R13;
	DWORD64 R14;
	DWORD64 R15;
	DWORD64 Rip;
	union {
		XMM_SAVE_AREA32 FltSave;
		NEON128         Q[16];
		ULONGLONG       D[32];
		struct {
			M128A Header[2];
			M128A Legacy[8];
			M128A Xmm0;
			M128A Xmm1;
			M128A Xmm2;
			M128A Xmm3;
			M128A Xmm4;
			M128A Xmm5;
			M128A Xmm6;
			M128A Xmm7;
			M128A Xmm8;
			M128A Xmm9;
			M128A Xmm10;
			M128A Xmm11;
			M128A Xmm12;
			M128A Xmm13;
			M128A Xmm14;
			M128A Xmm15;
		} DUMMYSTRUCTNAME;
		DWORD           S[32];
	} DUMMYUNIONNAME;
	M128A   VectorRegister[26];
	DWORD64 VectorControl;
	DWORD64 DebugControl;
	DWORD64 LastBranchToRip;
	DWORD64 LastBranchFromRip;
	DWORD64 LastExceptionToRip;
	DWORD64 LastExceptionFromRip;
} CONTEXT, *PCONTEXT;

typedef struct {
	PEXCEPTION_RECORD ExceptionRecord;
	PCONTEXT ContextRecord;
} EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;