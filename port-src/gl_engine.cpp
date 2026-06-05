// port-src/gl_engine.cpp — engine side of the GL client: boots the real engine, generates the
// world, ticks it live, and exposes world/entity data through gl_engine.h. Includes stdafx.h;
// no GL/X11 headers here (kept separate from gl_main.cpp).
#include "stdafx.h"
#include "MinecraftServer.h"
#include "ServerLevel.h"
#include "ServerChunkCache.h"
#include "Level.h"
#include "LevelData.h"
#include "AABB.h"
#include "Vec3.h"
#include "IntCache.h"
#include "OldChunkStorage.h"
#include "Tile.h"
#include "Entity.h"
#include "Mob.h"
#include "ItemEntity.h"
#include "Pig.h"
#include "Cow.h"
#include "Sheep.h"
#include "Chicken.h"
#include "Wolf.h"
#include "Zombie.h"
#include "Skeleton.h"
#include "Creeper.h"
#include "Spider.h"
#include "Villager.h"
#include "compression.h"
#include "JavaMath.h"
#include "gl_engine.h"
#include <cstdio>
#include <vector>

extern void MinecraftWorld_RunStaticCtors();

static MinecraftServer* g_server = nullptr;
static ServerLevel*     g_level  = nullptr;
static int g_sx=0, g_sy=64, g_sz=0;
static std::vector<shared_ptr<Entity> > g_entSnap;

extern "C" int engine_boot(void) {
    AABB::CreateNewThreadStorage(); Vec3::CreateNewThreadStorage(); IntCache::CreateNewThreadStorage();
    Compression::CreateNewThreadStorage(); OldChunkStorage::CreateNewThreadStorage();
    Tile::CreateNewThreadStorage(); Entity::useSmallIds(); Level::enableLightingCache();

    MinecraftWorld_RunStaticCtors();
    g_NetworkManager.Initialise();
    g_NetworkManager.ServerReadyCreate(true);
    g_NetworkManager.ServerStoppedCreate(true);
    app.HeadlessInitGameSettings();
    Math::setRandomSeed(20240604);

    g_server = new MinecraftServer();
    NetworkGameInitData initData; initData.seed = 20240604;
    if (!g_server->headlessInit(20240604, &initData)) { std::printf("[engine] world init failed\n"); return 0; }
    g_level = g_server->getLevel(0);
    LevelData* ld = g_level->getLevelData();
    g_sx = ld->getXSpawn(); g_sy = ld->getYSpawn(); g_sz = ld->getZSpawn();
    std::printf("[engine] world generated, spawn=(%d,%d,%d)\n", g_sx,g_sy,g_sz);
    return 1;
}

extern "C" void engine_spawn(int* x,int* y,int* z){ if(x)*x=g_sx; if(y)*y=g_sy; if(z)*z=g_sz; }
extern "C" int  engine_getTile(int x,int y,int z){ if(!g_level||y<0||y>127) return 0; return g_level->getTile(x,y,z); }
extern "C" int  engine_setTile(int x,int y,int z,int id){ if(!g_level||y<0||y>127) return 0; return g_level->setTile(x,y,z,id)?1:0; }
extern "C" int  engine_getBrightness(int x,int y,int z){ if(!g_level||y<0||y>127) return 15; return g_level->getRawBrightness(x,y,z); }
extern "C" float engine_timeOfDay(void){ return g_level ? g_level->getTimeOfDay(0.0f) : 0.0f; }
extern "C" float engine_dayLight(void){
    if(!g_level) return 1.0f;
    long long t = (long long)g_level->getTime() % 24000; if(t<0) t+=24000;
    // Minecraft day: noon ~6000 (brightest), midnight ~18000 (darkest).
    float d = 0.5f + 0.5f*cosf((float)((t-6000)/24000.0)*2.0f*3.14159265f);
    return d<0?0:(d>1?1:d);
}
extern "C" void engine_setTime(long long t){ if(g_level) g_level->setTime((__int64)t); }
extern "C" long long engine_getTimeRaw(void){ return g_level ? (long long)g_level->getTime() : 0; }

