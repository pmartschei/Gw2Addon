#include "ItemData.h"
#include "PluginBase.h"

std::map<uint, ItemData*> ItemData::datas = std::map<uint, ItemData*>();

ItemData::ItemData() : pItemData(nullptr), pExtendedType(nullptr)
{
}

ItemData::~ItemData()
{
}

void ItemData::AddData(ItemData * data)
{
	datas[data->id] = data;
}

ItemData * ItemData::GetData(uint id)
{
	if (datas.find(id) != datas.end()) {
		return datas[id];
	}
	return nullptr;
}

bool ItemData::IsOldTradingPostData()
{
	return (GetCurrentTime() - lastTradingPostUpdate > (uint)(PluginBase::tpUpdateInterval * 60000));
}

ItemFlags ItemData::GetFlags()
{
	return flags;
}

void ItemData::SetFlags(ItemFlags flags)
{
	pItemData.set<ItemFlags>(0x38, flags);
	this->flags = flags;
}

bool ItemData::HasFlags(ItemFlags flags)
{
	return (this->flags & flags);
}


