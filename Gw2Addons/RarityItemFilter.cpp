#include "RarityItemFilter.h"


RarityItemFilter::RarityItemFilter()
{
	value = ItemRarity::Junk;
	name = GetName();
}

std::string RarityItemFilter::GetName() {
	return "Rarity Filter (Rarity: " + std::string(ItemRarities[value])+")";
}

void RarityItemFilter::RenderInput(ItemRarity & value)
{
	int v = value;
	gotUpdated |= ImGui::Combo(UNIQUE_NO_DELIMITER("##rarity", id), &v, &ItemRarities[0], ItemRarity::RarityCount);
	value = (ItemRarity)(v);
}

bool RarityItemFilter::IsFiltered(ItemStackData data)
{
	return data.itemData->rarity == value;
}

char * RarityItemFilter::GetSerializeName()
{
	return "RarityItemFilter";
}

IFilter * RarityItemFilter::CreateNew()
{
	return new RarityItemFilter();
}
void RarityItemFilter::SerializeContent(tinyxml2::XMLPrinter & printer)
{
	printer.PushAttribute("value", value);
}

void RarityItemFilter::DeserializeContent(tinyxml2::XMLElement * element)
{
	value = (ItemRarity)CLAMP(element->IntAttribute("value", 0),0,ItemRarity::RarityCount - 1);
	name = GetName();
}