#pragma once
#include "../Minecraft.World/MapItem.h"
class Options;
class Font;
class Textures;
class Player;
class MapItemSavedData;

class Minimap
{
private:
	static const int w = MapItem::IMAGE_WIDTH;
    static const int h = MapItem::IMAGE_HEIGHT;
#ifdef _LINUX
	static short LUT[256];	// 
#else
	static int LUT[256];	// 
#endif
	static bool genLUT;		// 
	int renderCount;		// 
	bool m_optimised;		// 
#ifdef _LINUX
	shortArray pixels;
#else
    intArray pixels;
#endif
    int mapTexture;
    Options *options;
    Font *font;

public:
	Minimap(Font *font, Options *options, Textures *textures, bool optimised = true); //  optimised param
	static void reloadColours();
    void render(shared_ptr<Player> player, Textures *textures, shared_ptr<MapItemSavedData> data, int entityId); //  entityId param
};
