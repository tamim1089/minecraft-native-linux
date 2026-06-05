#pragma once

#include "StructureActionPlaceBlock.h"

class StructurePiece;
class Level;
class BoundingBox;
class GRFObject;

class StructureActionPlaceSpawner : public StructureActionPlaceBlock
{
private:
	wstring m_entityId;
public:
	StructureActionPlaceSpawner();
	~StructureActionPlaceSpawner();

	virtual ConsoleGameRules::EGameRuleType getActionType() { return ConsoleGameRules::eGameRuleType_PlaceSpawner; }
	
	virtual void writeAttributes(DataOutputStream *dos, UINT numAttrs);
	virtual void addAttribute(const wstring &attributeName, const wstring &attributeValue);

	bool placeSpawnerInLevel(StructurePiece *structure, Level *level, BoundingBox *chunkBB);
};