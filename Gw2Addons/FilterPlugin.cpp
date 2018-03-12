#include "FilterPlugin.h"
#include "RequestTradingpostTask.h"
#include "FilterPluginUtility.h"
#include "IDItemFilter.h"
#include "LevelItemFilter.h"
#include "RarityItemFilter.h"
#include "TypeItemFilter.h"
#include "Utility.h"
#include "SellableItemFilter.h"
#include "RarityRangeItemFilter.h"
#include "ItemFilterFactory.h"

uintptr_t* FilterPlugin::vendorSource = new uintptr_t(0);
uintptr_t* FilterPlugin::lastCallPtr = new uintptr_t(0);

void FilterPlugin::Init() {
	Plugin::Init();
	LogString(LogLevel::Debug, "Creating window");
	window = new Window("Filters", false);
	window->flags |= ImGuiWindowFlags_MenuBar;
	LogString(LogLevel::Debug, "Adding window");
	PluginBase* pluginBase = PluginBase::GetInstance();
	pluginBase->AddWindow(window);

	LogString(LogLevel::Debug, "Creating Filters");
	root = new RootGroupFilter();
	rootCopy = new RootGroupFilter();
	stdFilter = new RootGroupFilter();
	GroupFilter* groupFilter = new GroupFilter();
	RarityRangeItemFilter* rarityFilter = new RarityRangeItemFilter();
	rarityFilter->SetValues(ItemRarity::Basic, ItemRarity::Ascended);
	groupFilter->AddFilter(rarityFilter);
	groupFilter->AddFilter(new SellableItemFilter());
	stdFilter->AddFilter(groupFilter);

	LogString(LogLevel::Debug, "Filters successfully created");

	LogString(LogLevel::Debug, "Hooking Vendor Function start");
	lastUpdateIndex = 0;
	HookVendorFunc();
	LogString(LogLevel::Debug, "Hooking Vendor Function end");

	LoadFilterFrom(rootCopy, GetAddonFolder().append(STARTUP_FILTERNAME).c_str());
	copyItemKeyBind->plugin = GetName();
	openWindow->plugin = GetName();
	copyItemKeyBind->name = "CopyHoveredItem";
	openWindow->name = "OpenFilter";
	copyItemKeyBind->keys = Config::LoadKeyBinds(copyItemKeyBind->plugin, copyItemKeyBind->name, { VK_MENU,VK_SHIFT,'A' });
	openWindow->keys = Config::LoadKeyBinds(openWindow->plugin, openWindow->name, { VK_MENU,VK_SHIFT,'F' });
	copyItemKeyBind->func = std::bind(&FilterPlugin::AddHoveredItemToFilter, this);
	openWindow->func = std::bind(&Window::ChangeState, window);
	pluginBase->RegisterKeyBind(copyItemKeyBind);
	pluginBase->RegisterKeyBind(openWindow);
	LogString(LogLevel::Info, std::string(GetName())+ " initialization finished");
	ReloadFilterFiles();
}
const char * FilterPlugin::GetName()
{
	return "Filters";
}
void FilterPlugin::PluginMain()
{
	uint32_t updateIndex;
	InventoryData inventory = PluginBase::GetInstance()->GetInventory(&updateIndex);
	*root = *rootCopy;
	bool defaultFilterUpdated = rootCopy->Updated();
	rootCopy->ResetUpdateState();
	if (defaultFilterUpdated) {
		std::string folder = GetAddonFolder();
		SHCreateDirectoryExA(nullptr, folder.c_str(), nullptr);
		folder.append(STARTUP_FILTERNAME);
		SaveFilterAs(root,folder.c_str());
		lastCallPtr = new uintptr_t(0);
	}
	if (lastUpdateIndex != updateIndex || defaultFilterUpdated) {
		lastUpdateIndex = updateIndex;
		std::set<ItemStackData> collection = std::set<ItemStackData>(inventory.itemStackDatas.begin(), inventory.itemStackDatas.end());
		collection = stdFilter->Filter(collection);
		filteredCollection = root->Filter(collection);
		lastItemsFilteredCount = root->GetFilteredCount();
	}
	if (!vendorSuccessful) return;
	if (*lastCallPtr == 0) return;
	uintptr_t func = (uintptr_t)*(lastCallPtr - 1);//-0x8 = 1
	if (func != *proxyVendorLocation) {
		lastCallPtr = new uintptr_t(0);
		return;
	}
	for (auto it = filteredCollection.begin(); it != filteredCollection.end(); ) {
		ItemStackData data = (*filteredCollection.begin());
		if (std::find(skipUnsellableIds.begin(), skipUnsellableIds.end(), data.itemData.id) != skipUnsellableIds.end()) {
			filteredCollection.erase(it++);
		}
		else {
			++it;
		}
	}
	if (filteredCollection.size() > 0) {
		hl::ForeignClass vendor = (uintptr_t*)*lastCallPtr;
		ItemStackData data = (*filteredCollection.begin());
		if (lastSlot != data.slot) {
			lastSlot = data.slot;
			curRetry = 0;
		}
		vendor.set<int>(0x50, data.slot);
		vendorFunc(new firstParam{ 0x0,0x31,lastCallPtr }, new secondParam{ 0x0,0x2,0x3 });
		curRetry++;
		if (curRetry >= maxRetry) {
			LogString(LogLevel::Info, std::string("Skipping unsellable ID : ").append(std::to_string(data.itemData.id)));
			skipUnsellableIds.push_back(data.itemData.id);
		}
	}
	else {
		lastSlot = -1;
	}
}

