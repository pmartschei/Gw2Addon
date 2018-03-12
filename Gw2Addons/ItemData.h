#ifndef ITEM_DATA_H
#define ITEM_DATA_H
#include "hacklib\ForeignClass.h"
#include "ItemType.h"
#include "ItemRarity.h"

class ItemData {
public:
	
	hl::ForeignClass pItemData = nullptr;
	hl::ForeignClass pExtendedType = nullptr;
	uint32_t id = 0;
	bool sellable;
	uint32_t level;
	ItemRarity rarity;
	ItemType itemtype;
	std::string name;
	float buyTradingPost;
	float sellTradingPost;
	uint32_t vendorValue;
};
inline bool operator==(const ItemData& lhs, const ItemData& rhs)
{
	return lhs.pItemData == rhs.pItemData;
}


#endif