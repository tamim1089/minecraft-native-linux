# toolchain-linux.cmake — native Linux build of the headless engine (Phase 0).
# Usage:
#   cmake -S . -B build-linux -DCMAKE_TOOLCHAIN_FILE=toolchain-linux.cmake
#   cmake --build build-linux -j$(nproc)
#
# This is a NATIVE build (no CMAKE_SYSTEM_NAME -> not a cross-compile). It deliberately does
# NOT touch the frozen MinGW build in build/. The CMakeLists.txt LINUX_PORT branch keys off
# the LINUX_PORT variable set below.

set(CMAKE_C_COMPILER   gcc)
set(CMAKE_CXX_COMPILER g++)

# Flag consumed by CMakeLists.txt to select the headless-Linux configuration.
set(LINUX_PORT ON CACHE BOOL "Build the native-Linux headless port" FORCE)
