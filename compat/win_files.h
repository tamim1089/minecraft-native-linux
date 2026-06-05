// HANDLE-based; file handles share the unified handle base so CloseHandle() works on them.
#pragma once
#ifndef PORT_WIN_FILES_H
#define PORT_WIN_FILES_H

#include "win_types.h"

// ---- access / share / creation / attribute flags ----
#ifndef GENERIC_READ
#define GENERIC_READ   0x80000000u
#define GENERIC_WRITE  0x40000000u
#endif
#ifndef FILE_SHARE_READ
#define FILE_SHARE_READ   0x00000001u
#define FILE_SHARE_WRITE  0x00000002u
#define FILE_SHARE_DELETE 0x00000004u
#endif
#ifndef CREATE_NEW
#define CREATE_NEW        1
#define CREATE_ALWAYS     2
#define OPEN_EXISTING     3
#define OPEN_ALWAYS       4
#define TRUNCATE_EXISTING 5
#endif
#ifndef FILE_ATTRIBUTE_NORMAL
#define FILE_ATTRIBUTE_READONLY  0x00000001u
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010u
#define FILE_ATTRIBUTE_NORMAL    0x00000080u
#endif
#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES  0xFFFFFFFFu
#endif
#ifndef INVALID_FILE_SIZE
#define INVALID_FILE_SIZE        0xFFFFFFFFu
#endif
#ifndef FILE_BEGIN
#define FILE_BEGIN   0
#define FILE_CURRENT 1
#define FILE_END     2
#endif
#ifndef FILE_FLAG_RANDOM_ACCESS
#define FILE_FLAG_WRITE_THROUGH   0x80000000u
#define FILE_FLAG_NO_BUFFERING    0x20000000u
#define FILE_FLAG_RANDOM_ACCESS   0x10000000u
#define FILE_FLAG_SEQUENTIAL_SCAN 0x08000000u
#endif

// WIN32_FIND_DATA — wide variant is the one the engine uses under UNICODE.
typedef struct _WIN32_FIND_DATAW {
    DWORD    dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD    nFileSizeHigh;
    DWORD    nFileSizeLow;
    DWORD    dwReserved0;
    DWORD    dwReserved1;
    WCHAR    cFileName[MAX_PATH];
    WCHAR    cAlternateFileName[14];
} WIN32_FIND_DATAW, *PWIN32_FIND_DATAW, *LPWIN32_FIND_DATAW;

typedef struct _WIN32_FIND_DATAA {
    DWORD    dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD    nFileSizeHigh;
    DWORD    nFileSizeLow;
    DWORD    dwReserved0;
    DWORD    dwReserved1;
    CHAR     cFileName[MAX_PATH];
    CHAR     cAlternateFileName[14];
} WIN32_FIND_DATAA, *PWIN32_FIND_DATAA, *LPWIN32_FIND_DATAA;

#ifdef UNICODE
typedef WIN32_FIND_DATAW WIN32_FIND_DATA;
typedef LPWIN32_FIND_DATAW LPWIN32_FIND_DATA;
#else
typedef WIN32_FIND_DATAA WIN32_FIND_DATA;
typedef LPWIN32_FIND_DATAA LPWIN32_FIND_DATA;
#endif

typedef struct _WIN32_FILE_ATTRIBUTE_DATA {
    DWORD    dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD    nFileSizeHigh;
    DWORD    nFileSizeLow;
} WIN32_FILE_ATTRIBUTE_DATA, *LPWIN32_FILE_ATTRIBUTE_DATA;

typedef enum _GET_FILEEX_INFO_LEVELS { GetFileExInfoStandard = 0, GetFileExMaxInfoLevel = 1 } GET_FILEEX_INFO_LEVELS;

#ifdef __cplusplus
extern "C" {
#endif

BOOL GetFileAttributesExA(LPCSTR, GET_FILEEX_INFO_LEVELS, LPVOID info);
BOOL GetFileAttributesExW(LPCWSTR, GET_FILEEX_INFO_LEVELS, LPVOID info);

HANDLE CreateFileA(LPCSTR name, DWORD access, DWORD share, LPSECURITY_ATTRIBUTES,
                   DWORD creationDisposition, DWORD flagsAndAttrs, HANDLE templateFile);
HANDLE CreateFileW(LPCWSTR name, DWORD access, DWORD share, LPSECURITY_ATTRIBUTES,
                   DWORD creationDisposition, DWORD flagsAndAttrs, HANDLE templateFile);
BOOL  ReadFile (HANDLE, LPVOID buf, DWORD toRead,  LPDWORD read,    LPVOID overlapped);
BOOL  WriteFile(HANDLE, LPCVOID buf, DWORD toWrite, LPDWORD written, LPVOID overlapped);
DWORD GetFileSize(HANDLE, LPDWORD sizeHigh);
DWORD SetFilePointer(HANDLE, LONG distLow, PLONG distHigh, DWORD method);
BOOL  SetEndOfFile(HANDLE);
BOOL  FlushFileBuffers(HANDLE);

DWORD GetFileAttributesA(LPCSTR);
DWORD GetFileAttributesW(LPCWSTR);
BOOL  SetFileAttributesA(LPCSTR, DWORD);
BOOL  SetFileAttributesW(LPCWSTR, DWORD);
BOOL  CreateDirectoryA(LPCSTR, LPSECURITY_ATTRIBUTES);
BOOL  CreateDirectoryW(LPCWSTR, LPSECURITY_ATTRIBUTES);
BOOL  RemoveDirectoryA(LPCSTR);
BOOL  RemoveDirectoryW(LPCWSTR);
BOOL  DeleteFileA(LPCSTR);
BOOL  DeleteFileW(LPCWSTR);
BOOL  MoveFileA(LPCSTR, LPCSTR);
BOOL  MoveFileW(LPCWSTR, LPCWSTR);

HANDLE FindFirstFileA(LPCSTR pattern, LPWIN32_FIND_DATAA);
HANDLE FindFirstFileW(LPCWSTR pattern, LPWIN32_FIND_DATAW);
BOOL   FindNextFileA(HANDLE, LPWIN32_FIND_DATAA);
BOOL   FindNextFileW(HANDLE, LPWIN32_FIND_DATAW);
BOOL   FindClose(HANDLE);

#ifdef __cplusplus
}
#endif

// UNICODE / ANSI selection (the engine calls the un-suffixed names; under UNICODE -> W).
#ifdef UNICODE
#define CreateFile        CreateFileW
#define GetFileAttributes GetFileAttributesW
#define SetFileAttributes SetFileAttributesW
#define CreateDirectory   CreateDirectoryW
#define RemoveDirectory   RemoveDirectoryW
#define DeleteFile        DeleteFileW
#define MoveFile          MoveFileW
#define FindFirstFile     FindFirstFileW
#define FindNextFile      FindNextFileW
#define GetFileAttributesEx GetFileAttributesExW
#else
#define CreateFile        CreateFileA
#define GetFileAttributes GetFileAttributesA
#define SetFileAttributes SetFileAttributesA
#define CreateDirectory   CreateDirectoryA
#define RemoveDirectory   RemoveDirectoryA
#define DeleteFile        DeleteFileA
#define MoveFile          MoveFileA
#define FindFirstFile     FindFirstFileA
#define FindNextFile      FindNextFileA
#define GetFileAttributesEx GetFileAttributesExA
#endif

#endif // PORT_WIN_FILES_H
