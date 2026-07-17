// port-src/headless_server_main.cpp — Phase 1 headless authoritative server.
// Generates a world from a fixed seed, runs a real 20 Hz tick loop, ticks entities & world
// systems, saves, reloads, and verifies — with NO rendering / audio / UI.
#include "stdafx.h"
#include "MinecraftServer.h"
#include "ServerLevel.h"
#include "Level.h"
#include "LevelData.h"
#include "LevelStorage.h"
#include "ConsoleSaveFile.h"
#include "SharedConstants.h"
#include "AABB.h"
#include "Vec3.h"
#include "IntCache.h"
#include "OldChunkStorage.h"
#include "Tile.h"
#include "Entity.h"
#include "ItemEntity.h"
#include "compression.h"
#include "JavaMath.h"

#include <cstdio>
#include <cstdint>
#include <cstdlib>

// Engine bring-up (registers tiles/items/entities; starts the tile-update worker).
extern void MinecraftWorld_RunStaticCtors();

// Per-thread storage the engine requires on any thread that ticks/generates the world
// (mirrors GameNetworkManager's server-thread setup). Without these AABB::resetPool() etc. fault.
static void initThreadStorage() {
    AABB::CreateNewThreadStorage();
    Vec3::CreateNewThreadStorage();
    IntCache::CreateNewThreadStorage();
    // Use Create (not UseDefault) on this first/main thread — UseDefaultThreadStorage assumes a
    // prior CreateNewThreadStorage already established the shared default (done at process init
    // in the real game). Here the main thread IS the first thread.
    Compression::CreateNewThreadStorage();
    OldChunkStorage::CreateNewThreadStorage();
    Tile::CreateNewThreadStorage();
    Entity::useSmallIds();
    Level::enableLightingCache();
}

static const int   TPS       = SharedConstants::TICKS_PER_SECOND;   // 20
static const int   MS_PER_TICK = 1000 / TPS;

