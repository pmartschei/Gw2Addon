#include "TypeItemFilter.h"


TypeItemFilter::TypeItemFilter()
{
	value = ItemType::None;
	name = GetName();
}

std::string TypeItemFilter::GetName() {
	return "Type Filter (Type: " + std::string(ItemTypes[ItemTypeIndex[value]]) + ")";
}

void TypeItemFilter::RenderInput(ItemType & value)
{
	int v = ItemTypeIndex[value];
	gotUpdated |= ImGui::Combo(UNIQUE_NO_DELIMITER("##type", id), &v, &ItemTypes[0], (int)ItemTypeIndex.size());
	value = (ItemType)ItemTypeEnum[v];
}

bool TypeItemFilter::IsFiltered(ItemStackData data)
{
	return data.itemData.itemtype == value;
}

char * TypeItemFilter::GetSerializeName()
{
	return "TypeItemFilter";
}

IFilter * TypeItemFilter::CreateNew()
{
	return new TypeItemFilter();
}
void TypeItemFilter::SerializeContent(tinyxml2::XMLPrinter & printer)
{
	printer.PushAttribute("value", value);
}

void TypeItemFilter::DeserializeContent(tinyxml2::XMLElement * element)
{
	value = (ItemType)element->IntAttribute("value", 0);
	if (ItemTypeIndex.find(value) == ItemTypeIndex.end()) {
		value = ItemType::None;
	}
	name = GetName();
}