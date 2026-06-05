# CMakeLists.linux.cmake — native Linux headless build (Phase 0).
# Included from CMakeLists.txt when LINUX_PORT is set (toolchain-linux.cmake).
# Goal: compile Minecraft.World + platform-neutral Client parts and link a headless ELF that
# initializes the engine and prints a log line. No rendering / audio / UI / GPU.

message(STATUS "==== Minecraft native-Linux headless port (Phase 0) ====")

# C++14, NOT 17: the engine defines `typedef unsigned char byte;` and uses `using namespace std;`
# everywhere — C++17's std::byte makes every bare `byte` ambiguous. C++14 has no std::byte.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(ROOT_DIR    "${CMAKE_SOURCE_DIR}")
set(WORLD_DIR   "${ROOT_DIR}/Minecraft.World")
set(CLIENT_DIR  "${ROOT_DIR}/Minecraft.Client")
set(LIBS_WIN64  "${CLIENT_DIR}/Windows64")
set(COMPAT_DIR  "${ROOT_DIR}/compat")

# ---- Global preprocessor configuration -------------------------------------
# _WINDOWS64 keeps every platform #ifdef on the desktop path (avoids falling through to the
# console #else branches). _LINUX gates the raw-OS includes and selects POSIX implementations.
add_compile_definitions(
    _WINDOWS64
    _LINUX
    _LARGE_WORLDS
    _DEBUG_MENUS_ENABLED
    _DEBUG
    UNICODE
    _UNICODE
    _CRT_SECURE_NO_WARNINGS
    _CRT_NON_CONFORMING_SWPRINTFS
)

# Force-include the compatibility shims into every TU (no per-file edits).
# SHELL: prevents CMake from de-duplicating the repeated -include flag (which would otherwise
# collapse to "-include a.h b.h" and treat b.h as an input source).
add_compile_options(
    "SHELL:-include ${COMPAT_DIR}/msvc_compat.h"
    "SHELL:-include ${COMPAT_DIR}/win_types.h"
)

# Old-MSVC-targeted C++ needs leniency; silence the noise for Phase 0.
add_compile_options(
    -fpermissive
    -fno-strict-aliasing
    -w                       # Phase 0/1: quiet warnings, surface only hard errors
    -g                       # debug info for the Phase 1 runtime bring-up
)

include_directories(
    "${COMPAT_DIR}"
    # CLIENT_DIR before WORLD_DIR so Client files in subdirs (Common/...) whose own dir has no
    # stdafx.h resolve `#include "stdafx.h"` to the CLIENT stdafx (with UI/audio/app surface),
    # not the World one. World files live in Minecraft.World/ and get their own-dir stdafx first.
    "${CLIENT_DIR}"
    "${WORLD_DIR}"
    "${WORLD_DIR}/x64headers"
    "${ROOT_DIR}"
    "${CLIENT_DIR}/Windows64/4JLibs/inc"
    "${LIBS_WIN64}/Iggy/include"
    "${LIBS_WIN64}/Miles/include"
    "${CLIENT_DIR}/Common"
    "${CLIENT_DIR}/Common/Network"
    "${CLIENT_DIR}/Common/Audio"
    "${CLIENT_DIR}/Common/UI"
    "${CLIENT_DIR}/Common/GameRules"
    "${CLIENT_DIR}/Common/DLC"
    "${CLIENT_DIR}/Common/Colours"
    "${CLIENT_DIR}/Common/Telemetry"
    "${CLIENT_DIR}/Common/Tutorial"
    "${CLIENT_DIR}/Common/Trial"
    "${CLIENT_DIR}/Windows64"
    "${CLIENT_DIR}/Windows64/Sentient"
    "${CLIENT_DIR}/Windows64/GameConfig"
    "${CLIENT_DIR}/Windows64/XML"
    "${CLIENT_DIR}/Windows64/Social"
    "${CLIENT_DIR}/Windows64Media"
)

# ============================================================
#  Minecraft.World — engine static library
# ============================================================
file(GLOB_RECURSE WORLD_SOURCES "${WORLD_DIR}/*.cpp")
list(FILTER WORLD_SOURCES EXCLUDE REGEX ".*(PS3|PS4|Orbis|PSVita|Durango|Xbox|xbox).*\\.cpp$")
list(FILTER WORLD_SOURCES EXCLUDE REGEX ".*MemoryLevelStorage(Source)?\\.cpp$")
# Orphaned: SkyIslandDimension.h was never shipped and nothing but the .vcxproj references the
# class, so it can't be instantiated anywhere. Exclude (matches 4J's own dead-code handling).
list(FILTER WORLD_SOURCES EXCLUDE REGEX ".*SkyIslandDimension\\.cpp$")
# ZonedChunkStorage: 4J comment says "since we never use it anyway"; its impl targets an old
# LevelChunk API (skyLight/blocks/data members that no longer exist). Only NbtSlotFile uses its
# header constant CHUNKS_PER_ZONE, so the .cpp is safe to drop.
list(FILTER WORLD_SOURCES EXCLUDE REGEX ".*ZonedChunkStorage\\.cpp$")

add_library(Minecraft.World STATIC ${WORLD_SOURCES})

