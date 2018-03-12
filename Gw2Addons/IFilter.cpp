#include "IFilter.h"
#include <iterator>
#include <algorithm>
#include "AddonColors.h"

int IFilter::ID = 0;

void IFilter::CustomMenu()
{
}

IFilter::IFilter() : id(ID++) {

}

std::set<ItemStackData> IFilter::Filter(std::set<ItemStackData> collection)
{
	std::set<ItemStackData>::iterator iter;

	std::set<ItemStackData> filteredSet;

	if (isActive) {
		for (iter = collection.begin(); iter != collection.end(); ++iter) {
			ItemStackData data = *iter;
			if (IsFiltered(data)) {
				filteredSet.insert(data);
			}
		}
		filteredSet = InvertSet(collection, filteredSet);
	}
	filteredItems = (int)filteredSet.size();
	return filteredSet;
}

bool IFilter::IsFiltered(ItemStackData data)
{
	return false;
}

void IFilter::Render()
{
	tabSpace = ImGui::GetWindowContentRegionWidth() - ImGui::GetContentRegionAvailWidth() + 140.0f;

	if (!isActive) {
		ImGui::PushStyleColor(ImGuiCol_Header, Addon::Colors[AddonColor_DisabledHeader]);
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, Addon::Colors[AddonColor_DisabledHeaderActive]);
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, Addon::Colors[AddonColor_DisabledHeaderHovered]);
	}
	ImGui::SetNextTreeNodeOpen(isOpened);
	isOpened = ImGui::CollapsingHeader(UNIQUE(name, id), ImGuiTreeNodeFlags_DefaultOpen);

	if (!isActive) {
		ImGui::PopStyleColor(3);
	}
	if (ImGui::BeginPopupContextItem(UNIQUE("HeaderPopup", id), 1)) {
		CustomMenu();
		if (isActive) {
			if (ImGui::Selectable("Deactivate")) {
				SetActive(!isActive);
			}
		}
		else {
			if (ImGui::Selectable("Activate")) {
				SetActive(!isActive);
			}
		}
		if (ImGui::Selectable("Expand all")) {
			SetOpen(true);
		}
		if (ImGui::Selectable("Collapse all")) {
			SetOpen(false);
		}
		ImGui::Separator();
		ImGui::PushStyleColor(ImGuiCol_Text, Addon::Colors[AddonColor_NegativeText]);
		if (ImGui::Selectable("Delete Filter")) {
			markedForDelete = true;
		}
		ImGui::PopStyleColor();
		ImGui::EndPopup();
	}

	float textSize = ImGui::CalcTextSize("Items filtered : 125").x;
	char* text = "Items filtered: %3u";
	float headerSize = ImGui::CalcTextSize(name.c_str()).x;
	float x = ImGui::GetWindowContentRegionWidth() - ImGui::GetContentRegionAvailWidth();
	if ((ImGui::GetContentRegionMax().x - headerSize - 30.0f - x) > textSize) {
		ImGui::SameLine(ImGui::GetContentRegionMax().x - textSize);
		ImGui::PushStyleColor(ImGuiCol_Text, Addon::Colors[AddonColor_PositiveText]);
		ImGui::Text(text, filteredItems);
		ImGui::PopStyleColor();
	}

	if (isOpened) {
		ImGui::Indent(ImGui::GetStyle().IndentSpacing);

		bool invert = (flags & FilterFlags::Not);
		ImGui::Text("Inverted : ");
		ImGui::SameLine(tabSpace);
		if (ImGui::Checkbox(UNIQUE_NO_DELIMITER("##invertFilter", id), &invert)) {
			gotUpdated = true;
			flags = (FilterFlags)(flags ^ FilterFlags::Not);
		}

		RenderContent();

		ImGui::Unindent(ImGui::GetStyle().IndentSpacing);
	}
}

void IFilter::SetOpen(bool open) {
	isOpened = open;
}
void IFilter::SetActive(bool active)
{
	this->isActive = active;
	gotUpdated = true;
}

bool IFilter::IsActive()
{
	return isActive;
}

bool IFilter::IsMarkedForDeletion()
{
	return markedForDelete;
}

bool IFilter::Updated()
{
	return gotUpdated;
}

void IFilter::ResetUpdateState()
{
	gotUpdated = false;
}

void IFilter::SetName(std::string name)
{
	this->name = name;
}

void IFilter::Serialize(tinyxml2::XMLPrinter & printer)
{
	printer.OpenElement("Filter");
	if (!nameCalculated)
		printer.PushAttribute("name", name.c_str());
	printer.PushAttribute("isActive", isActive);
	printer.PushAttribute("type", GetSerializeName());
	printer.PushAttribute("flags", flags);
	SerializeContent(printer);
	printer.CloseElement();
}
void IFilter::SerializeContent(tinyxml2::XMLPrinter & printer)
{
}


void IFilter::Deserialize(tinyxml2::XMLElement * element)
{
	const char* c = element->Attribute("name");
	isActive = element->BoolAttribute("isActive", true);
	flags = (FilterFlags)element->IntAttribute("flags", FilterFlags::And);
	if (c)
		name = std::string(c);
	DeserializeContent(element);
}

void IFilter::DeserializeContent(tinyxml2::XMLElement * element)
{
}

std::set<ItemStackData> IFilter::InvertSet(std::set<ItemStackData> fullData,std::set<ItemStackData> selectedData)
{
	std::set<ItemStackData> invers;
	if (flags & FilterFlags::Not) {
		std::set_difference(fullData.begin(), fullData.end(), selectedData.begin(), selectedData.end(), std::inserter(invers, invers.end()));
		return invers;
	}
	return selectedData;
}

int IFilter::GetFilteredCount() {
	return filteredItems;
}
