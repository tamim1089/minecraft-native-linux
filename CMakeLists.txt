cmake_minimum_required(VERSION 3.16)
project(MinecraftConsoles CXX)

# ============================================================
#  NATIVE LINUX HEADLESS PORT (Phase 0)
#  Selected by toolchain-linux.cmake (sets LINUX_PORT).
#  Handled in a self-contained file; returns early so the
#  frozen MinGW configuration below is never touched.
# ============================================================
if(LINUX_PORT)
    include("${CMAKE_SOURCE_DIR}/CMakeLists.linux.cmake")
    return()
endif()

# ============================================================
#  TOOLCHAIN — use MinGW x86_64 cross-compiler
#  Run cmake with:
#    cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw64.cmake ..
# ============================================================

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)   # use -std=gnu++11 for GNU/MS extensions

# Force-include MSVC compatibility shim into every translation unit
add_compile_options(-include "${CMAKE_SOURCE_DIR}/msvc_compat.h")

# ---- Global Windows64 preprocessor flags -------------------
add_compile_definitions(
    _WINDOWS64
    _LARGE_WORLDS
    _DEBUG_MENUS_ENABLED
    _DEBUG
    _CRT_NON_CONFORMING_SWPRINTFS
    _CRT_SECURE_NO_WARNINGS
    WIN32_LEAN_AND_MEAN
    UNICODE
    _UNICODE
    MULTITHREAD_ENABLE
)

# ---- Paths relative to this CMakeLists.txt -----------------
# Expected layout:
#   <root>/
#     CMakeLists.txt          <- this file
#     toolchain-mingw64.cmake
#     Minecraft.World/
#     Minecraft.Client/
#     build/                  <- cmake -B build

set(ROOT_DIR    "${CMAKE_SOURCE_DIR}")
set(WORLD_DIR   "${ROOT_DIR}/Minecraft.World")
set(CLIENT_DIR  "${ROOT_DIR}/Minecraft.Client")

# ---- 4JLibs, Iggy (already MinGW ar archives) --------------
set(LIBS_WIN64  "${CLIENT_DIR}/Windows64")

# Pre-built MinGW archives — import as interface targets
add_library(4J_Render   STATIC IMPORTED)
set_target_properties(4J_Render   PROPERTIES IMPORTED_LOCATION "${LIBS_WIN64}/4JLibs/libs/4J_Render.lib")

add_library(4J_Input    STATIC IMPORTED)
set_target_properties(4J_Input    PROPERTIES IMPORTED_LOCATION "${LIBS_WIN64}/4JLibs/libs/4J_Input.lib")

add_library(4J_Profile  STATIC IMPORTED)
set_target_properties(4J_Profile  PROPERTIES IMPORTED_LOCATION "${LIBS_WIN64}/4JLibs/libs/4J_Profile.lib")

add_library(4J_Storage  STATIC IMPORTED)
set_target_properties(4J_Storage  PROPERTIES IMPORTED_LOCATION "${LIBS_WIN64}/4JLibs/libs/4J_Storage.lib")

add_library(iggy_w64    STATIC IMPORTED)
set_target_properties(iggy_w64    PROPERTIES IMPORTED_LOCATION "${LIBS_WIN64}/Iggy/lib/iggy_w64.lib")

# ---- Miles stub (compiled from our stub source) ------------
add_library(miles_stub STATIC "${CLIENT_DIR}/miles_stub.cpp")
# If you put miles_stub.cpp inside the repo, adjust path accordingly:
# add_library(miles_stub STATIC "${CLIENT_DIR}/miles_stub.cpp")

# ============================================================
#  1.  Minecraft.World  (static library)
# ============================================================
file(GLOB_RECURSE WORLD_SOURCES
    "${WORLD_DIR}/*.cpp"
)

# Exclude platform-specific files we don't want
list(FILTER WORLD_SOURCES EXCLUDE REGEX ".*(PS3|PS4|Orbis|PSVita|Durango|Xbox|xbox).*\\.cpp$")

# Exclude files 4J themselves marked unused (see net.minecraft.world.level.storage.h)
list(FILTER WORLD_SOURCES EXCLUDE REGEX ".*MemoryLevelStorage(Source)?\\.cpp$")

