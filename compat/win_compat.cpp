// Timing maps to CLOCK_MONOTONIC; everything else is a benign no-op for the headless build.
#include <ctime>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <sys/mman.h>
#include "win_types.h"

extern "C" {

void port_Sleep(unsigned int ms) {
    struct timespec ts;
    ts.tv_sec  = ms / 1000u;
    ts.tv_nsec = (long)(ms % 1000u) * 1000000L;
    nanosleep(&ts, nullptr);
}

unsigned int port_GetTickCount(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (unsigned int)(ts.tv_sec * 1000ull + ts.tv_nsec / 1000000ull);
}

// LARGE_INTEGER layout matches win_types.h (QuadPart is the low 64 bits of the union).
int port_QueryPerformanceCounter(union _LARGE_INTEGER* p) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    if (p) p->QuadPart = (long long)ts.tv_sec * 1000000000ll + ts.tv_nsec;
    return 1;
}

int port_QueryPerformanceFrequency(union _LARGE_INTEGER* p) {
    if (p) p->QuadPart = 1000000000ll;   // counter is in nanoseconds
    return 1;
}

// committed, zeroed anonymous mapping. TODO(port): honor MEM_RESERVE/MEM_COMMIT separately
// if a save-format path relies on reserve-then-commit-subregion semantics. ----
LPVOID VirtualAlloc(LPVOID /*addr*/, SIZE_T size, DWORD /*allocType*/, DWORD /*protect*/) {
    if (size == 0) return nullptr;
    void* p = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return (p == MAP_FAILED) ? nullptr : p;
}

BOOL VirtualFree(LPVOID addr, SIZE_T size, DWORD freeType) {
    if (!addr) return 0;
    if (freeType & MEM_DECOMMIT) return 1;          // no-op: keep the mapping
    munmap(addr, size ? size : 0);                  // MEM_RELEASE; size 0 -> best-effort
    return 1;
}

void GlobalMemoryStatus(LPMEMORYSTATUS s) {
    if (!s) return;
    long pages    = sysconf(_SC_PHYS_PAGES);
    long pagesize = sysconf(_SC_PAGE_SIZE);
    long avail    = sysconf(_SC_AVPHYS_PAGES);
    unsigned long long total = (pages > 0 && pagesize > 0) ? (unsigned long long)pages * pagesize : 0;
    unsigned long long free_ = (avail > 0 && pagesize > 0) ? (unsigned long long)avail * pagesize : 0;
    s->dwLength         = sizeof(*s);
    s->dwMemoryLoad     = (total > 0) ? (unsigned)(100ull - (free_ * 100ull / total)) : 0;
    s->dwTotalPhys      = (unsigned long)total;
    s->dwAvailPhys      = (unsigned long)free_;
    s->dwTotalPageFile  = (unsigned long)total;
    s->dwAvailPageFile  = (unsigned long)free_;
    s->dwTotalVirtual   = (unsigned long)total;
    s->dwAvailVirtual   = (unsigned long)free_;
}

static void fill_systime(LPSYSTEMTIME st, const struct tm& t, unsigned short ms) {
    st->wYear = (unsigned short)(t.tm_year + 1900);
    st->wMonth = (unsigned short)(t.tm_mon + 1);
    st->wDayOfWeek = (unsigned short)t.tm_wday;
    st->wDay = (unsigned short)t.tm_mday;
    st->wHour = (unsigned short)t.tm_hour;
    st->wMinute = (unsigned short)t.tm_min;
    st->wSecond = (unsigned short)t.tm_sec;
    st->wMilliseconds = ms;
}

void GetSystemTime(LPSYSTEMTIME st) {
    if (!st) return;
    struct timespec ts; clock_gettime(CLOCK_REALTIME, &ts);
    struct tm g; gmtime_r(&ts.tv_sec, &g);
    fill_systime(st, g, (unsigned short)(ts.tv_nsec / 1000000));
}

void GetLocalTime(LPSYSTEMTIME st) {
    if (!st) return;
    struct timespec ts; clock_gettime(CLOCK_REALTIME, &ts);
    struct tm l; localtime_r(&ts.tv_sec, &l);
    fill_systime(st, l, (unsigned short)(ts.tv_nsec / 1000000));
}

