#include "stdafx.h"
#include "MushroomIslandBiome.h"
#include "BiomeDecorator.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.level.tile.h"

MushroomIslandBiome::MushroomIslandBiome(int id) : Biome(id)
{
    decorator->treeCount = -100;
    decorator->flowerCount = -100;
    decorator->grassCount = -100;

    decorator->mushroomCount = 1;
    decorator->hugeMushrooms = 1;

    topMaterial = (byte) Tile::mycel_Id;

    enemies.clear();
    friendlies.clear();
	friendlies_chicken.clear();	// 
	friendlies_wolf.clear(); 	// 
    waterFriendlies.clear();

    friendlies_mushroomcow.push_back(new MobSpawnerData(eTYPE_MUSHROOMCOW, 8, 4, 8));		//  to own category
}