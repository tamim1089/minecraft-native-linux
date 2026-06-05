// File/dir-find handles share WinHandleBase so CloseHandle()/FindClose() free them correctly.
#include "win_files.h"
#include "win_handle.h"

#include <string>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fnmatch.h>
#include <libgen.h>

namespace {

struct WinFile : WinHandleBase {
    int fd = -1;
    WinFile() { kind = K_FILE; }
    ~WinFile() override { if (fd >= 0) ::close(fd); }
};

struct WinFind : WinHandleBase {
    DIR*        dir = nullptr;
    std::string dirPath;
    std::string pattern;       // basename glob, e.g. "*"
    WinFind() { kind = K_FILE; }
    ~WinFind() override { if (dir) ::closedir(dir); }
};

// Narrow a wide path. Engine paths are ASCII/Latin-1; take the low byte of each wchar.
std::string narrow(LPCWSTR w) {
    std::string s;
    if (w) for (; *w; ++w) s.push_back((char)(*w & 0xFF));
    return s;
}

int open_flags(DWORD access, DWORD disposition) {
    int f = 0;
    bool wr = (access & GENERIC_WRITE) != 0;
    bool rd = (access & GENERIC_READ) != 0;
    if (wr && rd)      f = O_RDWR;
    else if (wr)       f = O_RDWR;   // engine reads back write handles in places; keep RDWR
    else               f = O_RDONLY;
    switch (disposition) {
        case CREATE_ALWAYS:     f |= O_CREAT | O_TRUNC; break;
        case CREATE_NEW:        f |= O_CREAT | O_EXCL;  break;
        case OPEN_ALWAYS:       f |= O_CREAT;           break;
        case TRUNCATE_EXISTING: f |= O_TRUNC;           break;
        case OPEN_EXISTING:     default:                break;
    }
    return f;
}

HANDLE create_file(const std::string& path, DWORD access, DWORD disposition) {
    int fd = ::open(path.c_str(), open_flags(access, disposition), 0644);
    if (fd < 0) return INVALID_HANDLE_VALUE;
    WinFile* h = new WinFile();
    h->fd = fd;
    return (HANDLE)h;
}

DWORD attrs_of(const std::string& path) {
    struct stat st;
    if (::stat(path.c_str(), &st) != 0) return INVALID_FILE_ATTRIBUTES;
    if (S_ISDIR(st.st_mode)) return FILE_ATTRIBUTE_DIRECTORY;
    return FILE_ATTRIBUTE_NORMAL;
}

void split_pattern(const std::string& pat, std::string& dir, std::string& glob) {
    std::string p = pat;
    for (auto& c : p) if (c == '\\') c = '/';
    size_t slash = p.find_last_of('/');
    if (slash == std::string::npos) { dir = "."; glob = p; }
    else { dir = p.substr(0, slash); glob = p.substr(slash + 1); if (dir.empty()) dir = "/"; }
    if (glob.empty()) glob = "*";
}

bool find_step(WinFind* h, std::string& nameOut, DWORD& attrOut) {
    struct dirent* de;
    while ((de = ::readdir(h->dir)) != nullptr) {
        if (fnmatch(h->pattern.c_str(), de->d_name, 0) == 0) {
            nameOut = de->d_name;
            attrOut = attrs_of(h->dirPath + "/" + de->d_name);
            return true;
        }
    }
    return false;
}

template <class FindData, class CharT>
void fill_find(FindData* fd, const std::string& name, DWORD attr) {
    std::memset(fd, 0, sizeof(*fd));
    fd->dwFileAttributes = attr;
    size_t i = 0;
    for (; i + 1 < (sizeof(fd->cFileName) / sizeof(CharT)) && name[i]; ++i)
        fd->cFileName[i] = (CharT)(unsigned char)name[i];
    fd->cFileName[i] = 0;
}

} // namespace

