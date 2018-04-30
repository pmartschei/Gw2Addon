#include "TypeItemFilter.h"
#include "SubTypeItemFilter.h"


TypeItemFilter::TypeItemFilter()
{
	value = ItemType::None;
	name = GetName();
}

std::string TypeItemFilter::GetName() {
	return "Type Filter (" + std::string(ItemTypes[ItemTypeIndex[value]]) + (filterIndex==0?"": " -> "+std::to_string(activeChildFilters) + " Sub Types")+")";
}

void TypeItemFilter::RenderInput(ItemType & value)
{
	int v = ItemTypeIndex[value];
	gotUpdated |= ImGui::Combo(UNIQUE_NO_DELIMITER("##type", id), &v, &ItemTypes[0], (int)ItemTypeIndex.size());
	value = (ItemType)ItemTypeEnum[v];
}

void TypeItemFilter::CustomMenu() {
	if (ItemTypeSubTypes.find(value) != ItemTypeSubTypes.end()) {
		if (ImGui::MenuItem("Add Sub Filter")) {
			AddFilter(new SubTypeItemFilter(value));
		}
	}
}

void TypeItemFilter::AddFilter(IFilter * filter)
{
	GroupFilter::AddFilter(filter);
	name = GetName();
}

void TypeItemFilter::RemoveFilter(IFilter * filter)
{
	GroupFilter::RemoveFilter(filter);
	name = GetName();
}

bool TypeItemFilter::IsFiltered(FilterData data)
{
	return data->itemData->itemtype == value;
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
	GroupFilter::SerializeContent(printer);
}

bool TypeItemFilter::DeserializeContent(tinyxml2::XMLElement * element)
{
	value = (ItemType)element->IntAttribute("value", 0);
	if (ItemTypeIndex.find(value) == ItemTypeIndex.end()) {
		value = ItemType::None;
	}
	else {
		GroupFilter::DeserializeContent(element);
	}
	name = GetName();
	return true;
}