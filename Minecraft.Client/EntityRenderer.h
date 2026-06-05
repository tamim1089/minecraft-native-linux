#pragma once

#include "Model.h"
#include "TileRenderer.h"
#include "Tesselator.h"
#include "Textures.h"
#include "ItemInHandRenderer.h"
class Tile;
class Entity;
class Level;
class AABB;
class IconRegister;

using namespace std;

class EntityRenderDispatcher;
class Font;

// this was originally a generic of type EntityRenderer<T extends Entity>
class EntityRenderer
{
	friend class PlayerRenderer; //  to allow PlayerRenderer to call renderShadow
protected:
	EntityRenderDispatcher *entityRenderDispatcher;

private:
	Model *model;			// TODO - check why exactly this is here, it seems to get shadowed by classes inheriting from this by their own
protected:
    TileRenderer *tileRenderer;	// changed to protected so derived classes can use instead of shadowing their own

protected:
	float shadowRadius;
    float shadowStrength;

public:
	EntityRenderer();	// added
	virtual ~EntityRenderer();
public:
	virtual void render(shared_ptr<Entity> entity, double x, double y, double z, float rot, float a) = 0;
protected:
	virtual void bindTexture(int resourceName);	// added
	virtual void bindTexture(const wstring& resourceName);

    virtual bool bindTexture(const wstring& urlTexture, int backupTexture);			// 
	virtual bool bindTexture(const wstring& urlTexture, const wstring& backupTexture);	
private:
	virtual void renderFlame(shared_ptr<Entity> e, double x, double y, double z, float a);
    virtual void renderShadow(shared_ptr<Entity> e, double x, double y, double z, float pow, float a);

    virtual Level *getLevel();
    virtual void renderTileShadow(Tile *tt, double x, double y, double z, int xt, int yt, int zt, float pow, float r, double xo, double yo, double zo);
public:
	virtual void render(AABB *bb, double xo, double yo, double zo);
    static void renderFlat(AABB *bb);
	static void renderFlat(float x0, float y0, float z0, float x1, float y1, float z1);
    virtual void init(EntityRenderDispatcher *entityRenderDispatcher);
    virtual void postRender(shared_ptr<Entity> entity, double x, double y, double z, float rot, float a, bool bRenderPlayerShadow);
    virtual Font *getFont();
	virtual void registerTerrainTextures(IconRegister *iconRegister);

public:
	virtual Model *getModel() { return model; }
	virtual void SetItemFrame(bool bSet) {}
};
