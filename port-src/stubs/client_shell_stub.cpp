// port-src/stubs/client_shell_stub.cpp — strong real stubs for the hot client-shell symbols
// that must return valid values (the auto_link_stubs.s weak no-ops cover everything else).
#include "stdafx.h"
#include "Minecraft.h"
#include <cstring>

// Minecraft::GetInstance() — the server/world reads pMinecraft->players/options/skins/etc.
// Most uses are null-guarded (`if (pMinecraft && ...)`). Return a single zeroed Minecraft-sized
// buffer so member-pointer reads yield null (guards skip) instead of dereferencing garbage.
// TODO(port): a real headless Minecraft (or a server-only context object) in a later phase.
#include "ProgressRenderer.h"
#include "Options.h"
#include "TexturePackRepository.h"
static unsigned char g_minecraftBuf[ sizeof(Minecraft) ] __attribute__((aligned(64))) = {0};

Minecraft* Minecraft::GetInstance() {
    Minecraft* mc = reinterpret_cast<Minecraft*>(g_minecraftBuf);
    static bool inited = false;
    if (!inited) {
        inited = true;
        // The server derefs a few Minecraft members unguarded; give them valid objects (their
        // .cpp files are compiled, so the vtables/methods are real). Expanded as crashes surface.
        mc->progressRenderer = new ProgressRenderer(mc);
        mc->options          = new Options();
        // skins: only inline member-reads are exercised headless (isUsingDefaultSkin etc.);
        // a zeroed buffer is a safe valid `this`. (TexturePackRepository.cpp is excluded.)
        static unsigned char skinsBuf[ sizeof(TexturePackRepository) ] __attribute__((aligned(16))) = {0};
        mc->skins            = reinterpret_cast<TexturePackRepository*>(skinsBuf);
    }
    return mc;
}

// --- Strong stubs for stubbed functions that return NON-TRIVIAL types by value. The weak
// asm no-ops can't construct these (they'd leave the return object uninitialized -> garbage
// std::string length -> bad_alloc). Provide real, empty returns. ---
#include "4J_Storage.h"
std::string CStorage::GetMountedPath(std::string /*szMount*/) { return std::string(); }

// NOTE: C_Profile::GetGameDefinedProfileData is already provided by Extrax64Stubs.cpp
// (returns profileData[iQuadrant], a real member buffer) — do not duplicate it here.
