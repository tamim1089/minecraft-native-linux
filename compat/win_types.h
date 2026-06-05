// Force-included after msvc_compat.h. Lets the existing /Client headers parse unchanged
// instead of editing hundreds of files. Implementations of any non-inline stubbed symbols
// live in the stub .cpp files under build-linux (see CMakeLists LINUX_PORT branch).
// and keep the engine's std::wstring as-is. This is consistent within the process; the only
// place it bites is on-disk save format (UTF-16). Deferred — see PORT_LOG.md "wchar_t".
#pragma once
#ifndef PORT_WIN_TYPES_H
#define PORT_WIN_TYPES_H

#include <cstdint>
#include <cstddef>
#include <cstring>   // memset/memcpy for ZeroMemory etc.
#include <cwchar>

// ============================================================
// ============================================================
typedef unsigned char       BYTE;
typedef unsigned char       UCHAR;
typedef signed   char       INT8;
typedef unsigned char       UINT8;
typedef short               SHORT;
typedef unsigned short      USHORT;
typedef short               INT16;
typedef unsigned short      UINT16;
typedef unsigned short      WORD;
typedef int                 INT;
typedef unsigned int        UINT;
typedef int                 INT32;
typedef unsigned int        UINT32;
typedef int                 LONG;
typedef unsigned int        ULONG;
typedef unsigned int        DWORD;
typedef unsigned int        DWORD32;
typedef long long           LONGLONG;
typedef unsigned long long  ULONGLONG;
typedef long long           LONG64;
typedef unsigned long long  ULONG64_T_UNUSED;
typedef long long           INT64;
typedef unsigned long long  UINT64;
typedef unsigned long long  QWORD;
typedef unsigned long long  DWORD64;
typedef unsigned long long  ULONG64;
typedef int                 BOOL;
typedef float               FLOAT;
typedef char                CHAR;
typedef unsigned int        UINT_PTR;
typedef intptr_t            INT_PTR;
typedef intptr_t            LONG_PTR;
typedef uintptr_t           ULONG_PTR;
typedef ULONG_PTR           DWORD_PTR;
typedef ULONG_PTR           SIZE_T;
typedef LONG_PTR            SSIZE_T;
typedef long                HRESULT;

// The engine treats DWORD as a 32-bit counter/flags type; widening to 64-bit is harmless for
// logic but mis-sizes any struct serialized to disk/network. Revisit if a binary-layout bug
// appears. (Could switch to `unsigned int` — left as `unsigned long` to match swprintf %lu use.)

// ---- wide / text types ----
typedef wchar_t             WCHAR;          // 32-bit on Linux (see header note)
typedef WCHAR               TCHAR;
typedef char                TCHAR_A;

// ---- pointer aliases ----
typedef void               *PVOID;
typedef void               *LPVOID;
typedef const void         *LPCVOID;
typedef BYTE               *PBYTE;
typedef BYTE               *LPBYTE;
typedef WORD               *PWORD;
typedef DWORD              *PDWORD;
typedef DWORD              *LPDWORD;
typedef LONG               *PLONG;
typedef ULONG              *PULONG;
typedef int                *PINT;
typedef UINT               *PUINT;
typedef BOOL               *PBOOL;
typedef char               *PCHAR;
typedef char               *LPSTR;
typedef const char         *LPCSTR;
typedef const char         *PCSTR;
typedef WCHAR              *LPWSTR;
typedef WCHAR              *PWSTR;
typedef const WCHAR        *LPCWSTR;
typedef const WCHAR        *PCWSTR;
typedef TCHAR              *LPTSTR;
typedef const TCHAR        *LPCTSTR;

// ---- handles (opaque) ----
typedef void               *HANDLE;
typedef void               *HWND;
typedef void               *HINSTANCE;
typedef void               *HMODULE;
typedef void               *HDC;
typedef void               *HICON;
typedef void               *HCURSOR;
typedef void               *HMENU;
typedef void               *HGLRC;
typedef void               *HBITMAP;
typedef HINSTANCE           HRSRC;
typedef int                 SOCKET;

// ---- common macros / constants ----
#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)
#endif
#ifndef VOID
#define VOID void
#endif
#ifndef CONST
#define CONST const
#endif
#ifndef FAR
#define FAR
#endif
#ifndef NEAR
#define NEAR
#endif
#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
#ifndef ERROR_SUCCESS
#define ERROR_SUCCESS 0u
#endif
#ifndef ERROR_IO_PENDING
#define ERROR_IO_PENDING 997u
#endif
#ifndef ERROR_CANCELLED
#define ERROR_CANCELLED 1223u
#endif
#ifndef CDECL
#define CDECL
#endif
#ifndef S_OK
#define S_OK    ((HRESULT)0L)
#define S_FALSE ((HRESULT)1L)
#define E_FAIL  ((HRESULT)0x80004005L)
#endif
#ifndef SUCCEEDED
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#define FAILED(hr)    (((HRESULT)(hr)) < 0)
#endif

