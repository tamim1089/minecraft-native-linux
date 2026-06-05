#pragma once

#include "StructureActionPlaceBlock.h"

class AddItemRuleDefinition;
class StructurePiece;
class Level;
class BoundingBox;

class StructureActionPlaceContainer : public StructureActionPlaceBlock
{
private:
	vector<AddItemRuleDefinition *> m_items;
public:
	StructureActionPlaceContainer();
	~StructureActionPlaceContainer();

	virtual ConsoleGameRules::EGameRuleType getActionType() { return ConsoleGameRules::eGameRuleType_PlaceContainer; }
	
	virtual void getChildren(vector<GameRuleDefinition *> *children);
	virtual GameRuleDefinition *addChild(ConsoleGameRules::EGameRuleType ruleType);
	
	// Super class handles attr-facing fine.
	//virtual void writeAttributes(DataOutputStream *dos, UINT numAttributes);
	
	virtual void addAttribute(const wstring &attributeName, const wstring &attributeValue);

	bool placeContainerInLevel(StructurePiece *structure, Level *level, BoundingBox *chunkBB);
};