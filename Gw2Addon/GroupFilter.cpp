#include "GroupFilter.h"

GroupFilter::GroupFilter() {
	flags = FilterFlags::And;
	name = "Unnamed Group Filter"; 
	nameCalculated = false;
}

GroupFilter::~GroupFilter() {

}
std::set<ItemStackData> GroupFilter::Filter(std::set<ItemStackData> collection)
{
	std::set<ItemStackData> combinedSet;
	std::vector<IFilter*>::iterator iter;

	if (flags & FilterFlags::And) {
		combinedSet = collection;
	}
	int activeCount = 0;
	if (IsActive()) {
		for (iter = subFilters.begin(); iter != subFilters.end(); ++iter) {
			bool filterActive = (*iter)->IsActive();
			if (filterActive) {
				activeCount++;
			}
			std::set<ItemStackData> filteredSet = (*iter)->Filter(collection);
			if (!filterActive) continue;
			if (flags & FilterFlags::And) {
				std::set<ItemStackData> intersection;
				std::set_intersection(combinedSet.begin(), combinedSet.end(), filteredSet.begin(), filteredSet.end(), std::inserter(intersection, intersection.end()));
				combinedSet = intersection;
			}
			else if (flags & FilterFlags::Or) {
				combinedSet.insert(filteredSet.begin(), filteredSet.end());
			}
		}
	}

	if (activeCount == 0) {
		filteredItems = 0;
		return std::set<ItemStackData>();
	}
	if (flags & FilterFlags::Not) {
		std::set<ItemStackData> invers;
		std::set_difference(collection.begin(), collection.end(), combinedSet.begin(), combinedSet.end(), std::inserter(invers, invers.end()));
		combinedSet = invers;
	}
	filteredItems = (int)combinedSet.size();
	return combinedSet;
}

bool GroupFilter::Updated()
{
	if (gotUpdated) return true;

	std::vector<IFilter*>::iterator iter;

	for (iter = subFilters.begin(); iter != subFilters.end(); ++iter ) {
		if ((*iter)->Updated()) return true;
	}
	return false;
}

void GroupFilter::ResetUpdateState()
{
	IFilter::ResetUpdateState();
	std::vector<IFilter*>::iterator iter;

	for (iter = subFilters.begin(); iter != subFilters.end(); ++iter ) {
		(*iter)->ResetUpdateState();
	}
}

char * GroupFilter::GetSerializeName()
{
	return "GroupFilter";
}

void GroupFilter::SetOpen(bool open)
{
	IFilter::SetOpen(open);
	std::vector<IFilter*>::iterator iter;

	for (iter = subFilters.begin(); iter != subFilters.end(); ++iter) {
		(*iter)->SetOpen(open);
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

	std::vector<IFilter*>::iterator iter;

	for (iter = subFilters.begin(); iter != subFilters.end(); ) {
		IFilter* filter = *iter;
		GroupFilter* groupFilter = dynamic_cast<GroupFilter*>(filter);
		if (groupFilter) groupFilter->RemoveAndDeleteAll();
		iter = RemoveFilter(filter);
		delete filter;
	}
}

void GroupFilter::CheckForDeletion()
{
	std::vector<IFilter*>::iterator iter;

	for (iter = subFilters.begin(); iter != subFilters.end(); ) {
		IFilter* filter = *iter;
		GroupFilter* groupFilter = dynamic_cast<GroupFilter*>(filter);
		if (filter->IsMarkedForDeletion()) {
			iter = RemoveFilter(filter);
			if (groupFilter) {
				groupFilter->RemoveAndDeleteAll();
			}
			delete filter;
		}
		else {
			if (groupFilter) {
				groupFilter->CheckForDeletion();
			}
			++iter;
		}
	}
}

void GroupFilter::RenderContent() {
	char *cpy = new char[32];
	strcpy_s(cpy,32, name.c_str());

	float tab = ImGui::GetWindowContentRegionWidth() - ImGui::GetContentRegionAvailWidth() + 120;

	ImGui::Text("Name : ");
	ImGui::SameLine(tab);
	ImGui::PushItemWidth(-1);
	gotUpdated |= ImGui::InputText(UNIQUE_NO_DELIMITER("##nameinput", id), cpy, 32);
	ImGui::PopItemWidth();

	name = std::string(cpy);
	ImGui::Text("Operation : ");
	ImGui::SameLine(tab);
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
	/*bool orOperation = (flags & GroupFlags::Or) == GroupFlags::Or;
	bool andOperation = (flags & GroupFlags::And) == GroupFlags::And;
	ImGui::SameLine(tab);
	ImGui::Text("And ");
	ImGui::SameLine();
	if (ImGui::RadioButton(UNIQUE_NO_DELIMITER("##and", id), andOperation) && !andOperation) {
		gotUpdated = true;
		flags = (GroupFlags)(flags ^ GroupFlags::And ^ GroupFlags::Or);
	}
	ImGui::SameLine();
	ImGui::Text("Or ");
	ImGui::SameLine();
	if (ImGui::RadioButton(UNIQUE_NO_DELIMITER("##or", id), orOperation) && !orOperation) {
		gotUpdated = true;
		flags = (GroupFlags)(flags ^ GroupFlags::Or ^ GroupFlags::And);
	}*/
	bool invert = (flags & FilterFlags::Not);
	ImGui::Text("Inverted : ");
	ImGui::SameLine(tab);
	if (ImGui::Checkbox(UNIQUE_NO_DELIMITER("##invertFilter", id), &invert)) {
		gotUpdated = true;
		flags = (FilterFlags)(flags ^ FilterFlags::Not);
	}

	RenderChildren();
}

void GroupFilter::RenderChildren() {
	std::vector<IFilter*>::iterator iter;

	for (iter = subFilters.begin(); iter != subFilters.end(); ++iter) {
		(*iter)->Render();
	}
}

void GroupFilter::AddFilter(IFilter* filter) {
	subFilters.push_back(filter);
	gotUpdated = true;
}

std::vector<IFilter*>::iterator GroupFilter::RemoveFilter(IFilter* filter) {
	std::vector<IFilter*>::iterator it = std::find(subFilters.begin(), subFilters.end(), filter);
	gotUpdated = true;
	return subFilters.erase(it);
}


void GroupFilter::SerializeContent(tinyxml2::XMLPrinter & printer)
{
	printer.PushAttribute("flags", flags);
	std::vector<IFilter*>::iterator iter;
	for (iter = subFilters.begin(); iter != subFilters.end(); ++iter) {
		(*iter)->Serialize(printer);
	}
}

void GroupFilter::DeserializeContent(tinyxml2::XMLElement * element)
{
	flags = (FilterFlags)element->IntAttribute("flags", FilterFlags::And);
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

