#include "stdafx.h"
#include "../../../Minecraft.World/StringHelpers.h"
#include "StructureActionPlaceSpawner.h"
#include "../../../Minecraft.World/net.minecraft.world.level.levelgen.structure.h"
#include "../../../Minecraft.World/net.minecraft.world.level.h"
#include "../../../Minecraft.World/net.minecraft.world.level.tile.entity.h"

StructureActionPlaceSpawner::StructureActionPlaceSpawner()
{
	m_tile = Tile::mobSpawner_Id;
	m_entityId = L"Pig";
}

StructureActionPlaceSpawner::~StructureActionPlaceSpawner()
{
}

void StructureActionPlaceSpawner::writeAttributes(DataOutputStream *dos, UINT numAttrs)
{
	StructureActionPlaceBlock::writeAttributes(dos, numAttrs + 1);

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_entity);
	dos->writeUTF(m_entityId);
}

void StructureActionPlaceSpawner::addAttribute(const wstring &attributeName, const wstring &attributeValue)
{
	if(attributeName.compare(L"entity") == 0)
	{
		m_entityId = attributeValue;
#ifndef _CONTENT_PACKAGE
		wprintf(L"StructureActionPlaceSpawner: Adding parameter entity=%ls/n",m_entityId.c_str());
#endif
	}
	else
	{
		StructureActionPlaceBlock::addAttribute(attributeName, attributeValue);
	}
}

bool StructureActionPlaceSpawner::placeSpawnerInLevel(StructurePiece *structure, Level *level, BoundingBox *chunkBB)
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
		shared_ptr<MobSpawnerTileEntity> entity = dynamic_pointer_cast<MobSpawnerTileEntity>(level->getTileEntity( worldX, worldY, worldZ ));

#ifndef _CONTENT_PACKAGE
		wprintf(L"StructureActionPlaceSpawner - placing a %ls spawner at (%d,%d,%d)/n", m_entityId.c_str(), worldX, worldY, worldZ);
#endif
		if( entity != NULL )
		{
			entity->setEntityId(m_entityId);
		}
		return true;
	}
	return false;
}