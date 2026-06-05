# Minecraft Linux Edition

A fully native Linux Minecraft implementation in C++ — OpenGL renderer, X11 window, no JVM, no Wine, no emulation. The world generates deterministically, mobs spawn and tick, physics run, and the full title screen / HUD / inventory render using the real texture atlases.

![gameplay](gameplay.gif)

---

## What's in this repo

```
Minecraft.World/       Engine — world gen, blocks, entities, physics, NBT, packets
Minecraft.Client/      Client — GUI screens, renderers, input, DLC, multiplayer
  Common/              Platform-neutral shared code (UI, audio, game rules, network)
compat/                Platform abstraction layer — threading, file I/O, misc APIs
port-src/              Entry points: OpenGL client + headless server
  gl_engine.cpp/.h     Engine bridge (chunk meshing, entity ticking, input)
  gl_main.cpp          X11/GLX window, raw mouse, PNG loader, renderer
  headless_server_main.cpp   Authoritative server without a window
  stubs/               Link stubs for unused subsystems
CMakeLists.txt
CMakeLists.linux.cmake Linux build configuration (included when LINUX_PORT=ON)
toolchain-linux.cmake  GCC toolchain definition
Common/res/            Runtime textures (terrain, font, gui, mob atlases)
play.sh                Launcher — sets DISPLAY and working directory
```

---

## System requirements

| | Minimum |
|---|---|
| OS | Any 64-bit Linux with X11 |
| GPU | OpenGL 3.3 capable (Mesa / NVIDIA / AMD all work) |
| Compiler | GCC 9+ or Clang 10+ |
| CMake | 3.16+ |
| Libraries | `libGL`, `libX11`, `libXi` (XInput2), `libpng`, `zlib`, `pthread` |

---

## Building from source

### 1 — Install dependencies

**Ubuntu / Debian / Mint**
```bash
sudo apt update
sudo apt install -y \
    build-essential cmake git \
    libgl-dev libx11-dev libxi-dev \
    libpng-dev zlib1g-dev
```

**Fedora / RHEL**
```bash
sudo dnf install -y \
    gcc-c++ cmake git \
    mesa-libGL-devel libX11-devel libXi-devel \
    libpng-devel zlib-devel
```

**Arch / Manjaro**
```bash
sudo pacman -S --needed \
    base-devel cmake git \
    mesa libx11 libxi \
    libpng zlib
```

---

### 2 — Clone the repo

```bash
git clone https://github.com/yourname/minecraft-native-linux.git
cd minecraft-native-linux
```

---

### 3 — Configure

The build system uses a separate build directory so the source tree stays clean. Pass `toolchain-linux.cmake` as the toolchain file — it sets `LINUX_PORT=ON` which activates the Linux-specific `CMakeLists.linux.cmake` configuration.

```bash
cmake -S . -B build-linux -DCMAKE_TOOLCHAIN_FILE=toolchain-linux.cmake
```

What this does:
- Selects GCC as the compiler
- Sets C++14 standard (required — C++17's `std::byte` conflicts with the engine's `typedef unsigned char byte`)
- Force-includes `compat/compat.h` and `compat/platform_types.h` into every translation unit so the engine's internal platform types resolve correctly
- Defines `_LINUX`, `_LARGE_WORLDS`, `UNICODE`, and build configuration macros

---

### 4 — Build

```bash
cmake --build build-linux -j$(nproc)
```

This compiles two targets:

| Target | Description |
|---|---|
| `minecraft_gl` | Full client — X11 window, OpenGL renderer, title screen, HUD, gameplay |
| `minecraft_headless` | Authoritative server — world ticks and logs to stdout, no window |

Build time is roughly 3–5 minutes on a modern machine.

---

### 5 — Run

```bash
# Must run from the repo root — the engine loads textures relative to ./Common/res/
./play.sh
```

Or directly:

```bash
DISPLAY=:0 ./build-linux/minecraft_gl
```

The headless server (no window, logs to stdout):

```bash
./build-linux/minecraft_headless
```

---

## Controls

| Key / Button | Action |
|---|---|
| `W A S D` | Move |
| `Space` | Jump / swim up |
| `Shift` | Descend (fly mode) |
| `Ctrl` | Sprint |
| `F` | Toggle fly / walk |
| `1–9` or scroll wheel | Select hotbar slot |
| Left mouse | Break block |
| Right mouse | Place block |
| `Esc` | Back to menu / pause |

---

![System architecture diagram](images/Sys-Diagram.png)

## How it works

The engine is split into two libraries that are compiled and linked together:

**`Minecraft.World`** is the pure game logic — world generation, chunk storage, block simulation, entity AI, physics, NBT serialization, and the packet protocol. It has no rendering dependencies and can run completely headless.

**`Minecraft.Client`** contains the renderer, GUI system, input handling, audio, and multiplayer logic. On Linux, the rendering backend is `port-src/gl_main.cpp`, a standalone X11/GLX file that opens a window, loads PNG texture atlases with `libpng`, and drives the engine through `gl_engine.h`. It uses XInput2 raw mouse events for FPS-style look with no cursor warping or acceleration.

The `compat/` directory provides the platform abstraction layer — Linux implementations of the threading, file I/O, and utility APIs the engine uses internally:

- `compat/win_threads.cpp` — thread creation, synchronization primitives, event objects backed by `std::thread` and `std::mutex`
- `compat/win_files.cpp` — file and directory APIs backed by standard POSIX `open`/`read`/`write`/`stat`
- `compat/win_compat.cpp` — high-resolution timing, string utilities, miscellaneous stubs

---

## Project structure notes

- **C++14 is required.** Do not upgrade to C++17 — the engine defines `typedef unsigned char byte` globally and uses `using namespace std;` across many files. C++17's `std::byte` makes every bare `byte` ambiguous and the build breaks with thousands of errors.
- **`-fpermissive` is set.** GCC rejects a handful of constructs (implicit conversions, forward-declared enums used before definition) that the engine relies on. `-fpermissive` downgrades those to warnings so the build completes cleanly.
- **Cyclic linking.** `Minecraft.World` and `Minecraft.Client` reference each other. The linker invocation wraps them in `--start-group ... --end-group` to allow multiple resolution passes and break the cycle.

---

## Troubleshooting

**`cannot open display` or black window**
Make sure `DISPLAY` is set. If you're in a remote session without a display server, run with `minecraft_headless` instead or set up Xvfb:
```bash
Xvfb :99 -screen 0 1280x720x24 &
DISPLAY=:99 ./build-linux/minecraft_gl
```

**Missing texture / `[gl] missing Common/res/...`**
The binary must be run from the repo root, not from inside `build-linux/`. Use `./play.sh` or `cd` to the repo root first.

**`undefined reference to ...` during link**
Make sure all three dependency groups are installed: `libGL-dev`, `libX11-dev` + `libXi-dev`, and `libpng-dev`. On some distros the `-dev` / `-devel` package is separate from the runtime library.

**Build fails with `std::byte` errors**
CMake picked up a C++17 flag from somewhere. Verify that `CMAKE_TOOLCHAIN_FILE=toolchain-linux.cmake` is being passed and that no system-level CMake preset is overriding `CMAKE_CXX_STANDARD`.

---

## License

Source provided for educational and preservation purposes.