typedef union _LARGE_INTEGER {
    struct { DWORD LowPart; LONG HighPart; };
    LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef union _ULARGE_INTEGER {
    struct { DWORD LowPart; DWORD HighPart; };
    ULONGLONG QuadPart;
} ULARGE_INTEGER;

typedef struct _FILETIME { DWORD dwLowDateTime; DWORD dwHighDateTime; } FILETIME, *PFILETIME, *LPFILETIME;
typedef struct _SYSTEMTIME {
    WORD wYear, wMonth, wDayOfWeek, wDay, wHour, wMinute, wSecond, wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;
typedef struct _RECT { LONG left, top, right, bottom; } RECT, *PRECT, *LPRECT;
typedef struct _POINT { LONG x, y; } POINT, *PPOINT;
typedef struct _GUID { DWORD Data1; WORD Data2, Data3; BYTE Data4[8]; } GUID;
typedef GUID                IID;

// ============================================================
//  C++-only shims: sync primitives, atomics, helpers
// ============================================================
#ifdef __cplusplus
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

// ---- CRITICAL_SECTION ----
// thread, and (2) LeaveCriticalSection may be called from a DIFFERENT thread than the one that
// entered (the save path acquires the ConsoleSaveFile lock on the main thread via the output
// stream and releases it on the chunk-save worker via stream->close()). std::recursive_mutex
// enforces same-thread unlock, which deadlocks that pattern — so implement our own: an internal
// short-held mutex + condvar guarding an owner/count, where Leave just decrements (no owner check).
struct CRITICAL_SECTION {
    std::mutex              _g;
    std::condition_variable _cv;
    std::thread::id         _owner;
    int                     _count = 0;
    bool                    _held  = false;
    CRITICAL_SECTION() = default;
    CRITICAL_SECTION(const CRITICAL_SECTION&) = delete;
    CRITICAL_SECTION& operator=(const CRITICAL_SECTION&) = delete;
};
typedef CRITICAL_SECTION *LPCRITICAL_SECTION;

inline void InitializeCriticalSection(CRITICAL_SECTION*) {}
inline BOOL InitializeCriticalSectionAndSpinCount(CRITICAL_SECTION*, DWORD) { return TRUE; }
inline void DeleteCriticalSection(CRITICAL_SECTION*) {}
inline void EnterCriticalSection(CRITICAL_SECTION* cs) {
    std::unique_lock<std::mutex> lk(cs->_g);
    std::thread::id me = std::this_thread::get_id();
    if (cs->_held && cs->_owner == me) { cs->_count++; return; }   // recursive re-entry
    cs->_cv.wait(lk, [cs]{ return !cs->_held; });
    cs->_held = true; cs->_owner = me; cs->_count = 1;
}
inline void LeaveCriticalSection(CRITICAL_SECTION* cs) {
    std::unique_lock<std::mutex> lk(cs->_g);
    if (cs->_count > 0 && --cs->_count == 0) {          // release on count 0 (any thread may call)
        cs->_held = false; cs->_owner = std::thread::id();
        lk.unlock(); cs->_cv.notify_one();
    }
}
inline BOOL TryEnterCriticalSection(CRITICAL_SECTION* cs) {
    std::unique_lock<std::mutex> lk(cs->_g);
    std::thread::id me = std::this_thread::get_id();
    if (cs->_held && cs->_owner == me) { cs->_count++; return TRUE; }
    if (!cs->_held) { cs->_held = true; cs->_owner = me; cs->_count = 1; return TRUE; }
    return FALSE;
}

// ---- Interlocked* -> GCC atomic builtins (Win semantics: return prior/!new as documented) ----
inline LONG InterlockedIncrement(volatile LONG* p)            { return __sync_add_and_fetch(p, 1); }
inline LONG InterlockedDecrement(volatile LONG* p)            { return __sync_sub_and_fetch(p, 1); }
inline LONG InterlockedExchange(volatile LONG* p, LONG v)     { return __sync_lock_test_and_set(p, v); }
inline LONG InterlockedExchangeAdd(volatile LONG* p, LONG v)  { return __sync_fetch_and_add(p, v); }
inline LONG InterlockedCompareExchange(volatile LONG* p, LONG ex, LONG comp) { return __sync_val_compare_and_swap(p, comp, ex); }
inline LONGLONG InterlockedCompareExchange64(volatile LONGLONG* p, LONGLONG ex, LONGLONG comp) { return __sync_val_compare_and_swap(p, comp, ex); }
inline LONGLONG InterlockedCompareExchangeRelease64(volatile LONGLONG* p, LONGLONG ex, LONGLONG comp) { return __sync_val_compare_and_swap(p, comp, ex); }
inline LONGLONG InterlockedExchangeAdd64(volatile LONGLONG* p, LONGLONG v) { return __sync_fetch_and_add(p, v); }
#endif // __cplusplus

// ============================================================
// ============================================================
#ifndef ZeroMemory
#define ZeroMemory(p,sz) memset((p),0,(sz))
#endif
#ifndef CopyMemory
#define CopyMemory(d,s,sz) memcpy((d),(s),(sz))
#endif
#ifndef FillMemory
#define FillMemory(p,sz,v) memset((p),(v),(sz))
#endif

#ifdef __cplusplus
extern "C" {
#endif
// Implemented inline here where trivial; heavier ones are provided by win_compat.cpp stub.
void  port_Sleep(unsigned int ms);
unsigned int port_GetTickCount(void);
int   port_QueryPerformanceCounter(LARGE_INTEGER* p);
int   port_QueryPerformanceFrequency(LARGE_INTEGER* p);
#ifdef __cplusplus
}
#endif
#ifndef Sleep
#define Sleep(ms)                      port_Sleep(ms)
#endif
#ifndef GetTickCount
#define GetTickCount()                 port_GetTickCount()
#endif
#ifndef QueryPerformanceCounter
#define QueryPerformanceCounter(p)     port_QueryPerformanceCounter(p)
#endif
#ifndef QueryPerformanceFrequency
#define QueryPerformanceFrequency(p)   port_QueryPerformanceFrequency(p)
#endif
#ifndef GetModuleHandle
#ifdef UNICODE
#define GetModuleHandle   GetModuleHandleW
#define GetModuleFileName GetModuleFileNameW
#else
#define GetModuleHandle   GetModuleHandleA
#define GetModuleFileName GetModuleFileNameA
#endif
#endif
#ifndef OutputDebugString
#define OutputDebugString(s)  ((void)0)
#define OutputDebugStringA(s) ((void)0)
#define OutputDebugStringW(s) ((void)0)
#endif
#ifndef __debugbreak
#define __debugbreak() __builtin_trap()
#endif

// MSVC "safe" CRT string functions -> standard bounded equivalents.
#include <cstdio>
#ifndef sprintf_s
#define sprintf_s(buf, size, ...)  snprintf((buf), (size), __VA_ARGS__)
#endif
#ifndef _snprintf
#define _snprintf  snprintf
#endif
#ifndef _vsnprintf
#define _vsnprintf vsnprintf
#endif
#ifndef swscanf_s
#define swscanf_s  swscanf
#endif
#ifndef sscanf_s
#define sscanf_s   sscanf
#endif
#ifndef _TRUNCATE
#define _TRUNCATE ((size_t)-1)
#endif
#ifndef _snprintf_s
#define _snprintf_s(buf, size, count, ...)       snprintf((buf), (size), __VA_ARGS__)
#endif
#ifndef vsprintf_s
#define vsprintf_s(buf, size, fmt, ap)           vsnprintf((buf), (size), (fmt), (ap))
#endif
// _vsnprintf_s is used only in the secure 4-arg array form _vsnprintf_s(buf[N], count, fmt, ap);
// map to vsnprintf with the array's own size.
#ifndef _vsnprintf_s
#define _vsnprintf_s(buf, count, fmt, ap)  vsnprintf((buf), sizeof(buf), (fmt), (ap))
#endif

// ---- MSVC CRT string functions -> POSIX/standard equivalents ----
#include <cstring>
#include <cwchar>
#include <cstdlib>
#ifndef _stricmp
#define _stricmp   strcasecmp
#define _strnicmp  strncasecmp
#define _wcsicmp   wcscasecmp
#define _wcsnicmp  wcsncasecmp
#endif
#ifndef _strtoui64
#define _strtoui64 strtoull
#define _wcstoui64 wcstoull
#define _strtoi64  strtoll
#define _wcstoi64  wcstoll
#endif
// *_s secure copies — provided as inline overloads (not macros) so the secure *array* forms
// (deduce buffer size from a wchar_t[N]/char[N] member) and the explicit-size forms both work.
#ifdef __cplusplus
// --- wcsncpy_s ---
template <size_t N> inline int wcsncpy_s(wchar_t (&d)[N], const wchar_t* s, size_t c) {
    size_t n = c < N ? c : N - 1; wcsncpy(d, s, n); d[n] = 0; return 0;
}
inline int wcsncpy_s(wchar_t* d, size_t dsz, const wchar_t* s, size_t c) {
    size_t n = c < dsz ? c : (dsz ? dsz - 1 : 0); wcsncpy(d, s, n); if (dsz) d[n] = 0; return 0;
}
// --- strncpy_s ---
template <size_t N> inline int strncpy_s(char (&d)[N], const char* s, size_t c) {
    size_t n = c < N ? c : N - 1; strncpy(d, s, n); d[n] = 0; return 0;
}
inline int strncpy_s(char* d, size_t dsz, const char* s, size_t c) {
    size_t n = c < dsz ? c : (dsz ? dsz - 1 : 0); strncpy(d, s, n); if (dsz) d[n] = 0; return 0;
}
// --- wcscpy_s / strcpy_s ---
template <size_t N> inline int wcscpy_s(wchar_t (&d)[N], const wchar_t* s) { wcsncpy(d, s, N - 1); d[N-1] = 0; return 0; }
inline int wcscpy_s(wchar_t* d, size_t dsz, const wchar_t* s) { if (dsz) { wcsncpy(d, s, dsz - 1); d[dsz-1] = 0; } return 0; }
template <size_t N> inline int strcpy_s(char (&d)[N], const char* s) { strncpy(d, s, N - 1); d[N-1] = 0; return 0; }
inline int strcpy_s(char* d, size_t dsz, const char* s) { if (dsz) { strncpy(d, s, dsz - 1); d[dsz-1] = 0; } return 0; }
// --- wcscat_s / strcat_s ---
template <size_t N> inline int wcscat_s(wchar_t (&d)[N], const wchar_t* s) { wcsncat(d, s, N - wcslen(d) - 1); return 0; }
inline int wcscat_s(wchar_t* d, size_t dsz, const wchar_t* s) { wcsncat(d, s, dsz - wcslen(d) - 1); return 0; }
template <size_t N> inline int strcat_s(char (&d)[N], const char* s) { strncat(d, s, N - strlen(d) - 1); return 0; }
inline int strcat_s(char* d, size_t dsz, const char* s) { strncat(d, s, dsz - strlen(d) - 1); return 0; }
// secure radix int->string (explicit-size form). Forward to the _itoa/_i64toa shims.
extern "C" char* _itoa(int, char*, int);
extern "C" char* _i64toa(long long, char*, int);
inline int _itoa_s(int v, char* buf, size_t /*size*/, int radix)        { _itoa(v, buf, radix); return 0; }
inline int _i64toa_s(long long v, char* buf, size_t /*size*/, int radix) { _i64toa(v, buf, radix); return 0; }
#endif

#ifndef PAGE_READWRITE
#define PAGE_NOACCESS  0x01u
#define PAGE_READONLY  0x02u
#define PAGE_READWRITE 0x04u
#endif
#ifndef MEM_COMMIT
#define MEM_COMMIT   0x00001000u
#define MEM_RESERVE  0x00002000u
#define MEM_DECOMMIT 0x00004000u
#define MEM_RELEASE  0x00008000u
#define MEM_LARGE_PAGES 0x20000000u
#endif
#ifndef MAXULONG_PTR
#define MAXULONG_PTR ((ULONG_PTR)~((ULONG_PTR)0))
#endif

typedef struct _MEMORYSTATUS {
    DWORD     dwLength;
    DWORD     dwMemoryLoad;
    SIZE_T    dwTotalPhys;
    SIZE_T    dwAvailPhys;
    SIZE_T    dwTotalPageFile;
    SIZE_T    dwAvailPageFile;
    SIZE_T    dwTotalVirtual;
    SIZE_T    dwAvailVirtual;
} MEMORYSTATUS, *LPMEMORYSTATUS;

#ifdef __cplusplus
extern "C" {
#endif
LPVOID VirtualAlloc(LPVOID addr, SIZE_T size, DWORD allocType, DWORD protect);
BOOL   VirtualFree(LPVOID addr, SIZE_T size, DWORD freeType);
void   GlobalMemoryStatus(LPMEMORYSTATUS status);
void   GetSystemTime(LPSYSTEMTIME st);
void   GetLocalTime(LPSYSTEMTIME st);
HMODULE GetModuleHandleA(LPCSTR name);
HMODULE GetModuleHandleW(LPCWSTR name);
DWORD   GetModuleFileNameA(HMODULE, LPSTR buf, DWORD size);
DWORD   GetModuleFileNameW(HMODULE, LPWSTR buf, DWORD size);
BOOL   SystemTimeToFileTime(const SYSTEMTIME* st, FILETIME* ft);
BOOL   FileTimeToSystemTime(const FILETIME* ft, SYSTEMTIME* st);
// MSVC radix int->string conversions.
char*    _itoa(int value, char* str, int radix);
wchar_t* _itow(int value, wchar_t* str, int radix);
char*    _i64toa(long long value, char* str, int radix);
#ifdef __cplusplus
}
#endif

#include "d3d11_stub.h"
#include "platform_stub.h"
#include "win_threads.h"
#include "win_files.h"

#endif // PORT_WIN_TYPES_H