// ---- Module handle / path (headless: a dummy non-null handle; exe path via /proc) ----
HMODULE GetModuleHandleA(LPCSTR)  { return (HMODULE)(LONG_PTR)1; }
HMODULE GetModuleHandleW(LPCWSTR) { return (HMODULE)(LONG_PTR)1; }
DWORD GetModuleFileNameA(HMODULE, LPSTR buf, DWORD size) {
    if (!buf || size == 0) return 0;
    ssize_t n = readlink("/proc/self/exe", buf, size - 1);
    if (n < 0) { buf[0] = 0; return 0; }
    buf[n] = 0; return (DWORD)n;
}
DWORD GetModuleFileNameW(HMODULE, LPWSTR buf, DWORD size) {
    if (!buf || size == 0) return 0;
    char tmp[4096]; ssize_t n = readlink("/proc/self/exe", tmp, sizeof(tmp) - 1);
    if (n < 0) { buf[0] = 0; return 0; }
    tmp[n] = 0;
    DWORD i = 0; for (; i + 1 < size && tmp[i]; ++i) buf[i] = (wchar_t)(unsigned char)tmp[i];
    buf[i] = 0; return i;
}

// ---- SYSTEMTIME <-> FILETIME (FILETIME = 100ns ticks since 1601-01-01) ----
static const unsigned long long EPOCH_DIFF_100NS = 116444736000000000ULL; // 1601->1970
BOOL SystemTimeToFileTime(const _SYSTEMTIME* st, _FILETIME* ft) {
    if (!st || !ft) return 0;
    struct tm t; std::memset(&t, 0, sizeof(t));
    t.tm_year = st->wYear - 1900; t.tm_mon = st->wMonth - 1; t.tm_mday = st->wDay;
    t.tm_hour = st->wHour; t.tm_min = st->wMinute; t.tm_sec = st->wSecond;
    time_t secs = timegm(&t);
    unsigned long long ticks = (unsigned long long)secs * 10000000ULL
                             + (unsigned long long)st->wMilliseconds * 10000ULL + EPOCH_DIFF_100NS;
    ft->dwLowDateTime  = (unsigned long)(ticks & 0xFFFFFFFFu);
    ft->dwHighDateTime = (unsigned long)(ticks >> 32);
    return 1;
}
BOOL FileTimeToSystemTime(const _FILETIME* ft, _SYSTEMTIME* st) {
    if (!ft || !st) return 0;
    unsigned long long ticks = ((unsigned long long)ft->dwHighDateTime << 32) | ft->dwLowDateTime;
    if (ticks < EPOCH_DIFF_100NS) ticks = EPOCH_DIFF_100NS;
    time_t secs = (time_t)((ticks - EPOCH_DIFF_100NS) / 10000000ULL);
    struct tm g; gmtime_r(&secs, &g);
    st->wYear = (unsigned short)(g.tm_year + 1900); st->wMonth = (unsigned short)(g.tm_mon + 1);
    st->wDayOfWeek = (unsigned short)g.tm_wday; st->wDay = (unsigned short)g.tm_mday;
    st->wHour = (unsigned short)g.tm_hour; st->wMinute = (unsigned short)g.tm_min;
    st->wSecond = (unsigned short)g.tm_sec;
    st->wMilliseconds = (unsigned short)(((ticks - EPOCH_DIFF_100NS) / 10000ULL) % 1000ULL);
    return 1;
}

} // extern "C" (templates can't have C linkage; reopened below)

// ---- MSVC radix int->string conversions ----
template <class CharT, class IntT>
static CharT* radix_to_str(IntT value, CharT* str, int radix) {
    static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    if (radix < 2 || radix > 36) { str[0] = 0; return str; }
    CharT* p = str;
    bool neg = (radix == 10 && value < 0);
    unsigned long long uv = neg ? (unsigned long long)(-(long long)value) : (unsigned long long)value;
    CharT tmp[66]; int n = 0;
    do { tmp[n++] = (CharT)digits[uv % (unsigned)radix]; uv /= (unsigned)radix; } while (uv);
    if (neg) *p++ = (CharT)'-';
    while (n > 0) *p++ = tmp[--n];
    *p = 0;
    return str;
}
extern "C" {
char*    _itoa(int value, char* str, int radix)        { return radix_to_str<char>(value, str, radix); }
wchar_t* _itow(int value, wchar_t* str, int radix)     { return radix_to_str<wchar_t>(value, str, radix); }
char*    _i64toa(long long value, char* str, int radix){ return radix_to_str<char>(value, str, radix); }

} // extern "C"
