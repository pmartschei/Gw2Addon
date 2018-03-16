#include "IFilter.h"
#include <iterator>
#include <algorithm>
#include "AddonColors.h"
#include "FilterPlugin.h"
#include "Utility.h"

int IFilter::ID = 0;

void IFilter::CustomMenu()
{
}

void IFilter::SaveFilteredItemDatas(std::set<FilterData> filteredSet)
{
	filteredItemDatas.clear();
	for (auto it = filteredSet.begin(); it != filteredSet.end(); ++it) {
		FilterData data = (*it);
		filteredItemDatas.push_back(data->itemData);
	}
}

IFilter::IFilter() : id(ID++) {

}

std::set<FilterData> IFilter::Filter(std::set<FilterData> collection)
{
	std::set<FilterData>::iterator iter;

	std::set<FilterData> filteredSet;

	if (isActive) {
		for (iter = collection.begin(); iter != collection.end(); ++iter) {
			FilterData data = *iter;
			if (!data->itemData) continue;
			if (IsFiltered(data)) {
				filteredSet.insert(data);
			}
		}
		filteredSet = InvertSet(collection, filteredSet);
	}
	SaveFilteredItemDatas(filteredSet);
	return filteredSet;
}

bool IFilter::IsFiltered(FilterData data)
{
	return false;
}

void IFilter::DragDropSource() {
	if (ImGui::BeginDragDropSource()) {
		IFilter* filter = this;
		uintptr_t f = (uintptr_t)filter;
		ImGui::SetDragDropPayload(DRAG_DROP_PAYLOAD_TYPE_FILTER, &f, sizeof(f), ImGuiCond_Once);
		ImGui::Text("Filter");
		ImGui::EndDragDropSource();
	}
}

bool IFilter::HasParent(IFilter * filter)
{
	if (!parent) return false;
	return parent == filter || parent->HasParent(filter);
}

void IFilter::Render()
{
	tabSpace = ImGui::GetWindowContentRegionWidth() - ImGui::GetContentRegionAvailWidth() + TEXTSPACE;

	if (!isActive) {
		ImGui::PushStyleColor(ImGuiCol_Header, Addon::Colors[AddonColor_DisabledHeader]);
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, Addon::Colors[AddonColor_DisabledHeaderActive]);
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, Addon::Colors[AddonColor_DisabledHeaderHovered]);
	}
	ImGui::SetNextTreeNodeOpen(isOpened);
	isOpened = ImGui::CollapsingHeader(UNIQUE(name, id), ImGuiTreeNodeFlags_DefaultOpen);
	DragDropSource();
	DragDropTarget();
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

		int size = filteredItemDatas.size();
		if (size > 0) {
			filteredItemDatas.sort(ItemData::sortName);
			filteredItemDatas.unique();
			size = filteredItemDatas.size();
		}
		ImGui::PushStyleColor(ImGuiCol_Text, Addon::Colors[AddonColor_PositiveText]);
		ImGui::Text(text, size);
		ImGui::PopStyleColor();
		if (ImGui::IsItemHovered() && size > 0) {
			ImGui::BeginTooltip();
			int limit = min(size, FilterPlugin::maxTooltipSize);
			if (size > limit) {
				ImGui::Text("Use your mouse wheel to scroll");
				ImGui::Text("");
			}
			ImGuiIO io = ImGui::GetIO();
			filteredItemDatasStartY -= io.MouseWheel;
			if (filteredItemDatasStartY + FilterPlugin::maxTooltipSize >= size) filteredItemDatasStartY = size - FilterPlugin::maxTooltipSize;
			if (filteredItemDatasStartY < 0) filteredItemDatasStartY = 0;
			ItemData** arr = new ItemData*[size];
			std::copy(filteredItemDatas.begin(), filteredItemDatas.end(), arr);
			for (int i = filteredItemDatasStartY; i < filteredItemDatasStartY + limit; i++) {
				switch (arr[i]->rarity) {
				case ItemRarity::Junk:
					ImGui::PushStyleColor(ImGuiCol_Text, Addon::Colors[AddonColor_RarityJunk]);
					break;
				case ItemRarity::Basic:
					ImGui::PushStyleColor(ImGuiCol_Text, Addon::Colors[AddonColor_RarityBasic]);
					break;
				case ItemRarity::Fine:
					ImGui::PushStyleColor(ImGuiCol_Text, Addon::Colors[AddonColor_RarityFine]);
					break;
				case ItemRarity::Masterwork:
					ImGui::PushStyleColor(ImGuiCol_Text, Addon::Colors[AddonColor_RarityMasterwork]);
					break;
				case ItemRarity::Rare:
					ImGui::PushStyleColor(ImGuiCol_Text, Addon::Colors[AddonColor_RarityRare]);
					break;
				case ItemRarity::Exotic:
					ImGui::PushStyleColor(ImGuiCol_Text, Addon::Colors[AddonColor_RarityExotic]);
					break;
				case ItemRarity::Ascended:
					ImGui::PushStyleColor(ImGuiCol_Text, Addon::Colors[AddonColor_RarityAscended]);
					break;
				case ItemRarity::Legendary:
					ImGui::PushStyleColor(ImGuiCol_Text, Addon::Colors[AddonColor_RarityLegendary]);
					break;
				default:
					ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_Text]);
				}
				ImGui::Text(arr[i]->name.c_str());
				ImGui::PopStyleColor();
			}
			delete[] arr;
			ImGui::EndTooltip();
		}
		else {
			filteredItemDatasStartY = 0;
		}
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

std::set<ItemStackData*> IFilter::InvertSet(std::set<ItemStackData*> fullData,std::set<ItemStackData*> selectedData)
{
	std::set<ItemStackData*> invers;
	if (flags & FilterFlags::Not) {
		std::set_difference(fullData.begin(), fullData.end(), selectedData.begin(), selectedData.end(), std::inserter(invers, invers.end()));
		return invers;
	}
	return selectedData;
}
