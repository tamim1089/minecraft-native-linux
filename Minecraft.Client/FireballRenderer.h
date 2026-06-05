#pragma once
#include "EntityRenderer.h"

class FireballRenderer : public EntityRenderer
{
private:
	float scale;

public:
	FireballRenderer(float scale);

	virtual void render(shared_ptr<Entity> _fireball, double x, double y, double z, float rot, float a);

private:
	//  override
	virtual void renderFlame(shared_ptr<Entity> e, double x, double y, double z, float a);
};
