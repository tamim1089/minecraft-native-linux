#pragma once
#include "TileEntityRenderer.h"
class SignModel;

class SignRenderer : public TileEntityRenderer
{
private:
	SignModel *signModel;
public:
	SignRenderer();	// added
	virtual void render(shared_ptr<TileEntity> sign, double x, double y, double z, float a, bool setColor, float alpha=1.0f, bool useCompiled = true); //  setColor param
};