int main(int argc, char** argv) {
    std::setbuf(stdout, nullptr);
    std::printf("============================================================\n");
    std::printf(" Minecraft - headless server (Phase 1)\n");
    std::printf("============================================================\n");

    const long long SEED = 20240604;
    const int NUM_TICKS = (argc > 1) ? (int)std::strtol(argv[1], nullptr, 10) : 200;

    // Thread storage MUST be set up before the static ctors — tile/item registration calls
    // Tile::setShape() which writes through the per-thread TLS (TlsGetValue would be null otherwise).
    std::printf("[boot] thread storage ...\n");
    initThreadStorage();
    std::printf("[boot] MinecraftWorld_RunStaticCtors() ...\n");
    MinecraftWorld_RunStaticCtors();

    // Network manager must be initialised (creates the PlatformNetworkManagerStub) — the server
    // queries g_NetworkManager.IsInSession()/ServerReady() during load/save.
    std::printf("[boot] g_NetworkManager.Initialise() ...\n");
    g_NetworkManager.Initialise();
    g_NetworkManager.ServerReadyCreate(true);    // server signals these during init/stop
    g_NetworkManager.ServerStoppedCreate(true);

    // Populate app.GameSettingsA[] (per-user game settings) — read during world gen/tick. Now
    // that Minecraft::GetInstance()->options is valid, the SetDefaultOptions chain can run.
    std::printf("[boot] app.HeadlessInitGameSettings() ...\n");
    app.HeadlessInitGameSettings();

    // The global Math::rand is QPC-seeded by default (intentional, like Java's Math.random()) —
    // the one source of run-to-run non-determinism (entity velocity pops etc.). Seed it from our
    // fixed seed so the whole headless session is reproducible (proves no threading/UB non-determinism).
    Math::setRandomSeed(SEED);

    std::printf("[gen ] creating server, seed=%lld, ticks=%d\n", (long long)SEED, NUM_TICKS);
    MinecraftServer* server = new MinecraftServer();

    NetworkGameInitData initData;          // default ctor: new world, legacy size
    initData.seed = SEED;

    bool ok = server->headlessInit(SEED, &initData);
    std::printf("[gen ] headlessInit -> %s\n", ok ? "OK" : "FAILED");
    if (!ok) { std::printf("[fail] world init failed\n"); return 1; }

    ServerLevel* level = server->getLevel(0);
    LevelData* ld = level->getLevelData();
    int sx = ld->getXSpawn(), sy = ld->getYSpawn(), sz = ld->getZSpawn();
    std::printf("[gen ] overworld level=%p  spawn=(%d,%d,%d)\n", (void*)level, sx, sy, sz);

    // --- world-state fingerprint (deterministic for a given seed): mix block ids over a column
    //     region around spawn. Two identical-seed runs must produce the same value. ---
    auto fingerprint = [&](const char* tag) -> unsigned long long {
        unsigned long long h = 1469598103934665603ULL;            // FNV-1a 64
        for (int dx = -16; dx < 16; ++dx)
        for (int dz = -16; dz < 16; ++dz)
        for (int y = 0; y < 80; ++y) {
            int t = level->getTile(sx + dx, y, sz + dz);
            h ^= (unsigned long long)(t & 0xFF); h *= 1099511628211ULL;
        }
        std::printf("[%s] block-fingerprint=0x%016llx\n", tag, h);
        return h;
    };
    fingerprint("fp0 ");

    // --- spawn an entity (ItemEntity at spawn), tick it, observe deterministic state ---
    std::printf("[ent ] spawning ItemEntity at spawn ...\n");
    shared_ptr<ItemEntity> item(new ItemEntity(level, sx + 0.5, sy + 3.0, sz + 0.5));
    bool added = level->addEntity(item);
    std::printf("[ent ] addEntity -> %s, start y=%.3f\n", added ? "OK" : "NO", item->y);

    // --- 20 Hz tick loop (fixed timestep, real wall-clock paced) ---
    std::printf("[tick] running %d ticks @ %d Hz ...\n", NUM_TICKS, TPS);
    long long startMs = System::currentTimeMillis();
    for (int i = 0; i < NUM_TICKS; ++i) {
        long long t0 = System::currentTimeMillis();
        server->headlessTick();
        // MinecraftServer::tick() skips tickEntities() when there are 0 players; drive it directly
        // so spawned entities actually simulate in this headless (player-less) server.
        level->tickEntities();
        if ((i + 1) % 40 == 0) std::printf("[tick] %d/%d\n", i + 1, NUM_TICKS);
        long long elapsed = System::currentTimeMillis() - t0;
        if (elapsed < MS_PER_TICK) port_Sleep((unsigned)(MS_PER_TICK - elapsed));
    }
    long long wallMs = System::currentTimeMillis() - startMs;
    std::printf("[tick] done: %d ticks in %lld ms (target %lld ms)\n",
                NUM_TICKS, (long long)wallMs, (long long)(NUM_TICKS * (long long)MS_PER_TICK));

    // Entity state after ticking (gravity should have moved/settled it deterministically).
    std::printf("[ent ] after %d ticks: pos=(%.4f,%.4f,%.4f) age=%d removed=%d\n",
                NUM_TICKS, item->x, item->y, item->z, item->tickCount, (int)item->removed);
    unsigned long long fp1 = fingerprint("fp1 ");
    std::printf("DETERMINISM-FINGERPRINT: 0x%016llx ey=%.6f age=%d\n", fp1, item->y, item->tickCount);

    // --- save ---
    std::printf("[save] saveAllChunks() ...\n");
    server->headlessSave();
    std::printf("[save] done\n");

    // --- persist the in-memory ConsoleSaveFile to a real on-disk artifact (Saves/) ---
    LevelStorage* storage = level->getLevelStorage();
    ConsoleSaveFile* sf = storage->getSaveFile();
    if (sf) {
        std::printf("[save] flushing ConsoleSaveFile to disk (Saves/) ...\n");
        sf->DebugFlushToFile();
        std::printf("[save] on-disk save size = %zu bytes\n", (size_t)sf->getSizeOnDisk());
    }

    // --- RELOAD + VERIFY: re-read level.dat back through the engine's own load path
    //     (DirectoryLevelStorage::prepareLevel) and confirm the persisted level state matches. ---
    std::printf("[load] reloading level.dat via prepareLevel() ...\n");
    LevelData* reloaded = storage->prepareLevel();
    if (!reloaded) { std::printf("[load] FAILED: prepareLevel returned null\n"); return 1; }
    int rx = reloaded->getXSpawn(), ry = reloaded->getYSpawn(), rz = reloaded->getZSpawn();
    std::printf("[load] reloaded spawn=(%d,%d,%d)  original spawn=(%d,%d,%d)\n", rx, ry, rz, sx, sy, sz);
    bool verifyOk = (rx == sx && ry == sy && rz == sz);
    std::printf("[load] save->reload level-data verification: %s\n", verifyOk ? "PASS" : "FAIL");

    std::printf("[boot] headless server session complete.\n");
    return verifyOk ? 0 : 1;
}
