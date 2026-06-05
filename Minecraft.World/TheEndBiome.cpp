#include "stdafx.h"
#include "TheEndBiome.h"
#include "TheEndBiomeDecorator.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.level.tile.h"

TheEndBiome::TheEndBiome(int id) : Biome(id)
{
    enemies.clear();
    friendlies.clear();
	friendlies_chicken.clear();	// 
	friendlies_wolf.clear(); 	// 
    waterFriendlies.clear();

    enemies.push_back(new MobSpawnerData(eTYPE_ENDERMAN, 10, 4, 4));
    topMaterial = (byte) Tile::dirt_Id;
    this->material = (byte) Tile::dirt_Id;

    decorator = new TheEndBiomeDecorator(this);
}

// Don't need override
//int TheEndBiome::getSkyColor(float temp)
//{
//	return 0x000000;
//}