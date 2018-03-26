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
	ImGui::ListBoxHeader(UNIQUE_NO_DELIMITER("##types",id));
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
	std::string values = "";
	bool first = true;
	for (int i = 0; i < value.size(); i++) {
		if (value[i]) {
			if (!first)
				values += ",";
			values += std::to_string(ItemTypeEnum[i]);
			first = false;
		}
	}
	printer.PushAttribute("values", values.c_str());
}

void TypeItemMultiFilter::DeserializeContent(tinyxml2::XMLElement * element)
{
	std::string values = element->Attribute("values", 0);
	std::string delimiter = ",";
	size_t pos = 0;
	std::string token;
	while ((pos = values.find(delimiter)) != std::string::npos) {
		try{
			token = values.substr(0, pos);
			int index = std::stoi(token);
			value[ItemTypeIndex[(ItemType)index]] = true;
		}
		catch (...) {
			//ignore string value
		}
		values.erase(0, pos + delimiter.length());
	}
	if (!values.empty()){
		try {
			int index = std::stoi(values);
			value[ItemTypeIndex[(ItemType)index]] = true;
		}
		catch (...) {
			//ignore string value
		}
	}
	typeCount = 0;
	for (int i = 0; i < value.size(); i++) {
		if (value[i]) typeCount++;
	}
	name = GetName();
}