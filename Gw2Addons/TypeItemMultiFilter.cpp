#include "TypeItemMultiFilter.h"


TypeItemMultiFilter::TypeItemMultiFilter()
{
	value = std::vector<bool>();
	for (int i = 0; i < ItemTypeIndex.size(); i++) {
		value.push_back(false);
	}
	typeCount = 0;
	name = GetName();
}

std::string TypeItemMultiFilter::GetName() {
	return "Type Multi Filter (" + std::to_string(typeCount)+ (typeCount == 1? " Type" :" Types") + ")" ;
}

void TypeItemMultiFilter::RenderInput(std::vector<bool> & value)
{
	typeCount = 0;
	ImGui::ListBoxHeader("##types");
	for (int i = 0; i < value.size(); i++) {
		bool b = value[i];
		gotUpdated |= ImGui::Selectable(UNIQUE(ItemTypes[i],id),&b);
		value[i] = b;
		typeCount += b ? 1 : 0;
	}
	ImGui::ListBoxFooter();
}

bool TypeItemMultiFilter::IsFiltered(FilterData data)
{
	for (int i = 0; i < value.size(); i++) {
		if (value[i] && data->itemData->itemtype == ItemTypeEnum[i]) return true;
	}
	return false;
}

char * TypeItemMultiFilter::GetSerializeName()
{
	return "TypeItemMultiFilter";
}

IFilter * TypeItemMultiFilter::CreateNew()
{
	return new TypeItemMultiFilter();
}
void TypeItemMultiFilter::SerializeContent(tinyxml2::XMLPrinter & printer)
{
	const char* values = "12,12,12";
	printer.PushAttribute("values", values);
}

void TypeItemMultiFilter::DeserializeContent(tinyxml2::XMLElement * element)
{
	const char* values = element->Attribute("values", 0);
	/*if (ItemTypeIndex.find(value) == ItemTypeIndex.end()) {
		value = ItemType::None;
	}*/
	name = GetName();
}