void FilterPlugin::Render() {
	if (window->Begin()) {
		RenderMenu();
		rootCopy->Render();
		rootCopy->CheckForDeletion();
		window->End();
	}
}

void FilterPlugin::RenderMenu() {
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("Menu"))
		{
			if (ImGui::BeginMenu("Export Filter")) {
				ImGui::Text("Enter a filename :");
				ImGui::PushItemWidth(300);
				if (ImGui::InputText("##savefilename", fileName, 64, ImGuiInputTextFlags_CallbackCharFilter, TextFilters::FilterImGuiLetters)) {
					extraMessageSave = false;
				}
				ImGui::PopItemWidth();
				if (ImGui::Button("Save")) {
					extraMessageSave = true;
					std::string fileNameString = std::string(fileName);
					trim(fileNameString);
					if (fileNameString.size() == 0) {
						extraMessage = "Please enter a filename!";
						extraMessageColor = Addon::Colors[AddonColor_NegativeText];
					}
					else {
						std::string folder = GetFilterFolder();
						SHCreateDirectoryExA(nullptr, folder.c_str(), nullptr);
						folder.append(fileNameString);
						if (SaveFilterAs(rootCopy,folder.append(".filter").c_str())) {
							extraMessage = "Filter successfully saved!";
							extraMessageColor = Addon::Colors[AddonColor_PositiveText];
						}
						else {
							extraMessage = "Filter could not be saved!";
							extraMessageColor = Addon::Colors[AddonColor_NegativeText];
						}
					}
				}
				if (extraMessageSave) {
					ImGui::PushStyleColor(ImGuiCol_Text, extraMessageColor);
					ImGui::Text(extraMessage.c_str());
					ImGui::PopStyleColor();
				}
				ImGui::EndMenu();
			}
			else {
				extraMessageSave = false;
			}
			if (ImGui::BeginMenu("Import Filter")) {
				if (firstOpenedImportFilter) {
					ReloadFilterFiles();
					firstOpenedImportFilter = false;
				}
				ImGui::Text("Select a file to load: ");
				ImGui::PushItemWidth(300);
				ImGui::ListBox("##loadlist", &loadIndex, filesToLoad, fileCount, 10);
				ImGui::PopItemWidth();
				ImGui::Text("Append filter ");
				ImGui::SameLine();
				ImGui::Checkbox("##appendLoad", &appendLoad);
				if (ImGui::Button("Load")) {
					extraMessageLoad = true;
					if (loadIndex == -1) {
						extraMessage = "No filter selected!";
						extraMessageColor = Addon::Colors[AddonColor_NegativeText];
					}
					else {
						if (LoadFilterFrom(rootCopy,GetFilterFolder().append(filesToLoad[loadIndex]).c_str(), appendLoad)) {
							extraMessage = "Filter successfully loaded!";
							extraMessageColor = Addon::Colors[AddonColor_PositiveText];
						}
						else {
							extraMessage = "Filter could not be loaded!";
							extraMessageColor = Addon::Colors[AddonColor_NegativeText];
						}
					}
				}
				float buttonSize = 65.f;
				ImGui::SameLine();
				if (ImGui::Button("Refresh", ImVec2(buttonSize, 0))) {
					ReloadFilterFiles();
					extraMessageLoad = true;
					extraMessage = "Filter reloaded";
					extraMessageColor = Addon::Colors[AddonColor_PositiveText];
				}
				if (extraMessageLoad) {
					ImGui::PushStyleColor(ImGuiCol_Text, extraMessageColor);
					ImGui::Text(extraMessage.c_str());
					ImGui::PopStyleColor();
				}
				ImGui::EndMenu();
			}
			else {
				extraMessageLoad = false;
				firstOpenedImportFilter = true;
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Hide")) {
				if (window) window->SetOpen(false);
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit")) {
			rootCopy->CustomMenu();
			if (ImGui::Selectable("Expand all")) {
				rootCopy->SetOpen(true);
			}
			if (ImGui::Selectable("Collapse all")) {
				rootCopy->SetOpen(false);
			}
			ImGui::EndMenu();
		}

		float textSize = ImGui::CalcTextSize("Total items : 125").x;
		char* text = "Total items: %3u";
		float c = ImGui::CalcItemWidth();
		float a = ImGui::GetContentRegionAvailWidth();
		if (a > textSize) {
			ImGui::SameLine(0, a - textSize + ImGui::GetStyle().ColumnsMinSpacing * 2);
			ImGui::PushStyleColor(ImGuiCol_Text, Addon::Colors[AddonColor_PositiveText]);
			ImGui::Text(text, lastItemsFilteredCount);
			ImGui::PopStyleColor();
		}

		ImGui::EndMenuBar();
	}

}


