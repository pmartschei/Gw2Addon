#include "SellableItemFilter.h"

SellableItemFilter::SellableItemFilter() {
	value = true;
	name = GetName();
}


std::string SellableItemFilter::GetName()
{
	return "Sellable Filter ( " + std::to_string(value) + ")";
}

void SellableItemFilter::RenderInput(bool& value)
{
	//not needed
}

bool SellableItemFilter::IsFiltered(ItemStackData data)
{
	return data.itemData.sellable == value;
}

char * SellableItemFilter::GetSerializeName()
{
	return "SellableItemFilter";
}

IFilter * SellableItemFilter::CreateNew()
{
	return new SellableItemFilter();
}
