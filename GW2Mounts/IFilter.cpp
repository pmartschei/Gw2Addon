#include "IFilter.h"

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
	if (!isActive) {
		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(1, 110/255.f, 50/255.f, 1));
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(1, 170 / 255.f, 90 / 255.f, 1));
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1, 140 / 255.f, 70 / 255.f, 1));
	}
	ImGui::SetNextTreeNodeOpen(isOpened);
	isOpened = ImGui::CollapsingHeader(UNIQUE(name, id), ImGuiTreeNodeFlags_DefaultOpen);

	if (!isActive) {
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
	}
	if (ImGui::BeginPopupContextItem(UNIQUE("HeaderPopup", id), 1)) {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
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
		ImGui::PopStyleColor();
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
		if (ImGui::Selectable("Delete Filter")) {
			markedForDelete = true;
		}
		ImGui::PopStyleColor();
		ImGui::EndPopup();
	}

	float textSize = ImGui::CalcTextSize("Items filtered : 125").x;
	char* text = "Items filtered: %3u";
	if ((ImGui::GetContentRegionMax().x - ImGui::CalcItemWidth()) > textSize) {
		/*textSize = ImGui::CalcTextSize(" 125 ").x;
		text = " %3u ";*/
		ImGui::SameLine(ImGui::GetContentRegionMax().x - textSize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
		ImGui::Text(text, filteredItems);
		ImGui::PopStyleColor();
	}
	

	if (isOpened) {
		ImGui::Indent(ImGui::GetStyle().IndentSpacing);

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
	if (c)
		name = std::string(c);
	DeserializeContent(element);
}

void IFilter::DeserializeContent(tinyxml2::XMLElement * element)
{
}

int IFilter::GetFilteredCount() {
	return filteredItems;
}
