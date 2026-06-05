#include "stdafx.h"
#include "../../../Minecraft.World/StringHelpers.h"
#include "StructureActionPlaceContainer.h"
#include "AddItemRuleDefinition.h"
#include "../../../Minecraft.World/net.minecraft.world.level.levelgen.structure.h"
#include "../../../Minecraft.World/net.minecraft.world.level.h"
#include "../../../Minecraft.World/net.minecraft.world.level.tile.h"
#include "../../../Minecraft.World/net.minecraft.world.inventory.h"

StructureActionPlaceContainer::StructureActionPlaceContainer()
{
	m_tile = Tile::chest_Id;
}

StructureActionPlaceContainer::~StructureActionPlaceContainer()
{
	for(AUTO_VAR(it, m_items.begin()); it != m_items.end(); ++it)
	{
		delete *it;
	}
}

// Super class handles attr-facing fine.
//void StructureActionPlaceContainer::writeAttributes(DataOutputStream *dos, UINT numAttrs)
	

void StructureActionPlaceContainer::getChildren(vector<GameRuleDefinition *> *children)
{
	StructureActionPlaceBlock::getChildren(children);
	for(AUTO_VAR(it, m_items.begin()); it!=m_items.end(); it++)
		children->push_back( *it );
}

GameRuleDefinition *StructureActionPlaceContainer::addChild(ConsoleGameRules::EGameRuleType ruleType)
{
	GameRuleDefinition *rule = NULL;
	if(ruleType == ConsoleGameRules::eGameRuleType_AddItem)
	{
		rule = new AddItemRuleDefinition();
		m_items.push_back((AddItemRuleDefinition *)rule);
	}
	else
	{
#ifndef _CONTENT_PACKAGE
		wprintf(L"StructureActionPlaceContainer: Attempted to add invalid child rule - %d/n", ruleType );
#endif
	}
	return rule;
}

void StructureActionPlaceContainer::addAttribute(const wstring &attributeName, const wstring &attributeValue)
{
	if(attributeName.compare(L"facing") == 0)
	{
		int value = _fromString<int>(attributeValue);
		m_data = value;
		app.DebugPrintf("StructureActionPlaceContainer: Adding parameter facing=%d/n",m_data);
	}
	else
	{
		StructureActionPlaceBlock::addAttribute(attributeName, attributeValue);
	}
}

bool StructureActionPlaceContainer::placeContainerInLevel(StructurePiece *structure, Level *level, BoundingBox *chunkBB)
{
	int worldX = structure->getWorldX( m_x, m_z );
	int worldY = structure->getWorldY( m_y );
	int worldZ = structure->getWorldZ( m_x, m_z );

	if ( chunkBB->isInside( worldX, worldY, worldZ ) )
	{
		if ( level->getTileEntity( worldX, worldY, worldZ ) != NULL )
		{
			// Remove the current tile entity
			level->removeTileEntity( worldX, worldY, worldZ );
			level->setTile( worldX, worldY, worldZ, 0 );
		}

		level->setTile( worldX, worldY, worldZ, m_tile );
		shared_ptr<Container> container = dynamic_pointer_cast<Container>(level->getTileEntity( worldX, worldY, worldZ ));
		
		app.DebugPrintf("StructureActionPlaceContainer - placing a container at (%d,%d,%d)/n", worldX, worldY, worldZ);
		if ( container != NULL )
		{
			level->setData( worldX, worldY, worldZ, m_data);
			// Add items
			int slotId = 0;
			for(AUTO_VAR(it, m_items.begin()); it != m_items.end() && (slotId < container->getContainerSize()); ++it, ++slotId )
			{
				AddItemRuleDefinition *addItem = *it;

				addItem->addItemToContainer(container,slotId);
			}
		}
		return true;
	}
	return false;
}