void FilterPlugin::ReloadFilterFiles() {
	for (int i = 0; i < fileCount; i++) {
		delete[] filesToLoad[i];
	}
	delete[] filesToLoad;
	std::vector<std::string> files = ListFilesInDir(GetFilterFolder(), "*.filter");
	fileCount = (int)files.size();
	filesToLoad = new const char*[fileCount];
	for (int i = 0; i < fileCount; i++) {
		char* file = new char[files[i].size() + 1];
		strcpy_s(file, files[i].size()+1, files[i].c_str());
		filesToLoad[i] = file;
	}
}

void FilterPlugin::AddHoveredItemToFilter()
{
	if (PluginBase::GetInstance()->HasHoveredItem()) {
		ItemData data = PluginBase::GetInstance()->GetHoveredItem();
		if (!data.sellable) return;
		GroupFilter* groupFilter = new GroupFilter();
		IDItemFilter* idFilter = new IDItemFilter();
		LevelItemFilter* levelFilter = new LevelItemFilter();
		RarityItemFilter* rarityFilter = new RarityItemFilter();
		TypeItemFilter* typeFilter = new TypeItemFilter();
		groupFilter->SetName(std::string(data.name));
		idFilter->SetValue(data.id);
		levelFilter->SetValue(data.level);
		rarityFilter->SetValue(data.rarity);
		typeFilter->SetValue(data.itemtype);
		levelFilter->SetActive(false);
		rarityFilter->SetActive(false);
		typeFilter->SetActive(false);
		groupFilter->AddFilter(idFilter);
		groupFilter->AddFilter(levelFilter);
		groupFilter->AddFilter(rarityFilter);
		groupFilter->AddFilter(typeFilter);
		groupFilter->SetOpen(false);
		rootCopy->AddFilter(groupFilter);
	}
}


