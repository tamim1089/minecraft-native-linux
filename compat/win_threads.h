// This is how CThread (and other CreateThread users like Connection.cpp) are "ported to
// std::thread": the primitives they sit on are implemented with std::thread / std::mutex /
// std::condition_variable / std::atomic in win_threads.cpp. The engine code is unchanged.
#pragma once
#ifndef PORT_WIN_THREADS_H
#define PORT_WIN_THREADS_H

#include "win_types.h"

// ---- constants ----
#ifndef INFINITE
#define INFINITE       0xFFFFFFFFu
#endif
#ifndef WAIT_OBJECT_0
#define WAIT_OBJECT_0  0x00000000u
#endif
#ifndef WAIT_TIMEOUT
#define WAIT_TIMEOUT   0x00000102u
#endif
#ifndef WAIT_FAILED
#define WAIT_FAILED    0xFFFFFFFFu
#endif
#ifndef WAIT_ABANDONED
#define WAIT_ABANDONED 0x00000080u
#endif
#ifndef CREATE_SUSPENDED
#define CREATE_SUSPENDED 0x00000004u
#endif
#ifndef TLS_OUT_OF_INDEXES
#define TLS_OUT_OF_INDEXES 0xFFFFFFFFu
#endif
#ifndef STILL_ACTIVE
#define STILL_ACTIVE 0x00000103u
#endif

// thread priority levels (no-op on Linux, accepted for source compat)
#ifndef THREAD_PRIORITY_IDLE
#define THREAD_PRIORITY_IDLE          (-15)
#define THREAD_PRIORITY_LOWEST        (-2)
#define THREAD_PRIORITY_BELOW_NORMAL  (-1)
#define THREAD_PRIORITY_NORMAL        (0)
#define THREAD_PRIORITY_ABOVE_NORMAL  (1)
#define THREAD_PRIORITY_HIGHEST       (2)
#define THREAD_PRIORITY_TIME_CRITICAL (15)
#endif

typedef DWORD (*LPTHREAD_START_ROUTINE)(LPVOID);
typedef void  *LPSECURITY_ATTRIBUTES;

#ifdef __cplusplus
extern "C" {
#endif

HANDLE CreateThread(LPSECURITY_ATTRIBUTES, SIZE_T stackSize,
                    LPTHREAD_START_ROUTINE start, LPVOID param,
                    DWORD creationFlags, LPDWORD threadId);
DWORD  ResumeThread(HANDLE thread);
DWORD  SuspendThread(HANDLE thread);
BOOL   SetThreadPriority(HANDLE thread, int priority);
int    GetThreadPriority(HANDLE thread);
DWORD  GetCurrentThreadId(void);
HANDLE GetCurrentThread(void);
BOOL   TerminateThread(HANDLE thread, DWORD exitCode);
BOOL   GetExitCodeThread(HANDLE thread, LPDWORD exitCode);

HANDLE CreateEvent(LPSECURITY_ATTRIBUTES, BOOL manualReset, BOOL initialState, LPCSTR name);
BOOL   SetEvent(HANDLE ev);
BOOL   ResetEvent(HANDLE ev);
BOOL   PulseEvent(HANDLE ev);

DWORD  WaitForSingleObject(HANDLE h, DWORD ms);
DWORD  WaitForMultipleObjects(DWORD count, const HANDLE* handles, BOOL waitAll, DWORD ms);
BOOL   CloseHandle(HANDLE h);

DWORD  GetLastError(void);
void   SetLastError(DWORD);

DWORD  TlsAlloc(void);
BOOL   TlsSetValue(DWORD index, LPVOID value);
LPVOID TlsGetValue(DWORD index);
BOOL   TlsFree(DWORD index);

#ifdef __cplusplus
}
#endif

#endif // PORT_WIN_THREADS_H
