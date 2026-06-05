// port-src/gl_engine.h — engine-facing C interface for the GL client (keeps the engine's
// stdafx.h, whose fake-GL/Font/Screen/Display symbols clash with real GL/X11, out of gl_main.cpp).
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

int   engine_boot(void);                                  // boot + generate world; 1 = ok
void  engine_spawn(int* x, int* y, int* z);               // overworld spawn point
int   engine_getTile(int x, int y, int z);                // block id (0 = air)
int   engine_setTile(int x, int y, int z, int id);        // place/break a block
int   engine_getBrightness(int x, int y, int z);          // 0..15 combined light
void  engine_tick(void);                                  // advance the live world one game tick
float engine_timeOfDay(void);                             // engine sun-angle value
float engine_dayLight(void);                              // 0 (midnight) .. 1 (noon)
void  engine_setTime(long long t);                        // force world time
long long engine_getTimeRaw(void);                        // raw world tick count
void  engine_ensureChunk(int chunkX, int chunkZ);         // generate a chunk if not present
void  engine_spawnDemoMobs(int sx, int sy, int sz);       // spawn a few animals near a point

// Entity species codes (returned in *type by engine_getEntity).
enum {
    ET_OTHER=0, ET_ITEM=1,
    ET_PIG=2, ET_COW=3, ET_SHEEP=4, ET_CHICKEN=5, ET_WOLF=6, ET_VILLAGER=7,
    ET_ZOMBIE=8, ET_SKELETON=9, ET_CREEPER=10, ET_SPIDER=11,
    ET_MOB=12   // a mob we don't have a textured model for yet
};

// Entities: snapshot the live entity list, then read each by index.
int   engine_entitySnapshot(void);                        // returns count; call before getEntity
int   engine_getEntity(int i, float* x, float* y, float* z,
                       float* w, float* h, int* type, float* yaw);  // type: ET_* code; yaw deg; 1=ok

#ifdef __cplusplus
}
#endif
