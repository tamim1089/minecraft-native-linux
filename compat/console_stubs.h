// compat/console_stubs.h — Stub definitions for console-platform SDK headers.
// Included in place of the real SDK headers when building under __LINUX_PORT__.
// Defines only the types and constants that the engine code references.
#pragma once
#ifndef PORT_CONSOLE_STUBS_H
#define PORT_CONSOLE_STUBS_H

#include <cstdint>
#include <cstddef>

// ---- XDK / Xbox One stubs ----
typedef unsigned int       UINT;
typedef int                INT;
typedef void              *HANDLE;
typedef void              *HMODULE;
typedef unsigned long      DWORD;
typedef int                BOOL;
typedef wchar_t            WCHAR;
typedef unsigned int       UID;
typedef UID                PlayerUID;
typedef uint64_t           XNKID;
typedef XNKID              SessionID;
typedef UID                GameSessionUID;
typedef void              *HXUIOBJ;
typedef int                HRESULT;

struct XINVITE_INFO { DWORD dummy; };
typedef XINVITE_INFO INVITE_INFO;

struct XUSER_SIGNIN_INFO { DWORD dummy; };
struct XSESSION_SEARCHRESULT_HEADER { DWORD dummy; };
struct XNQOS { DWORD dummy; };
typedef void* XMEMCOMPRESSION_CONTEXT;
typedef void* XMEMDECOMPRESSION_CONTEXT;

// ---- DirectX math stubs ----
struct D3DXVECTOR3 { float x,y,z; };
struct D3DXCOLOR { float r,g,b,a; };
struct D3DXMATRIX { float m[16]; };

// ---- UI stubs ----
class CXuiScene {};
class CXuiControl {};
class CXuiProgressBar {};

// ---- Polygon ----
struct _Polygon { int dummy; };

// ---- PS3 SDK stubs ----
struct CellPadData { int dummy; };
struct CellPadInfo { int dummy; };
typedef uint32_t sys_event_flag_t;
typedef uint32_t SceKernelEventFlag;
typedef uint32_t SceUID;
typedef uint16_t SceKernelCpumask;
typedef uint32_t sys_ppu_thread_t;
typedef uint32_t ScePthreadAttr;
typedef uint32_t ScePthread;
typedef int32_t   SceInt32;
typedef uint32_t  SceSize;

// ---- CustomMap / CustomSet stubs (PS3-era custom containers) ----
template<typename K, typename V>
using CustomMap = std::unordered_map<K, V>;

template<typename T>
using CustomSet = std::unordered_set<T>;

// ---- CStorage stub (storage abstraction) ----
class CStorage {};

// ---- String-table resource IDs (from .rc / autogen headers) ----
const unsigned int IDS_TILE_ANVIL_INTACT = 0;
const unsigned int IDS_TILE_ANVIL_SLIGHTLYDAMAGED = 1;
const unsigned int IDS_TILE_ANVIL_VERYDAMAGED = 2;

// ---- Misc engine constants ----
const int XUSER_INDEX_ANY = 255;
const int XUSER_INDEX_FOCUS = 254;
const int XUSER_MAX_COUNT = 4;
const int MINECRAFT_NET_MAX_PLAYERS = 8;

// ---- Forward declarations for World library cross-dependencies ----
class LocalPlayer;

#endif // PORT_CONSOLE_STUBS_H
