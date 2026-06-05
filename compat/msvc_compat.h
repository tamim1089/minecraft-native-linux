// compat/msvc_compat.h — MSVC language-extension shims for native GCC/Clang on Linux.
// Force-included into every translation unit (CMake -include). Companion: win_types.h.
// This file is ONLY about MSVC *language* extensions (keywords, calling conventions,
#pragma once
#ifndef PORT_MSVC_COMPAT_H
#define PORT_MSVC_COMPAT_H

// GCC defines these legacy, non-ISO macros in gnu++ mode. The engine uses `linux`/`unix` as
// so undefine them to avoid "expected identifier before numeric constant" corruption.
#undef linux
#undef unix
#undef i386

// ---- Sized integer keywords (MSVC) -------------------------------
#ifndef __int8
#define __int8  char
#endif
#ifndef __int16
#define __int16 short
#endif
#ifndef __int32
#define __int32 int
#endif
#ifndef __int64
#define __int64 long long
#endif

// ---- Calling conventions (meaningless on x86-64 SysV; erase them) -
#ifndef __cdecl
#define __cdecl
#endif
#ifndef __stdcall
#define __stdcall
#endif
#ifndef __fastcall
#define __fastcall
#endif
#ifndef __thiscall
#define __thiscall
#endif
#ifndef __vectorcall
#define __vectorcall
#endif
#ifndef WINAPI
#define WINAPI
#endif
#ifndef WINAPIV
#define WINAPIV
#endif
#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef CALLBACK
#define CALLBACK
#endif

// ---- Inlining / declspec -----------------------------------------
#ifndef __forceinline
#define __forceinline inline __attribute__((always_inline))
#endif
// __declspec(X) -> nothing. Covers dllexport/dllimport/align/novtable/selectany/noinline...
// (alignment hints are dropped; fine for a headless logic build.)
#ifndef __declspec
#define __declspec(x)
#endif
#ifndef __assume
#define __assume(x) ((void)0)
#endif
#ifndef __nounwind
#define __nounwind
#endif
#ifndef __pragma
#define __pragma(x)
#endif
// MSVC SAL annotations (sal.h). Erase the common ones.
#ifndef _In_
#define _In_
#define _In_opt_
#define _Out_
#define _Out_opt_
#define _Inout_
#define _Inout_opt_
#define _In_z_
#define _In_opt_z_
#define _Printf_format_string_
#define _Ret_maybenull_
// NOTE: do NOT define bare __in/__out/__inout — they collide with libstdc++ internal
// parameter names (e.g. std::move(__in) in <bits/stl_pair.h>). Old-style SAL is unused here.
#endif

// MSVC headers GCC needs explicitly (FLT_MAX/DBL_MAX/INT_MAX, etc.)
#ifdef __cplusplus
#include <cfloat>
#include <climits>
#endif

#endif // PORT_MSVC_COMPAT_H
