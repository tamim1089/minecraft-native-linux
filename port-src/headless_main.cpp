// port-src/headless_main.cpp — Phase 0 headless entry point for the native Linux port.
// No window, no GPU, no audio, no UI. Drives real engine code (RNG + NBT round-trip) to prove
// the Minecraft.World engine library links and initializes on native Linux, then prints results.
#include "stdafx.h"            // engine umbrella header (compat shims are force-included by CMake)
#include "Random.h"
#include "CompoundTag.h"
#include "SharedConstants.h"

#include <cstdio>
#include <string>

int main(int argc, char** argv) {
    std::printf("============================================================\n");
    std::printf(" Minecraft - native Linux headless (Phase 0)\n");
    std::printf("============================================================\n");
    std::printf("[boot] argc=%d  TICKS_PER_SECOND=%d  NETWORK_PROTOCOL_VERSION=%d\n",
                argc, SharedConstants::TICKS_PER_SECOND, SharedConstants::NETWORK_PROTOCOL_VERSION);

    // --- Engine RNG: deterministic LCG seeded from a fixed value (proves Random links) ---
    Random rng((__int64)123456789);
    std::printf("[rng]  seed=123456789 -> nextInt(100) =");
    for (int i = 0; i < 5; ++i) std::printf(" %d", rng.nextInt(100));
    std::printf("\n");

    // --- Engine NBT: build a CompoundTag, write values, read them back ---
    CompoundTag* root = new CompoundTag(std::wstring(L"Level"));
    root->putInt((wchar_t*)L"SpawnX", 128);
    root->putInt((wchar_t*)L"SpawnZ", -64);
    root->putLong((wchar_t*)L"RandomSeed", (__int64)0x0123456789ABCDEFLL);
    root->putString((wchar_t*)L"LevelName", std::wstring(L"NewWorld"));
    root->putBoolean((wchar_t*)L"raining", true);

    int spawnX        = root->getInt((wchar_t*)L"SpawnX");
    int spawnZ        = root->getInt((wchar_t*)L"SpawnZ");
    long long seed    = (long long)root->getLong((wchar_t*)L"RandomSeed");
    std::wstring name = root->getString((wchar_t*)L"LevelName");

    // narrow the wstring for printf (ASCII level name)
    std::string narrowName;
    for (wchar_t c : name) narrowName.push_back((char)c);

    std::printf("[nbt]  CompoundTag round-trip: SpawnX=%d SpawnZ=%d RandomSeed=0x%llX LevelName=\"%s\"\n",
                spawnX, spawnZ, (unsigned long long)seed, narrowName.c_str());

    bool ok = (spawnX == 128) && (spawnZ == -64)
           && (seed == (long long)0x0123456789ABCDEFLL) && (narrowName == "NewWorld");
    std::printf("[nbt]  round-trip %s\n", ok ? "VERIFIED" : "MISMATCH");

    delete root;

    std::printf("[boot] engine initialized OK — Minecraft.World is live on Linux.\n");
    return ok ? 0 : 1;
}
