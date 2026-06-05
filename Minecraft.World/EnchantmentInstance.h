#pragma once

#include "WeighedRandom.h"

class EnchantmentInstance : public WeighedRandomItem
{
public:
	const Enchantment *enchantment;
	const int level;

	EnchantmentInstance(Enchantment *enchantment, int level);
	EnchantmentInstance(int id, int level);

	EnchantmentInstance *copy();
};