# ============================================================
#  Minecraft.Client — server-side + app (compiled selectively for headless)
# ============================================================
# Compile the Client tree EXCEPT the console platform dirs and the pure render/UI/platform-shell
# files that need real D3D11/Iggy. The server path (MinecraftServer/ServerLevel/ServerChunkCache/
# app/network) lives here; unused renderer objects simply won't be pulled into the exe.
file(GLOB_RECURSE CLIENT_SOURCES "${CLIENT_DIR}/*.cpp")
list(FILTER CLIENT_SOURCES EXCLUDE REGEX ".*/(PS3|PS4|Orbis|PSVita|Durango|Xbox|XboxMedia|PS3Media|OrbisMedia|PSVitaMedia|DurangoMedia)/.*")
list(FILTER CLIENT_SOURCES EXCLUDE REGEX ".*(Iggy|iggy)/.*")          # Iggy UI middleware (closed)
list(FILTER CLIENT_SOURCES EXCLUDE REGEX ".*/Windows64_Minecraft\\.cpp$")   # WinMain + D3D bootstrap
list(FILTER CLIENT_SOURCES EXCLUDE REGEX ".*miles_stub\\.cpp$")        # provide our own audio stub
# --- Headless server build: drop the rendering / UI / audio half of the client. The server
#     path (MinecraftServer/ServerLevel/ServerChunkCache/app/network/gamerules) does not need
#     them; any symbol that IS referenced gets a link stub in port-src/stubs/. ---
list(FILTER CLIENT_SOURCES EXCLUDE REGEX ".*/Common/(UI|XUI|Audio)/.*")
list(FILTER CLIENT_SOURCES EXCLUDE REGEX ".*/Common/Network/Sony/.*")
list(FILTER CLIENT_SOURCES EXCLUDE REGEX ".*(Renderer|Model|Screen)\\.cpp$")
list(FILTER CLIENT_SOURCES EXCLUDE REGEX ".*Particle.*\\.cpp$")
# Specific render/texture/UI-shell files (DirectXMath / GPU / font / texture-pack heavy):
list(FILTER CLIENT_SOURCES EXCLUDE REGEX ".*/(Camera|Frustum|Minimap|CompassTexture|DLCTexturePack|GuiComponent|ScreenSizeCalculator|glWrapper|Tesselator|stubs|GameRenderer|LevelRenderer|Lighting|ItemInHandRenderer|Textures|Texture|Font|TexturePack|AbstractTexturePack|DefaultTexturePack|FolderTexturePack|FileTexturePack|TextureMap|PreStitchedTextureMap)\\.cpp$")
# Client-side gameplay / player classes (reference MultiPlayerLevel / MultiplayerLocalPlayer —
# the client-multiplayer representation; the headless SERVER uses ServerLevel/ServerPlayer).
list(FILTER CLIENT_SOURCES EXCLUDE REGEX ".*/(LocalPlayer|RemotePlayer|CreativeMode|SurvivalMode|GameMode|MultiPlayerChunkCache|Minecraft|Tutorial)\\.cpp$")
list(FILTER CLIENT_SOURCES EXCLUDE REGEX ".*/Common/Tutorial/.*")
list(FILTER CLIENT_SOURCES EXCLUDE REGEX ".*/Windows64_UIController\\.cpp$")
# Re-include a few "*Renderer.cpp" files that are actually lightweight, non-GPU, and needed by
# the server path (progress feedback) — the broad *Renderer exclude over-matched them.
list(APPEND CLIENT_SOURCES "${CLIENT_DIR}/ProgressRenderer.cpp")
list(REMOVE_DUPLICATES CLIENT_SOURCES)
if(CLIENT_SOURCES)
    add_library(Minecraft.Client STATIC ${CLIENT_SOURCES})
    add_dependencies(Minecraft.Client Minecraft.World)
endif()

# ============================================================
#  Platform / stub support + headless entry point
# ============================================================
# Shared support: Win32 shims + link stubs (reused by every port executable).
enable_language(ASM)
set(PORT_SUPPORT
    "${COMPAT_DIR}/win_compat.cpp"        # timing/misc Win32 stubs
    "${COMPAT_DIR}/win_threads.cpp"       # std::thread-backed Win32 thread/event API
    "${COMPAT_DIR}/win_files.cpp"         # POSIX-backed Win32 file API
)
file(GLOB PORT_STUBS "${ROOT_DIR}/port-src/stubs/*.cpp")
set(PORT_ASM_STUBS "${ROOT_DIR}/port-src/stubs/auto_link_stubs.s")

# --start-group resolves the cyclic World<->Client symbol references.
set(ENGINE_LIBS -Wl,--start-group Minecraft.Client Minecraft.World -Wl,--end-group pthread z)

# Headless authoritative server (Phase 1).
add_executable(minecraft_headless
    ${PORT_SUPPORT} ${PORT_STUBS} ${PORT_ASM_STUBS}
    "${ROOT_DIR}/port-src/headless_server_main.cpp")
target_link_libraries(minecraft_headless ${ENGINE_LIBS})

# GL window (Phase 2/3): renders the engine's generated world via X11/GLX + the real terrain atlas.
add_executable(minecraft_gl
    ${PORT_SUPPORT} ${PORT_STUBS} ${PORT_ASM_STUBS}
    "${ROOT_DIR}/port-src/gl_engine.cpp"     # engine side (includes stdafx)
    "${ROOT_DIR}/port-src/gl_main.cpp")       # GL/X11 side (no stdafx)
target_link_libraries(minecraft_gl ${ENGINE_LIBS} GL X11 Xi png)
