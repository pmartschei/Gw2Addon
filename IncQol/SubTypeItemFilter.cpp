#include "SubTypeItemFilter.h"


SubTypeItemFilter::SubTypeItemFilter(ItemType base) : baseType(base)
{
	value = 0;
	name = GetName();
	supportDrag = false;
}

std::string SubTypeItemFilter::GetName() {
	ExtendedItemType* type = ItemTypeSubTypes[baseType];
	if (type) {
		return "Sub Type Filter (" + std::string(type->GetSubTypes()[value]) + ")";
	}
	return "Sub Type Filter invalid type";
}

void SubTypeItemFilter::RenderInput(int & value)
{
	ExtendedItemType* type = ItemTypeSubTypes[baseType];
	const char** items = type->GetSubTypes();
	gotUpdated |= ImGui::Combo(UNIQUE_NO_DELIMITER("##type", id), &value, &items[0], type->GetSubSize());
}

bool SubTypeItemFilter::IsFiltered(FilterData data)
{
	if (data->itemData->extendedItemType)
		return data->itemData->extendedItemType->IsFiltered(&value, ExtendedItemType::FilterBy::Type);
	return false;
}

char * SubTypeItemFilter::GetSerializeName()
{
	return "SubTypeItemFilter";
}

IFilter * SubTypeItemFilter::CreateNew()
{
	return new SubTypeItemFilter(baseType);
}
void SubTypeItemFilter::SerializeContent(tinyxml2::XMLPrinter & printer)
{
	printer.PushAttribute("value", value);
	printer.PushAttribute("baseType", baseType);
}

bool SubTypeItemFilter::DeserializeContent(tinyxml2::XMLElement * element)
{
	tinyxml2::XMLElement* e = element->Parent()->ToElement();
	int parentValue = e->IntAttribute("value", ItemType::None);
	baseType = (ItemType)element->IntAttribute("baseType", ItemType::None);
	if (baseType != parentValue) return false;//will remove the filter
	if (ItemTypeSubTypes.find(baseType) == ItemTypeSubTypes.end()) {
		return false;//will remove the filter
	}
	value = element->IntAttribute("value", 0);
	value = CLAMP(value, 0, ItemTypeSubTypes[baseType]->GetSubSize()-1);
	name = GetName();
	return true;
}