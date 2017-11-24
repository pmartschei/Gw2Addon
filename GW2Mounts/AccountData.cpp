#include "AccountData.h"

void AccountData::ReadMemory(Process * process)
{
	Reset();
	_ReadMemory(process);
}
void AccountData::Reset()
{
	_items.clear();
}

std::vector<Item> AccountData::GetItems()
{
	return _items;
}
