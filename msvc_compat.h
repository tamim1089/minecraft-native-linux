// msvc_compat.h — MSVC-to-MinGW/GCC compatibility shim.
// Force-included into every translation unit via CMake (-include).
// Provides MSVC-specific keywords/intrinsics that GCC (MinGW-w64) lacks.
#pragma once

#ifndef MSVC_COMPAT_H
#define MSVC_COMPAT_H

// MSVC implicitly exposes these via its prefix headers; GCC needs them
// explicitly. Pull them in once so FLT_MAX/DBL_MAX/INT_MAX etc. are always
// available without per-file includes.
#ifdef __cplusplus
#include <cfloat>
#include <climits>
#endif

// ---- Sized integer keywords (MSVC) -------------------------------
// MinGW GCC does not recognise the __intN keyword family.
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

// ---- Inlining / calling-convention hints -------------------------
#ifndef __forceinline
#define __forceinline inline __attribute__((always_inline))
#endif

#endif // MSVC_COMPAT_H
