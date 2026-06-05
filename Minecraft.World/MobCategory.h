#pragma once
using namespace std;

class Material;

class MobCategory
{
public:
	static const int CONSOLE_MONSTERS_HARD_LIMIT = 50;									// Max number of enemies (skeleton, zombie, creeper etc) that the mob spawner will produce
	static const int CONSOLE_ANIMALS_HARD_LIMIT = 50;									// Max number of animals (cows, sheep, pigs) that the mob spawner will produce	

	static const int MAX_LINUX_CHICKENS = 8;										// Max number of chickens that the mob spawner will produce
	static const int MAX_LINUX_WOLVES = 8;										// Max number of wolves that the mob spawner will produce
	static const int MAX_LINUX_MUSHROOMCOWS = 2;									// Max number of mushroom cows that the mob spawner will produce
	static const int MAX_SNOWMEN = 16;										// Max number of snow golems that can be created by placing blocks -  increased limit due to player requests
	static const int MAX_IRONGOLEM = 16;									// Max number of iron golems that can be created by placing blocks -  increased limit due to player requests
	static const int CONSOLE_SQUID_HARD_LIMIT = 5;
	
	static const int MAX_LINUX_ANIMALS_WITH_BREEDING = CONSOLE_ANIMALS_HARD_LIMIT + 20;	// Max number of animals that we can produce (in total), when breeding
	static const int MAX_LINUX_CHICKENS_WITH_BREEDING = MAX_LINUX_CHICKENS + 8;	// Max number of chickens that we can produce (in total), when breeding/hatching
	static const int MAX_LINUX_MUSHROOMCOWS_WITH_BREEDING = MAX_LINUX_MUSHROOMCOWS + 20;	// Max number of mushroom cows that we can produce (in total), when breeding
	static const int MAX_LINUX_WOLVES_WITH_BREEDING = MAX_LINUX_WOLVES + 8;		// Max number of wolves that we can produce (in total), when breeding
	static const int MAX_VILLAGERS_WITH_BREEDING = 35;

	static const int MAX_LINUX_ANIMALS_WITH_SPAWN_EGG = MAX_LINUX_ANIMALS_WITH_BREEDING + 20;
	static const int MAX_LINUX_CHICKENS_WITH_SPAWN_EGG = MAX_LINUX_CHICKENS_WITH_BREEDING + 10;
	static const int MAX_LINUX_WOLVES_WITH_SPAWN_EGG = MAX_LINUX_WOLVES_WITH_BREEDING + 10;
	static const int MAX_LINUX_MONSTERS_WITH_SPAWN_EGG = CONSOLE_MONSTERS_HARD_LIMIT + 20;
	static const int MAX_LINUX_VILLAGERS_WITH_SPAWN_EGG = MAX_VILLAGERS_WITH_BREEDING + 15; // increased this limit due to player requests
	static const int MAX_LINUX_MUSHROOMCOWS_WITH_SPAWN_EGG = MAX_LINUX_MUSHROOMCOWS_WITH_BREEDING + 8;
	static const int MAX_LINUX_SQUIDS_WITH_SPAWN_EGG = CONSOLE_SQUID_HARD_LIMIT + 8;

	/*
		Maximum animals = 50 + 20 + 20 = 90
		Maximum monsters = 50 + 20 = 70
		Maximum chickens = 8 + 8 + 10 = 26
		Maximum wolves = 8 + 8 + 10 = 26
		Maximum mooshrooms = 2 + 20 + 8 = 30
		Maximum snowmen = 16
		Maximum iron golem = 16
		Maximum squid = 5 + 8 = 13
		Maximum villagers = 35 + 15 = 50

		Maximum natural = 50 + 50 + 8 + 8 + 2 + 5 + 35 = 158
		Total maxium = 90 + 70 + 26 + 26 + 30 + 16 + 16 + 13 + 50 = 337
	*/

	static MobCategory *monster;
	static MobCategory *creature;
	static MobCategory *waterCreature;
	//  extra categories, to break these out of general creatures & give us more control of levels
	static MobCategory *creature_wolf;
	static MobCategory *creature_chicken;
	static MobCategory *creature_mushroomcow;

	//  Sometimes we want to access the values by name, other times iterate over all values
	// Added these arrays so we can static initialise a collection which we can iterate over
	static MobCategoryArray values;


private:
	const int m_max;
	const int m_maxPerLevel;
	const Material *spawnPositionMaterial;
	const bool m_isFriendly;
	const bool m_isSingleType; // 
	const eINSTANCEOF m_eBase;	// 

	MobCategory(int maxVar, Material *spawnPositionMaterial, bool isFriendly, eINSTANCEOF eBase, bool isSingleType, int maxPerLevel);

public:
	const type_info getBaseClass();
	const eINSTANCEOF getEnumBaseClass();	// 
	int getMaxInstancesPerChunk();
	int getMaxInstancesPerLevel();		// 
	Material *getSpawnPositionMaterial();
	bool isFriendly();
	bool isSingleType();
public:
	static void staticCtor();
};
