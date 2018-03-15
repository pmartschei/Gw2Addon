#include "LevelRangeItemFilter.h"

LevelRangeItemFilter::LevelRangeItemFilter()
{
	minValue = 0;
	maxValue = 0;
	name = GetName();
}

std::string LevelRangeItemFilter::GetName()
{
	int displayMin = minValue;
	int displayMax = maxValue;
	if (maxValue < minValue) {
		displayMin = maxValue;
		displayMax = minValue;
	}
	return "Level Range Filter (" + std::to_string(displayMin) + (maxValue == minValue ? ")" : " to " + std::to_string(displayMax) + ")");
}

void LevelRangeItemFilter::RenderInput(int & value, std::string s)
{
	gotUpdated |= ImGui::InputInt(UNIQUE_NO_DELIMITER("##level" + s, id), &value, 1, 1000);
	if (ImGui::IsItemHovered() && !ImGui::IsAnyItemActive()) {
		value += ImGui::GetIO().MouseWheel;
	}
	value = CLAMP(value, 0, 80);
}
bool LevelRangeItemFilter::IsFiltered(FilterData data)
{
	if (minValue < maxValue) {
		return data->itemData->level >= (uint32_t)minValue && data->itemData->level <= (uint32_t)maxValue;
	}
	else {
		return data->itemData->level >= (uint32_t)maxValue && data->itemData->level <= (uint32_t)minValue;
	}
}

char * LevelRangeItemFilter::GetSerializeName()
{
	return "LevelRangeItemFilter";
}

IFilter * LevelRangeItemFilter::CreateNew()
{
	return new LevelRangeItemFilter();
}

void LevelRangeItemFilter::SerializeContent(tinyxml2::XMLPrinter & printer)
{
	printer.PushAttribute("minValue", minValue);
	printer.PushAttribute("maxValue", maxValue);
}

void LevelRangeItemFilter::DeserializeContent(tinyxml2::XMLElement * element)
{
	minValue = CLAMP(element->IntAttribute("minValue", 0), 0, 80);
	maxValue = CLAMP(element->IntAttribute("maxValue", 0), 0, 80);
	name = GetName();
}