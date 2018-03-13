#ifndef ITEM_DATA_H
#define ITEM_DATA_H
#include "hacklib\ForeignClass.h"
#include "ItemType.h"
#include "ItemRarity.h"
#include "main.h"
#include <map>

class ItemData {
private:
	static std::map<uint, ItemData*> datas;
public:
	ItemData();
	~ItemData();
	static void AddData(ItemData* data);
	static ItemData* GetData(uint id);
	hl::ForeignClass pItemData;
	hl::ForeignClass pExtendedType;
	uint id = 0;
	bool sellable;
	uint level;
	ItemRarity rarity;
	ItemType itemtype;
	std::string name;
	bool validTradingPostData = false;
	float lastTradingPostUpdate = 0;
	float buyTradingPost;
	float sellTradingPost;
	uint vendorValue;
};
inline bool operator==(const ItemData& lhs, const ItemData& rhs)
{
	return lhs.pItemData == rhs.pItemData;
}


#endif