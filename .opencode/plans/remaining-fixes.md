# Remaining Fixes Implementation Plan

## Files and Edits

### C3: Replace duplicated video physics (gl_main.cpp:1063-1069)
**File:** `/home/hex/minecraft-native-linux/port-src/gl_main.cpp`
**Old:**
```cpp
            float yr=pl.yaw*M_PI/180; float fxv=-sinf(yr),fzv=-cosf(yr),rxv=cosf(yr),rzv=-sinf(yr);
            float acc=0.09f;
            pl.vx+=(fxv*fwd+rxv*strafe)*acc; pl.vz+=(fzv*fwd+rzv*strafe)*acc;
            pl.vy-=0.08f; pl.vy*=0.98f; pl.onGround=false;
            moveAxis(pl,pl.vx,0,0); moveAxis(pl,0,0,pl.vz); moveAxis(pl,0,pl.vy,0);
            pl.vx*=0.60f; pl.vz*=0.60f;
            if(pl.onGround && fwd>0 && fabsf(pl.vx)+fabsf(pl.vz)<0.02f) pl.vy=0.42f;  // auto-jump if stuck
```
**New:**
```cpp
            bool autoJump = pl.onGround && fwd>0 && fabsf(pl.vx)+fabsf(pl.vz) < 0.02f;
            stepPhysics(pl, fwd, strafe, 0, autoJump, 1, sy);
```

### C4: Error checks (gl_main.cpp ~line 753)
After `glXCreateContext`, add:
```cpp
if(!ctx){printf("[gl] no GL context\n");return 1;}
```
After `glXMakeCurrent`, add:
```cpp
if(!glXMakeCurrent(dpy,win,ctx)){ printf("[gl] makeCurrent failed\n"); return 1; }
```

### C5: SIGINT handler (gl_main.cpp, near top and ~line 733)
Near top, after includes:
```cpp
#include <signal.h>
static Display* signal_dpy = nullptr;
static Window signal_win = 0;
static void handle_sigint(int) {
    if(signal_dpy){ XUngrabPointer(signal_dpy,CurrentTime); XUndefineCursor(signal_dpy,signal_win); XFlush(signal_dpy); }
    _exit(128+SIGINT);
}
```
At line 744 after XOpenDisplay:
```cpp
signal_dpy = dpy; signal_win = win; signal(SIGINT, handle_sigint);
```

### H1: HRESULT int32_t (win_types.h:53)
**Old:** `typedef long HRESULT;`
**New:** `typedef int32_t HRESULT;`

### H4: Event handlers (gl_main.cpp)
After the FocusOut handler, add Expose handler:
```cpp
else if(e.type==Expose){ /* will be redrawn on next frame */ }
```
After ClientMessage handler, add DestroyNotify handling by checking if DestroyNotify is in the event type.

### H6: SavePNG label — search for dead label in gl_main.cpp
(Need to verify if present)

### H7: M_PI fallback (gl_main.cpp, before first use)
Add near top after includes:
```cpp
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
```

### H9: Dangling paths (CMakeLists.linux.cmake:74-76)
Remove:
```
    "${LIBS_WIN64}/Miles/include"
    "${CLIENT_DIR}/Windows64/4JLibs/inc"
    "${LIBS_WIN64}/Iggy/include"
```
(These paths don't exist on Linux and serve no purpose)

### M1-M10: Various medium fixes
- M2: Add `resDir()` helper in gl_main.cpp
- M5: Check ffmpeg popen+fwrite return values
- M6: Extract magic numbers to constexpr
- M8: Honor VirtualAlloc flags
- M10: Map __declspec(align(n)) properly
