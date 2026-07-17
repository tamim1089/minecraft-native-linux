// compat/win_threads.cpp — std::thread / std::condition_variable / std::atomic implementation
// engine's threading layer (CThread, Connection, etc.) to native Linux primitives.
#include "win_threads.h"
#include "win_handle.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <pthread.h>

namespace {

// Monotonic per-process thread id, mirrored into the running thread so GetCurrentThreadId()
// returns the same DWORD that CreateThread() reported via its out-param (CThread compares them).
std::atomic<DWORD> g_nextThreadId{1};
thread_local DWORD t_threadId = 0;

struct WinHandle : WinHandleBase {
    // --- event state ---
    std::mutex              mtx;
    std::condition_variable cv;
    bool                    signaled  = false;
    bool                    manualReset = false;
    // --- thread state ---
    std::thread             thr;
    std::atomic<bool>       finished{false};
    DWORD                   exitCode = 0;
    DWORD                   threadId = 0;
    // suspended-start gate
    std::mutex              startMtx;
    std::condition_variable startCv;
    bool                    resumed = true;

    ~WinHandle() override {
        if (kind == K_THREAD && thr.joinable()) {
            { std::lock_guard<std::mutex> lk(startMtx); resumed = true; }
            startCv.notify_all();
            thr.detach();   // loose ownership during Phase 0; avoid blocking on close
        }
    }
};

inline DWORD self_id() {
    if (t_threadId == 0) t_threadId = g_nextThreadId.fetch_add(1);
    return t_threadId;
}

} // namespace