void FilterPlugin::HookVendorFunc() {
	proxyVendorLocation = new uintptr_t((uintptr_t)vendorFunc);
	uintptr_t vendorHookBasePtr = hl::FindPattern("C7 44 24 ?? ?? ?? ?? ?? C7 44 24 ?? ?? ?? ?? ?? C7 44 24 ?? ?? ?? ?? ?? e8 ?? ?? ?? ?? 0F B6 9C 24 ?? ?? ?? ?? 4C 8D 25 ?? ?? ?? ??");
	if (!vendorHookBasePtr) {
		LogString(LogLevel::Error, "Pattern for VendorFunc is invalid");
		return;
	}
	vendorHookBasePtr = hl::FollowRelativeAddress(vendorHookBasePtr + 0x28);
	if (!vendorHookBasePtr) {
		LogString(LogLevel::Error, "Follow for VendorFunc is invalid");
		return;
	}
	LogString(LogLevel::Info, "Hooking Vendor addr: "+ToHex(vendorHookBasePtr));
	uintptr_t funcLocation = vendorHookBasePtr + 0x30;
	LogString(LogLevel::Info, "FunctionLocation addr: " + ToHex(funcLocation));
	hl::PageProtect((void*)funcLocation, 8, hl::PROTECTION_READ_WRITE_EXECUTE);
	memcpy((void*)vendorSource, (void*)funcLocation, 8);
	memcpy((void*)funcLocation, proxyVendorLocation, 8);
	LogString(LogLevel::Info, "OldFunction addr: " + ToHex((uintptr_t)*vendorSource));
	LogString(LogLevel::Info, "NewFunction addr: " + ToHex((uintptr_t)*((uintptr_t*)funcLocation)));
	vendorSuccessful = true;
}

void __stdcall vendorFunc(firstParam* a, secondParam* b) {
	static auto orgFunc = ((void(__fastcall*)(firstParam*, secondParam*))*FilterPlugin::vendorSource);
	if (a && b) {//make sure a and b are pointers
		if ( (uintptr_t)b != 1 && b->id == 3 && b->id2 == 3) {//when b is 1 then its no pointer obviously, else check the correct params 3,3 for sell junk
			FilterPlugin::lastCallPtr = a->ptr;
		}
	}
	orgFunc(a, b);
}

bool FilterPlugin::LoadFilterFrom(RootGroupFilter* rootFilter, const char * name,bool append)
{
	LogString(LogLevel::Info, "Loading filter : "+std::string(name));
	LogString(LogLevel::Info, append ? "Appending = true": "Appending = false");
	if (!append)
		rootFilter->RemoveAndDeleteAll();
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError error = doc.LoadFile(name);
	if (error != tinyxml2::XMLError::XML_SUCCESS) {
		LogString(LogLevel::Error, "Could not load filter XML_ERROR : " + std::to_string(error));
		return false;
	}
	tinyxml2::XMLElement* root = doc.RootElement();
	if (root) {
		tinyxml2::XMLElement* child = root->FirstChildElement();
		tinyxml2::XMLElement* currentChild;
		while (child) {
			const char* type = child->Attribute("type");
			currentChild = child;
			child = child->NextSiblingElement();
			if (!type) {
				LogString(LogLevel::Error, "Element is missing type");
				continue;
			}
			LogString(LogLevel::Info, "Adding filter type : " + std::string(type));
			ItemFilterFactory iff = ItemFilterFactory::GetItemFilterFactory();
			IFilter* filter = iff.get(type);
			if (!filter) {
				LogString(LogLevel::Error, "Filter type not defined! corrupted File? wrong version?");
				continue;
			}
			filter = filter->CreateNew();
			filter->Deserialize(currentChild);
			filter->SetOpen(false);
			rootFilter->AddFilter(filter);
		}
	}
	LogString(LogLevel::Info, "Filter successfully loaded");
	return true;
}



bool FilterPlugin::SaveFilterAs(RootGroupFilter* rootFilter, const char * name)
{
	std::FILE* file;
	errno_t err = fopen_s(&file,name, "w");
	if (file) {
		tinyxml2::XMLDocument doc;
		LogString(LogLevel::Debug, "Creating XMLPrinter");
		tinyxml2::XMLPrinter printer(file);
		LogString(LogLevel::Debug, "Serializing filter");
		rootFilter->Serialize(printer);
		LogString(LogLevel::Debug, "Saving File");
		doc.Print(&printer);
		LogString(LogLevel::Debug, "Closing File");
		fclose(file);
		//LogString(LogLevel::Info, "File created : " + std::string(name));
		return true;
	}
	LogString(LogLevel::Error, "Could not create File : " + std::string(name));
	LogString(LogLevel::Error, "Errno : " + std::to_string(err));
	return false;
}
