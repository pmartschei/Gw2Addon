#include "RarityRangeItemFilter.h"

RarityRangeItemFilter::RarityRangeItemFilter()
{
	minValue = ItemRarity::Junk;
	maxValue = ItemRarity::Junk;
	name = GetName();
}

std::string RarityRangeItemFilter::GetName()
{
	int minRarity = minValue;
	int maxRarity = maxValue;
	if (maxValue < minValue) {
		minRarity = maxValue;
		maxRarity = minValue;
	}
	std::string displayMin = std::string(ItemRarities[minRarity]);
	std::string displayMax = std::string(ItemRarities[maxRarity]);
	return "Rarity Filter (Rarity: " + displayMin + (maxValue == minValue ? ")" : " to " + displayMax + ")");
}

void RarityRangeItemFilter::RenderInput(ItemRarity & value, std::string s)
{
	int v = value;
	gotUpdated |= ImGui::Combo(UNIQUE_NO_DELIMITER("##rarity"+s, id), &v, &ItemRarities[0], ItemRarity::RarityCount);
	value = (ItemRarity)(v);
}

bool RarityRangeItemFilter::IsFiltered(FilterData data)
{
	if (minValue < maxValue) {
		return data->itemData->rarity >= minValue && data->itemData->rarity <= maxValue;
	}
	else {
		return data->itemData->rarity >= maxValue && data->itemData->rarity <= minValue;
	}
}

char * RarityRangeItemFilter::GetSerializeName()
{
	return "RarityRangeItemFilter";
}

IFilter * RarityRangeItemFilter::CreateNew()
{
	return new RarityRangeItemFilter();
}
void RarityRangeItemFilter::SerializeContent(tinyxml2::XMLPrinter & printer)
{
	printer.PushAttribute("minValue", minValue);
	printer.PushAttribute("maxValue", maxValue);
}

void RarityRangeItemFilter::DeserializeContent(tinyxml2::XMLElement * element)
{
	minValue = (ItemRarity)CLAMP(element->IntAttribute("minValue", 0), 0, ItemRarity::RarityCount - 1);
	maxValue = (ItemRarity)CLAMP(element->IntAttribute("maxValue", 0), 0, ItemRarity::RarityCount - 1);
	name = GetName();
}