extern "C" void engine_tick(void){
    if (!g_server || !g_level) return;
    g_server->headlessTick();
    g_level->tickEntities();   // server tick skips entities with 0 players; drive them directly
}

extern "C" void engine_ensureChunk(int chunkX,int chunkZ){
    if (!g_level) return;
    if (g_level->hasChunk(chunkX, chunkZ)) return;
    ServerChunkCache* cache = (ServerChunkCache*)g_level->getChunkSource();
    if (cache) cache->create(chunkX, chunkZ, false);
}

extern "C" void engine_spawnDemoMobs(int sx,int sy,int sz){
    if (!g_level) return;
    // Surface y near spawn: scan down from a high point to find the top solid block.
    auto surfaceY = [&](int x,int z)->int{ for(int y=110;y>1;--y) if(g_level->getTile(x,y,z)!=0) return y+1; return sy; };
    struct { int dx,dz; int kind; } spots[] = {
        {3,2,0},{5,3,0},{-2,4,1},{-4,6,1},{6,-3,2},{8,-1,2},{-4,-2,3},{2,7,3},   // animals
        {7,5,4},{-6,3,5},                                                         // wolf, villager
        {10,2,6},{-8,-4,7},{4,-8,8},{-10,5,9}                                     // zombie, skeleton, creeper, spider
    };
    int made=0;
    for (auto& s : spots) {
        int x=sx+s.dx, z=sz+s.dz, y=surfaceY(x,z);
        shared_ptr<Entity> e;
        switch (s.kind) {
            case 0: e.reset(new Pig(g_level)); break;
            case 1: e.reset(new Cow(g_level)); break;
            case 2: e.reset(new Sheep(g_level)); break;
            case 3: e.reset(new Chicken(g_level)); break;
            case 4: e.reset(new Wolf(g_level)); break;
            case 5: e.reset(new Villager(g_level)); break;
            case 6: e.reset(new Zombie(g_level)); break;
            case 7: e.reset(new Skeleton(g_level)); break;
            case 8: e.reset(new Creeper(g_level)); break;
            default: e.reset(new Spider(g_level)); break;
        }
        e->moveTo((double)x+0.5, (double)y, (double)z+0.5, 0.0f, 0.0f);
        if (g_level->addEntity(e)) made++;
    }
    std::printf("[engine] spawned %d demo mobs (animals + monsters + villager)\n", made);
}

extern "C" int engine_entitySnapshot(void){
    if (!g_level) { g_entSnap.clear(); return 0; }
    g_entSnap = g_level->getAllEntities();
    return (int)g_entSnap.size();
}
extern "C" int engine_getEntity(int i, float* x,float* y,float* z, float* w,float* h, int* type, float* yaw){
    if (i<0 || i>=(int)g_entSnap.size()) return 0;
    Entity* e = g_entSnap[i].get(); if(!e) return 0;
    if(x)*x=(float)e->x; if(y)*y=(float)e->y; if(z)*z=(float)e->z;
    if(w)*w=e->bbWidth>0?e->bbWidth:0.6f; if(h)*h=e->bbHeight>0?e->bbHeight:1.8f;
    if(yaw)*yaw=e->yRot;
    int t = ET_OTHER;
    if      (dynamic_cast<ItemEntity*>(e)) t = ET_ITEM;
    else if (dynamic_cast<Pig*>(e))        t = ET_PIG;
    else if (dynamic_cast<Cow*>(e))        t = ET_COW;
    else if (dynamic_cast<Sheep*>(e))      t = ET_SHEEP;
    else if (dynamic_cast<Chicken*>(e))    t = ET_CHICKEN;
    else if (dynamic_cast<Wolf*>(e))       t = ET_WOLF;
    else if (dynamic_cast<Villager*>(e))   t = ET_VILLAGER;
    else if (dynamic_cast<Zombie*>(e))     t = ET_ZOMBIE;
    else if (dynamic_cast<Skeleton*>(e))   t = ET_SKELETON;
    else if (dynamic_cast<Creeper*>(e))    t = ET_CREEPER;
    else if (dynamic_cast<Spider*>(e))     t = ET_SPIDER;
    else if (dynamic_cast<Mob*>(e))        t = ET_MOB;
    if(type)*type=t;
    return 1;
}
