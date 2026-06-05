#pragma once
class LevelType;

class Abilities;
class LevelData;

// Was Java enum class
class GameType
{
public:
	static GameType *NOT_SET;
	static GameType *SURVIVAL;
	static GameType *CREATIVE;
	static GameType *ADVENTURE;

	static void staticCtor();

private:
	int id;
	wstring name;

	GameType(int id, const wstring &name);

public:
	int getId();
	wstring getName();
	void updatePlayerAbilities(Abilities *abilities);
	bool isReadOnly();
	bool isCreative();
	bool isSurvival();
	static GameType *byId(int id);
	static GameType *byName(const wstring &name);
};

class LevelSettings
{
private:
	__int64 seed;
	GameType *gameType;
	bool generateMapFeatures;
	bool hardcore;
	bool newSeaLevel;
	LevelType *levelType;
	bool allowCommands;
	bool startingBonusItems;	// brought forward from 1.3.2
	int m_xzSize;	// 
	int m_hellScale;

	void _init(__int64 seed, GameType *gameType, bool generateMapFeatures, bool hardcore, bool newSeaLevel, LevelType *levelType, int xzSize, int hellScale); //  xzSize and hellScale param

public:
	LevelSettings(__int64 seed, GameType *gameType, bool generateMapFeatures, bool hardcore, bool newSeaLevel, LevelType *levelType, int xzSize, int hellScale); //  xzSize and hellScale param
	LevelSettings(LevelData *levelData);
	LevelSettings *enableStartingBonusItems();		// brought forward from 1.3.2
	LevelSettings *enableSinglePlayerCommands();
	bool hasStartingBonusItems(); // brought forward from 1.3.2
	__int64 getSeed();
	GameType *getGameType();
	bool isHardcore();
	LevelType *getLevelType();
	bool getAllowCommands();
	bool isGenerateMapFeatures();
	bool useNewSeaLevel();
	int getXZSize(); // 
	int getHellScale(); // 
	static GameType *validateGameType(int gameType);
};
