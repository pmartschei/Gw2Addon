#include "GroupFilter.h"
#include "IDItemFilter.h"
#include "IDRangeItemFilter.h"
#include "LevelItemFilter.h"
#include "LevelRangeItemFilter.h"
#include "RarityItemFilter.h"
#include "RarityRangeItemFilter.h"
#include "TypeItemFilter.h"
#include "ItemFilterFactory.h"
#include "TradingPostValueFilter.h"
#include "TypeItemMultiFilter.h"
#include "PluginBase.h"
#include <algorithm>
#include <iterator>
#include "Utility.h"

GroupFilter::GroupFilter() {
	flags = FilterFlags::And;
	name = "Unnamed Group Filter"; 
	nameCalculated = false;
}

GroupFilter::~GroupFilter() {

}
std::set<FilterData> GroupFilter::Filter(std::set<FilterData> collection)
{
	std::lock_guard<std::recursive_mutex> lock(mutex);
	std::set<FilterData> filteredSet;
	std::vector<IFilter*>::iterator iter;

	if (flags & FilterFlags::And) {
		filteredSet = collection;
	}
	int activeCount = 0;
	if (IsActive()) {
		for (int i = 0; i < filterIndex; i++) {
			if (subFilters[i]) {
				bool filterActive = subFilters[i]->IsActive();
				if (filterActive) {
					activeCount++;
				}
				else {
					continue;
				}
				std::set<FilterData> subFilteredSet = subFilters[i]->Filter(collection);
				if (flags & FilterFlags::And) {
					std::set<FilterData> intersection;
					std::set_intersection(filteredSet.begin(), filteredSet.end(), subFilteredSet.begin(), subFilteredSet.end(), std::inserter(intersection, intersection.end()));
					filteredSet = intersection;
				}
				else if (flags & FilterFlags::Or) {
					filteredSet.insert(subFilteredSet.begin(), subFilteredSet.end());
				}
			}
		}
	}

	if (activeCount == 0) {
		filteredSet = std::set<FilterData>();
		SaveFilteredItemDatas(filteredSet);
		return filteredSet;
	}
	filteredSet = InvertSet(collection, filteredSet);
	SaveFilteredItemDatas(filteredSet);
	return filteredSet;
}

bool GroupFilter::Updated()
{
	if (gotUpdated) return true;

	std::lock_guard<std::recursive_mutex> lock(mutex);
	for (int i = 0; i < filterIndex; i++) {
		if (subFilters[i]) {
			if (subFilters[i]->Updated()) return true;
		}
	}
	return false;
}

void GroupFilter::ResetUpdateState()
{
	IFilter::ResetUpdateState();
	std::lock_guard<std::recursive_mutex> lock(mutex);
	for (int i = 0; i < filterIndex; i++) {
		if (subFilters[i]) {
			subFilters[i]->ResetUpdateState();
		}
	}
}

char * GroupFilter::GetSerializeName()
{
	return "GroupFilter";
}

void GroupFilter::SetOpen(bool open)
{
	IFilter::SetOpen(open);
	std::lock_guard<std::recursive_mutex> lock(mutex);
	for (int i = 0; i < filterIndex; i++) {
		if (subFilters[i]) {
			subFilters[i]->SetOpen(open);
		}
	}
}

void GroupFilter::DragDropTarget()
{
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DRAG_DROP_PAYLOAD_TYPE_FILTER)) {
			uintptr_t* f = (uintptr_t*)payload->Data;
			IFilter* filter = (IFilter*)*f;
			if (filter && !this->HasParent(filter)) {
				if (filter->parent) {
					GroupFilter* groupFilter = dynamic_cast<GroupFilter*>(filter->parent);
					if (groupFilter) {
						groupFilter->RemoveFilter(filter);
					}
				}
				this->AddFilter(filter);
			}
		}
		ImGui::EndDragDropTarget();
	}
}

IFilter * GroupFilter::CreateNew()
{
	return new GroupFilter();
}

