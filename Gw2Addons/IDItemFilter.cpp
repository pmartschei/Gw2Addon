#include "IDItemFilter.h"

IDItemFilter::IDItemFilter() {
	value = 0;
	name = GetName();
}


std::string IDItemFilter::GetName()
{
	return "ID Filter (" + std::to_string(value) + ")";
}

void IDItemFilter::RenderInput(int & value)
{
	gotUpdated |= ImGui::InputInt(UNIQUE_NO_DELIMITER("##itemid", id), &value);
	if (ImGui::IsItemHovered() && !ImGui::IsAnyItemActive()) {
		value += ImGui::GetIO().MouseWheel;
	}
	value = CLAMP(value, 0, 1000000);
}

char * IDItemFilter::GetSerializeName()
{
	return "IDItemFilter";
}

bool IDItemFilter::IsFiltered(FilterData data)
{
	return data->itemData->id == value;
}

IFilter * IDItemFilter::CreateNew()
{
	return new IDItemFilter();
}

void IDItemFilter::SerializeContent(tinyxml2::XMLPrinter & printer)
{
	printer.PushAttribute("value", value);
}

void IDItemFilter::DeserializeContent(tinyxml2::XMLElement * element)
{
	value = CLAMP(element->IntAttribute("value", 0), 0, 1000000); 
	name = GetName();
}