extern "C" {

HANDLE CreateThread(LPSECURITY_ATTRIBUTES, SIZE_T /*stackSize*/,
                    LPTHREAD_START_ROUTINE start, LPVOID param,
                    DWORD creationFlags, LPDWORD threadId) {
    WinHandle* h = new WinHandle();
    h->kind = WinHandleBase::K_THREAD;
    h->threadId = g_nextThreadId.fetch_add(1);
    h->resumed = (creationFlags & CREATE_SUSPENDED) ? false : true;
    if (threadId) *threadId = h->threadId;

    DWORD assignedId = h->threadId;
    try {
        h->thr = std::thread([h, start, param, assignedId]() {
            t_threadId = assignedId;
            {   // honor CREATE_SUSPENDED until ResumeThread
                std::unique_lock<std::mutex> lk(h->startMtx);
                h->startCv.wait(lk, [h]{ return h->resumed; });
            }
            DWORD rc = start ? start(param) : 0;
            h->exitCode = rc;
            {
                std::lock_guard<std::mutex> lk(h->mtx);
                h->finished.store(true);
                h->signaled = true;           // a thread handle becomes signaled on exit
            }
            h->cv.notify_all();
        });
    } catch (...) {
        delete h;
        throw;
    }
    return (HANDLE)h;
}

DWORD ResumeThread(HANDLE thread) {
    WinHandle* h = (WinHandle*)thread;
    if (!h) return (DWORD)-1;
    {
        std::lock_guard<std::mutex> lk(h->startMtx);
        h->resumed = true;
    }
    h->startCv.notify_all();
    return 0;
}

DWORD SuspendThread(HANDLE) { return 0; }                  // not meaningfully supported
BOOL  SetThreadPriority(HANDLE, int) { return TRUE; }      // no-op on Linux
int   GetThreadPriority(HANDLE) { return THREAD_PRIORITY_NORMAL; }
BOOL  TerminateThread(HANDLE, DWORD) { return FALSE; }     // unsafe; unsupported
BOOL  GetExitCodeThread(HANDLE thread, LPDWORD exitCode) {
    WinHandle* h = (WinHandle*)thread;
    if (!h || !exitCode) return FALSE;
    *exitCode = h->finished.load() ? h->exitCode : STILL_ACTIVE;
    return TRUE;
}

DWORD  GetCurrentThreadId(void) { return self_id(); }
HANDLE GetCurrentThread(void)   { return (HANDLE)(LONG_PTR)-2; } // pseudo-handle

HANDLE CreateEvent(LPSECURITY_ATTRIBUTES, BOOL manualReset, BOOL initialState, LPCSTR) {
    WinHandle* h = new WinHandle();
    h->kind = WinHandleBase::K_EVENT;
    h->manualReset = manualReset ? true : false;
    h->signaled    = initialState ? true : false;
    return (HANDLE)h;
}

BOOL SetEvent(HANDLE ev) {
    WinHandle* h = (WinHandle*)ev;
    if (!h) return FALSE;
    {
        std::lock_guard<std::mutex> lk(h->mtx);
        h->signaled = true;
    }
    if (h->manualReset) h->cv.notify_all();
    else                h->cv.notify_one();
    return TRUE;
}

BOOL ResetEvent(HANDLE ev) {
    WinHandle* h = (WinHandle*)ev;
    if (!h) return FALSE;
    std::lock_guard<std::mutex> lk(h->mtx);
    h->signaled = false;
    return TRUE;
}

BOOL PulseEvent(HANDLE ev) {
    WinHandle* h = (WinHandle*)ev;
    if (!h) return FALSE;
    {
        std::lock_guard<std::mutex> lk(h->mtx);
        h->signaled = true;
    }
    h->cv.notify_all();
    {
        std::lock_guard<std::mutex> lk(h->mtx);
        h->signaled = false;
    }
    return TRUE;
}

static bool wait_one(WinHandle* h, DWORD ms) {
    std::unique_lock<std::mutex> lk(h->mtx);
    auto pred = [h]{ return h->signaled; };
    bool ok;
    if (ms == INFINITE) { h->cv.wait(lk, pred); ok = true; }
    else                { ok = h->cv.wait_for(lk, std::chrono::milliseconds(ms), pred); }
    if (ok && !h->manualReset && h->kind == WinHandleBase::K_EVENT) h->signaled = false; // auto-reset
    return ok;
}

DWORD WaitForSingleObject(HANDLE handle, DWORD ms) {
    WinHandle* h = (WinHandle*)handle;
    if (!h) return WAIT_FAILED;
    return wait_one(h, ms) ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
}

DWORD WaitForMultipleObjects(DWORD count, const HANDLE* handles, BOOL waitAll, DWORD ms) {
    if (!handles || count == 0) return WAIT_FAILED;
    using clock = std::chrono::steady_clock;
    auto deadline = clock::now() + std::chrono::milliseconds(ms == INFINITE ? 0 : ms);
    for (;;) {
        if (waitAll) {
            bool all = true;
            for (DWORD i = 0; i < count; ++i) {
                WinHandle* h = (WinHandle*)handles[i];
                std::lock_guard<std::mutex> lk(h->mtx);
                if (!h->signaled) { all = false; break; }
            }
            if (all) {
                for (DWORD i = 0; i < count; ++i) {
                    WinHandle* h = (WinHandle*)handles[i];
                    std::lock_guard<std::mutex> lk(h->mtx);
                    if (!h->manualReset && h->kind == WinHandleBase::K_EVENT) h->signaled = false;
                }
                return WAIT_OBJECT_0;
            }
        } else {
            for (DWORD i = 0; i < count; ++i) {
                WinHandle* h = (WinHandle*)handles[i];
                std::unique_lock<std::mutex> lk(h->mtx);
                if (h->signaled) {
                    if (!h->manualReset && h->kind == WinHandleBase::K_EVENT) h->signaled = false;
                    return WAIT_OBJECT_0 + i;
                }
            }
        }
        if (ms != INFINITE && clock::now() >= deadline) return WAIT_TIMEOUT;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

BOOL CloseHandle(HANDLE handle) {
    if (!handle || handle == (HANDLE)(LONG_PTR)-2 || handle == INVALID_HANDLE_VALUE) return FALSE;
    // Delete through the polymorphic base so the correct subtype dtor runs (thread join/detach,
    // file close, etc.) — handles created by any subsystem (threads/events/files) close here.
    delete static_cast<WinHandleBase*>(handle);
    return TRUE;
}

static thread_local DWORD g_lastError = 0;
DWORD GetLastError(void)     { return g_lastError; }
void  SetLastError(DWORD e)  { g_lastError = e; }

DWORD TlsAlloc(void) {
    pthread_key_t k;
    if (pthread_key_create(&k, nullptr) != 0) return TLS_OUT_OF_INDEXES;
    return (DWORD)k;
}
BOOL   TlsSetValue(DWORD index, LPVOID value) { return pthread_setspecific((pthread_key_t)index, value) == 0 ? TRUE : FALSE; }
LPVOID TlsGetValue(DWORD index)               { return pthread_getspecific((pthread_key_t)index); }
BOOL   TlsFree(DWORD index)                   { return pthread_key_delete((pthread_key_t)index) == 0 ? TRUE : FALSE; }

} // extern "C"
