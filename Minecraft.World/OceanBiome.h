#pragma once

#include "Biome.h"

class OceanBiome : public Biome
{
public:
	OceanBiome(int id) : Biome(id)
	{
		friendlies.clear();
		friendlies_chicken.clear();	//  since chicken now separated from main friendlies
		friendlies_wolf.clear();	//  since wolf now separated from main friendlies
	}
};
