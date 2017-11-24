#include "IDRangeItemFilter.h"

IDRangeItemFilter::IDRangeItemFilter()
{
	minValue = 0;
	maxValue = 0;
	name = GetName();
}

std::string IDRangeItemFilter::GetName()
{
	int displayMin = minValue;
	int displayMax = maxValue;
	if (maxValue < minValue) {
		displayMin = maxValue;
		displayMax = minValue;
	}
	return "ID Range Filter (ID: " + std::to_string(displayMin) + (maxValue == minValue ? ")" : " to " + std::to_string(displayMax) + ")");
}

void IDRangeItemFilter::RenderInput(int & value, std::string s)
{
	gotUpdated |= ImGui::InputInt(UNIQUE_NO_DELIMITER("##itemid"+s, id), &value);
	value = CLAMP(value, 0, 1000000);
}

bool IDRangeItemFilter::IsFiltered(ItemStackData data)
{
	if (minValue < maxValue) {
		return data.itemData.id >= (uint32_t)minValue && data.itemData.id <= (uint32_t)maxValue;
	}
	else {
		return data.itemData.id >= (uint32_t)maxValue && data.itemData.id <= (uint32_t)minValue;
	}
}

char * IDRangeItemFilter::GetSerializeName()
{
	return "IDRangeItemFilter";
}

IFilter * IDRangeItemFilter::CreateNew()
{
	return new IDRangeItemFilter();
}

void IDRangeItemFilter::SerializeContent(tinyxml2::XMLPrinter & printer)
{
	printer.PushAttribute("minValue", minValue);
	printer.PushAttribute("maxValue", maxValue);
}

void IDRangeItemFilter::DeserializeContent(tinyxml2::XMLElement * element)
{
	minValue = CLAMP(element->IntAttribute("minValue", 0), 0, 1000000);
	maxValue = CLAMP(element->IntAttribute("maxValue", 0), 0, 1000000);
	name = GetName();
}