void GroupFilter::CustomMenu()
{
	if (ImGui::BeginMenu("Add Filter")) {
		if (ImGui::BeginMenu("Basic Filters")) {
			if (ImGui::MenuItem("Group Filter")) {
				AddFilter(new GroupFilter());
			}
			if (ImGui::MenuItem("ID Filter")) {
				AddFilter(new IDItemFilter());
			}
			if (ImGui::MenuItem("Level Filter")) {
				AddFilter(new LevelItemFilter());
			}
			if (ImGui::MenuItem("Rarity Filter")) {
				AddFilter(new RarityItemFilter());
			}
			if (ImGui::MenuItem("Type Filter")) {
				AddFilter(new TypeItemFilter());
			}
			if (ImGui::MenuItem("Type Multi Filter")) {
				AddFilter(new TypeItemMultiFilter());
			}
			if (ImGui::MenuItem("Trading Post Filter")) {
				AddFilter(new TradingPostValueFilter());
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Range Filters")) {

			if (ImGui::MenuItem("ID Filter")) {
				AddFilter(new IDRangeItemFilter());
			}
			if (ImGui::MenuItem("Level Filter")) {
				AddFilter(new LevelRangeItemFilter());
			}
			if (ImGui::MenuItem("Rarity Filter")) {
				AddFilter(new RarityRangeItemFilter());
			}

			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}
}

void GroupFilter::RemoveAndDeleteAll() {

	std::lock_guard<std::recursive_mutex> lock(mutex);
	for (int i = 0; i < filterIndex; i++) {
		if (subFilters[i]) {
			IFilter* filter = subFilters[i];
			GroupFilter* groupFilter = dynamic_cast<GroupFilter*>(filter);
			if (groupFilter) groupFilter->RemoveAndDeleteAll();
			RemoveFilter(filter);
			i--;
			delete filter;
		}
	}
}

void GroupFilter::CheckForDeletion()
{
	std::lock_guard<std::recursive_mutex> lock(mutex);
	for (int i = 0; i < filterIndex; i++) {
		if (subFilters[i]) {
			IFilter* filter = subFilters[i];
			GroupFilter* groupFilter = dynamic_cast<GroupFilter*>(filter);
			if (filter->IsMarkedForDeletion()) {
				if (groupFilter) {
					groupFilter->RemoveAndDeleteAll();
				}
				this->RemoveFilter(filter);
				delete filter;
			}
			else {
				if (groupFilter) {
					groupFilter->CheckForDeletion();
				}
			}
		}
	}
}

void GroupFilter::RenderContent() {
	char *cpy = new char[64];
	strcpy_s(cpy,64, name.c_str());

	ImGui::Text("Name : ");
	ImGui::SameLine(tabSpace);
	ImGui::PushItemWidth(-1);
	gotUpdated |= ImGui::InputText(UNIQUE_NO_DELIMITER("##nameinput", id), cpy, 64);
	ImGui::PopItemWidth();

	name = std::string(cpy);
	ImGui::Text("Operation : ");
	ImVec4 colorAnd = flags & FilterFlags::And ? Addon::Colors[AddonColor_PositiveText] : ImGui::GetStyle().Colors[ImGuiCol_Text];
	ImVec4 colorOr = flags & FilterFlags::Or ? Addon::Colors[AddonColor_PositiveText] : ImGui::GetStyle().Colors[ImGuiCol_Text];
	RenderTooltip(std::vector<TooltipColor>{TooltipColor("And", colorAnd),
		TooltipColor("Each filter is connected with \"AND\", only items are filtered, which were also filtered in all sub filters.", colorAnd),
		TooltipColor("\n"),
		TooltipColor("Or",colorOr),
		TooltipColor("Each filter is connected with \"OR\", every item is filtered, which is also filtered in at least one sub filter.", colorOr)});
	ImGui::SameLine(tabSpace);
	int index = 0;
	if (flags & FilterFlags::Or) index = 1;
	ImGui::PushItemWidth(-1);
	gotUpdated |= ImGui::Combo(UNIQUE_NO_DELIMITER("##operation", id), &index, new char*[2] {"And","Or"},2);
	ImGui::PopItemWidth();
	flags = (FilterFlags)((flags & ~FilterFlags::And) & ~FilterFlags::Or);
	if (index == 0)
		flags = (FilterFlags)(flags | FilterFlags::And);
	else if (index == 1)
		flags = (FilterFlags)(flags | FilterFlags::Or);

	RenderChildren();
}

void GroupFilter::RenderChildren() {
	std::lock_guard<std::recursive_mutex> lock(mutex);
	for (int i = 0; i < filterIndex; i++) {
		if (subFilters[i])
			subFilters[i]->Render();
	}
}

void GroupFilter::AddFilter(IFilter* filter) {
	std::lock_guard<std::recursive_mutex> lock(mutex);

	if (filterIndex == filterSize) {
		filterSize *= 2;
		IFilter** newFilters = new IFilter*[filterSize];
		memcpy(newFilters, subFilters, filterIndex * sizeof(IFilter*));
		delete[] subFilters;
		subFilters = newFilters;
	}
	subFilters[filterIndex] = filter;
	filter->parent = this;
	filterIndex++;
	gotUpdated = true;
}

void GroupFilter::RemoveFilter(IFilter* filter) {
	std::lock_guard<std::recursive_mutex> lock(mutex);
	int i = 0;
	for (; i < filterIndex; i++) {
		if (subFilters[i] == filter) {
			break;
		}
	}
	for (; i < filterIndex; i++) {
		if (i < filterIndex - 1)
			subFilters[i] = subFilters[i + 1];
		else
			subFilters[i] = nullptr;
	}
	filterIndex--;
	gotUpdated = true;
}


void GroupFilter::SerializeContent(tinyxml2::XMLPrinter & printer)
{
	std::lock_guard<std::recursive_mutex> lock(mutex);

	for (int i = 0; i < filterIndex; i++) {
		if (subFilters[i])
			subFilters[i]->Serialize(printer);
	}
}

void GroupFilter::DeserializeContent(tinyxml2::XMLElement * element)
{

	tinyxml2::XMLElement* child = element->FirstChildElement();
	tinyxml2::XMLElement* currentChild;
	while (child) {
		const char* type = child->Attribute("type");
		currentChild = child;
		child = child->NextSiblingElement();
		if (type == 0) continue;
		ItemFilterFactory iff = ItemFilterFactory::GetItemFilterFactory();
		IFilter* filter = iff.get(type);
		if (!filter) continue;
		filter = filter->CreateNew();
		filter->Deserialize(currentChild);
		AddFilter(filter);
	}
}

