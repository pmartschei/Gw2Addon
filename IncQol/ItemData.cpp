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
