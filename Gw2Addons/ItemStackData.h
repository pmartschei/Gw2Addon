#ifndef ITEM_STACK_DATA_H
#define ITEM_STACK_DATA_H

#include "hacklib\ForeignClass.h"
#include "ItemData.h"

class ItemStackData {
public:
	ItemStackData();
	~ItemStackData();
	uint32_t slot = 0;
	uint32_t count = 0;
	bool accountBound;
	bool tradingpostSellable;
	hl::ForeignClass pItem;
	ItemData* itemData;
};
inline bool operator==(const ItemStackData& lhs, const ItemStackData& rhs)
{
	return lhs.pItem == rhs.pItem;
}
inline bool operator<(const ItemStackData& lhs, const ItemStackData& rhs)
{
	return lhs.slot < rhs.slot;
}
#endif