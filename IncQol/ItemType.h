#ifndef ITEM_TYPE_H
#define ITEM_TYPE_H
#include <map>
#include "UpgradeComponentType.h"

enum ItemType {
	None = -1,
	Armor = 0,
	Back = 1,
	Bag = 2,
	Consumable = 3,
	Container = 4,
	CraftingMaterial = 5,
	Gathering = 6,
	Gizmo = 7,
	Minipet = 11,
	Tool = 13,
	Trait = 14,
	Trinket = 15,
	Trophy = 16,
	UpgradeComponent = 17,
	Weapon = 18
};

static std::map<ItemType, int> ItemTypeIndex = {
	{ None,0 },{ Armor,1 },{ Back,2 },{ Bag,3 },{ Consumable,4 },{ Container,5 },{ CraftingMaterial,6 },{ Gathering,7 },{ Gizmo,8 },{ Minipet,9 },{ Tool,10 },{ Trait,11 },
	{ Trinket,12 },{ Trophy,13 },{ UpgradeComponent,14 },{ Weapon,15 } };
static std::map<int, ItemType> ItemTypeEnum = { { 0,None },{ 1,Armor },{ 2,Back },{ 3,Bag },{ 4,Consumable },{ 5,Container },{ 6,CraftingMaterial },{ 7,Gathering },{ 8,Gizmo },
{ 9,Minipet },{ 10,Tool },{ 11,Trait },{ 12,Trinket },{ 13,Trophy },{ 14,UpgradeComponent },{ 15,Weapon } };
static std::map<ItemType, ExtendedItemType*> ItemTypeSubTypes = { {UpgradeComponent,new UpgradeComponentType()} };
static const char * ItemTypes[] = { "None", "Armor", "Back", "Bag", "Consumable", "Container", "CraftingMaterial", "Gathering","Gizmo","Minipet"
,"Tool" ,"Trait" ,"Trinket" ,"Trophy" ,"UpgradeComponent" ,"Weapon" };
#endif