extern "C" {

HANDLE CreateFileA(LPCSTR name, DWORD access, DWORD, LPSECURITY_ATTRIBUTES,
                   DWORD disposition, DWORD, HANDLE) {
    return create_file(name ? name : "", access, disposition);
}
HANDLE CreateFileW(LPCWSTR name, DWORD access, DWORD, LPSECURITY_ATTRIBUTES,
                   DWORD disposition, DWORD, HANDLE) {
    return create_file(narrow(name), access, disposition);
}

BOOL ReadFile(HANDLE h, LPVOID buf, DWORD toRead, LPDWORD readOut, LPVOID) {
    WinFile* f = (WinFile*)h;
    if (!f || f->fd < 0) return FALSE;
    ssize_t n = ::read(f->fd, buf, toRead);
    if (n < 0) { if (readOut) *readOut = 0; return FALSE; }
    if (readOut) *readOut = (DWORD)n;
    return TRUE;
}

BOOL WriteFile(HANDLE h, LPCVOID buf, DWORD toWrite, LPDWORD writtenOut, LPVOID) {
    WinFile* f = (WinFile*)h;
    if (!f || f->fd < 0) return FALSE;
    ssize_t n = ::write(f->fd, buf, toWrite);
    if (n < 0) { if (writtenOut) *writtenOut = 0; return FALSE; }
    if (writtenOut) *writtenOut = (DWORD)n;
    return TRUE;
}

DWORD GetFileSize(HANDLE h, LPDWORD sizeHigh) {
    WinFile* f = (WinFile*)h;
    if (!f || f->fd < 0) return INVALID_FILE_SIZE;
    struct stat st;
    if (::fstat(f->fd, &st) != 0) return INVALID_FILE_SIZE;
    if (sizeHigh) *sizeHigh = (DWORD)((unsigned long long)st.st_size >> 32);
    return (DWORD)((unsigned long long)st.st_size & 0xFFFFFFFFu);
}

DWORD SetFilePointer(HANDLE h, LONG distLow, PLONG distHigh, DWORD method) {
    WinFile* f = (WinFile*)h;
    if (!f || f->fd < 0) return INVALID_FILE_SIZE;
    long long off = (long long)(unsigned long)(DWORD)distLow;
    if (distHigh) off |= ((long long)*distHigh) << 32;
    else          off = (long long)distLow;            // sign-extend when no high part
    int whence = (method == FILE_END) ? SEEK_END : (method == FILE_CURRENT) ? SEEK_CUR : SEEK_SET;
    off_t res = ::lseek(f->fd, (off_t)off, whence);
    if (res == (off_t)-1) return INVALID_FILE_SIZE;
    if (distHigh) *distHigh = (LONG)((unsigned long long)res >> 32);
    return (DWORD)((unsigned long long)res & 0xFFFFFFFFu);
}

BOOL SetEndOfFile(HANDLE h) {
    WinFile* f = (WinFile*)h;
    if (!f || f->fd < 0) return FALSE;
    off_t pos = ::lseek(f->fd, 0, SEEK_CUR);
    return ::ftruncate(f->fd, pos) == 0 ? TRUE : FALSE;
}
BOOL FlushFileBuffers(HANDLE h) {
    WinFile* f = (WinFile*)h;
    if (!f || f->fd < 0) return FALSE;
    return ::fsync(f->fd) == 0 ? TRUE : FALSE;
}

DWORD GetFileAttributesA(LPCSTR p)  { return attrs_of(p ? p : ""); }
DWORD GetFileAttributesW(LPCWSTR p) { return attrs_of(narrow(p)); }

static BOOL get_attr_ex(const std::string& path, LPVOID info) {
    struct stat st;
    if (::stat(path.c_str(), &st) != 0 || !info) return FALSE;
    WIN32_FILE_ATTRIBUTE_DATA* d = (WIN32_FILE_ATTRIBUTE_DATA*)info;
    std::memset(d, 0, sizeof(*d));
    d->dwFileAttributes = S_ISDIR(st.st_mode) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
    d->nFileSizeHigh = (DWORD)((unsigned long long)st.st_size >> 32);
    d->nFileSizeLow  = (DWORD)((unsigned long long)st.st_size & 0xFFFFFFFFu);
    return TRUE;
}
BOOL GetFileAttributesExA(LPCSTR p, GET_FILEEX_INFO_LEVELS, LPVOID info)  { return get_attr_ex(p ? p : "", info); }
BOOL GetFileAttributesExW(LPCWSTR p, GET_FILEEX_INFO_LEVELS, LPVOID info) { return get_attr_ex(narrow(p), info); }
BOOL  SetFileAttributesA(LPCSTR, DWORD)  { return TRUE; }
BOOL  SetFileAttributesW(LPCWSTR, DWORD) { return TRUE; }
BOOL  CreateDirectoryA(LPCSTR p, LPSECURITY_ATTRIBUTES)  { return (p && ::mkdir(p, 0755) == 0) ? TRUE : FALSE; }
BOOL  CreateDirectoryW(LPCWSTR p, LPSECURITY_ATTRIBUTES) { std::string s = narrow(p); return ::mkdir(s.c_str(), 0755) == 0 ? TRUE : FALSE; }
BOOL  RemoveDirectoryA(LPCSTR p)  { return (p && ::rmdir(p) == 0) ? TRUE : FALSE; }
BOOL  RemoveDirectoryW(LPCWSTR p) { std::string s = narrow(p); return ::rmdir(s.c_str()) == 0 ? TRUE : FALSE; }
BOOL  DeleteFileA(LPCSTR p)  { return (p && ::unlink(p) == 0) ? TRUE : FALSE; }
BOOL  DeleteFileW(LPCWSTR p) { std::string s = narrow(p); return ::unlink(s.c_str()) == 0 ? TRUE : FALSE; }
BOOL  MoveFileA(LPCSTR a, LPCSTR b)   { return (a && b && ::rename(a, b) == 0) ? TRUE : FALSE; }
BOOL  MoveFileW(LPCWSTR a, LPCWSTR b) { std::string sa = narrow(a), sb = narrow(b); return ::rename(sa.c_str(), sb.c_str()) == 0 ? TRUE : FALSE; }

static HANDLE find_first(const std::string& patternNarrow, std::string& name, DWORD& attr) {
    WinFind* h = new WinFind();
    split_pattern(patternNarrow, h->dirPath, h->pattern);
    h->dir = ::opendir(h->dirPath.c_str());
    if (!h->dir) { delete h; return INVALID_HANDLE_VALUE; }
    if (!find_step(h, name, attr)) { delete h; return INVALID_HANDLE_VALUE; }
    return (HANDLE)h;
}

HANDLE FindFirstFileA(LPCSTR pattern, LPWIN32_FIND_DATAA fd) {
    std::string name; DWORD attr;
    HANDLE h = find_first(pattern ? pattern : "", name, attr);
    if (h != INVALID_HANDLE_VALUE) fill_find<WIN32_FIND_DATAA, CHAR>(fd, name, attr);
    return h;
}
HANDLE FindFirstFileW(LPCWSTR pattern, LPWIN32_FIND_DATAW fd) {
    std::string name; DWORD attr;
    HANDLE h = find_first(narrow(pattern), name, attr);
    if (h != INVALID_HANDLE_VALUE) fill_find<WIN32_FIND_DATAW, WCHAR>(fd, name, attr);
    return h;
}
BOOL FindNextFileA(HANDLE h, LPWIN32_FIND_DATAA fd) {
    WinFind* f = (WinFind*)h; std::string name; DWORD attr;
    if (!f || !find_step(f, name, attr)) return FALSE;
    fill_find<WIN32_FIND_DATAA, CHAR>(fd, name, attr);
    return TRUE;
}
BOOL FindNextFileW(HANDLE h, LPWIN32_FIND_DATAW fd) {
    WinFind* f = (WinFind*)h; std::string name; DWORD attr;
    if (!f || !find_step(f, name, attr)) return FALSE;
    fill_find<WIN32_FIND_DATAW, WCHAR>(fd, name, attr);
    return TRUE;
}
BOOL FindClose(HANDLE h) {
    if (!h || h == INVALID_HANDLE_VALUE) return FALSE;
    delete static_cast<WinHandleBase*>(h);
    return TRUE;
}

} // extern "C"