add_library(Minecraft.World STATIC ${WORLD_SOURCES})

target_include_directories(Minecraft.World PUBLIC
    "${WORLD_DIR}"
    "${WORLD_DIR}/x64headers"
    "${CLIENT_DIR}"
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

target_compile_definitions(Minecraft.World PUBLIC
    _WINDOWS64
    _LARGE_WORLDS
    _DEBUG_MENUS_ENABLED
    _LIB
    _CRT_NON_CONFORMING_SWPRINTFS
    _CRT_SECURE_NO_WARNINGS
)

# Suppress noisy warnings common in old MSVC-targeted code
target_compile_options(Minecraft.World PRIVATE
    -Wno-deprecated
    -Wno-unknown-pragmas
    -Wno-narrowing
    -Wno-unused-variable
    -Wno-unused-function
    -Wno-reorder
    -Wno-sign-compare
    -Wno-char-subscripts
    -fpermissive          # old-style C++ patterns
)

# ============================================================
#  2.  Minecraft.Client  (executable)
# ============================================================
file(GLOB_RECURSE CLIENT_SOURCES
    "${CLIENT_DIR}/*.cpp"
)

# Keep only the root client files + Windows64 platform files
list(FILTER CLIENT_SOURCES EXCLUDE REGEX ".*(PS3|PS4|Orbis|PSVita|Durango|Xbox|xbox|PS3_|PSVita_|Orbis_|Durango_).*\\.cpp$")

# Include Windows64 platform entry points explicitly
file(GLOB WIN64_SOURCES
    "${CLIENT_DIR}/Windows64/*.cpp"
    "${CLIENT_DIR}/Common/*.cpp"
    "${CLIENT_DIR}/Common/Audio/*.cpp"
    "${CLIENT_DIR}/Common/Network/*.cpp"
    "${CLIENT_DIR}/Common/UI/*.cpp"
    "${CLIENT_DIR}/Common/GameRules/*.cpp"
    "${CLIENT_DIR}/Common/DLC/*.cpp"
    "${CLIENT_DIR}/Common/Colours/*.cpp"
    "${CLIENT_DIR}/Common/Telemetry/*.cpp"
    "${CLIENT_DIR}/Common/Tutorial/*.cpp"
    "${CLIENT_DIR}/Common/Trial/*.cpp"
)

list(APPEND CLIENT_SOURCES ${WIN64_SOURCES})
list(REMOVE_DUPLICATES CLIENT_SOURCES)

add_executable(Minecraft ${CLIENT_SOURCES})

target_include_directories(Minecraft PRIVATE
    "${CLIENT_DIR}"
    "${CLIENT_DIR}/Windows64"
    "${CLIENT_DIR}/Common"
    "${WORLD_DIR}"
    "${WORLD_DIR}/x64headers"
    "${CLIENT_DIR}/Windows64/4JLibs/inc"
    "${LIBS_WIN64}/Iggy/include"
    "${LIBS_WIN64}/Miles/include"
    "${LIBS_WIN64}/Sentient"
    "${LIBS_WIN64}/GameConfig"
)

target_compile_definitions(Minecraft PRIVATE
    _WINDOWS64
    _LARGE_WORLDS
    _DEBUG_MENUS_ENABLED
    _DEBUG
    _CRT_NON_CONFORMING_SWPRINTFS
    _CRT_SECURE_NO_WARNINGS
    WIN32_LEAN_AND_MEAN
    UNICODE
    _UNICODE
)

target_compile_options(Minecraft PRIVATE
    -Wno-deprecated
    -Wno-unknown-pragmas
    -Wno-narrowing
    -Wno-unused-variable
    -Wno-unused-function
    -Wno-reorder
    -Wno-sign-compare
    -Wno-char-subscripts
    -fpermissive
)

target_link_libraries(Minecraft PRIVATE
    Minecraft.World
    4J_Render
    4J_Input
    4J_Profile
    4J_Storage
    iggy_w64
    miles_stub
    # Windows system libs (provided by MinGW)
    d3d11
    dxgi
    xinput9_1_0
    winmm
    ws2_32
    ole32
    uuid
    -static-libgcc
    -static-libstdc++
)
