#include "LevelItemFilter.h"

LevelItemFilter::LevelItemFilter() {
	value = 0;
	name = GetName();
}


std::string LevelItemFilter::GetName()
{
	return "Level Filter (Level: " + std::to_string(value) + ")";
}

void LevelItemFilter::RenderInput(int & value)
{
	gotUpdated |= ImGui::InputInt(UNIQUE_NO_DELIMITER("##level", id), &value);
	value = CLAMP(value, 0, 80);
}

bool LevelItemFilter::IsFiltered(ItemStackData data)
{
	return data.itemData->level == value;
}

char * LevelItemFilter::GetSerializeName()
{
	return "LevelItemFilter";
}

IFilter * LevelItemFilter::CreateNew()
{
	return new LevelItemFilter();
}

void LevelItemFilter::SerializeContent(tinyxml2::XMLPrinter & printer)
{
	printer.PushAttribute("value", value);
}

void LevelItemFilter::DeserializeContent(tinyxml2::XMLElement * element)
{
	value = CLAMP(element->IntAttribute("value", 0), 0, 80);
	name = GetName();
}
