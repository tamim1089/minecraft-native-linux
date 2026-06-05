#pragma once

#include "Recipy.h"

class ArmorDyeRecipe : public Recipy
{
public:
	bool matches(shared_ptr<CraftingContainer> craftSlots, Level *level);

	static shared_ptr<ItemInstance> assembleDyedArmor(shared_ptr<CraftingContainer> craftSlots);
	shared_ptr<ItemInstance> assemble(shared_ptr<CraftingContainer> craftSlots);

	int size();
	const ItemInstance *getResultItem();

	
	virtual const int getGroup();		

	virtual bool requires(int iRecipe);
	virtual void requires(INGREDIENTS_REQUIRED *pIngReq);
};