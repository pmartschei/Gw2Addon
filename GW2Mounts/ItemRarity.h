#ifndef ITEM_RARITY_H
#define ITEM_RARITY_H
#include <map>

enum ItemRarity : int {
	Junk = 0,
	Basic,
	Fine,
	Masterwork,
	Rare,
	Exotic,
	Ascended,
	Legendary = 7,
	RarityCount = 8
}; 
static const char * ItemRarities[] = { "Junk", "Basic", "Fine", "Masterwork", "Rare", "Exotic", "Ascended", "Legendary","Undefined" };
#endif