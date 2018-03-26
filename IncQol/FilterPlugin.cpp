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

int FilterPlugin::maxTooltipSize = 20;

void FilterPlugin::Init() {
	Plugin::Init();
	LogString(LogLevel::Debug, "Creating window");
	window = new Window("Filters", false);
	infoWindow = new Window("FiltersSmallInfo", false);
	infoWindow->flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
	infoWindow->SetStartPosition(ImVec2(400.0f, 0.1f));
	infoWindow->SetFirstSize(ImVec2(0, 0));
	window->flags |= ImGuiWindowFlags_MenuBar;
	window->SetMinSize(ImVec2(100, 100));
	LogString(LogLevel::Debug, "Adding window");
	PluginBase* pluginBase = PluginBase::GetInstance();
	pluginBase->AddWindow(infoWindow);
	pluginBase->AddWindow(window);

	LogString(LogLevel::Debug, "Creating Filters");
	root = new RootGroupFilter();
	stdFilter = new RootGroupFilter();
	ascFilter = new RootGroupFilter();
	GroupFilter* safetyFilter = new GroupFilter();
	GroupFilter* safetyFilterAscended = new GroupFilter();
	RarityRangeItemFilter* rarityFilter = new RarityRangeItemFilter();
	RarityRangeItemFilter* rarityFilterAscended = new RarityRangeItemFilter();
	rarityFilter->SetValues(ItemRarity::Basic, ItemRarity::Ascended);
	rarityFilterAscended->SetValues(ItemRarity::Basic, ItemRarity::Exotic);
	safetyFilter->AddFilter(rarityFilter);
	safetyFilter->AddFilter(new SellableItemFilter());
	safetyFilterAscended->AddFilter(rarityFilterAscended);
	safetyFilterAscended->AddFilter(new SellableItemFilter());
	stdFilter->AddFilter(safetyFilter);
	ascFilter->AddFilter(safetyFilterAscended);

	LogString(LogLevel::Debug, "Filters successfully created");

	LogString(LogLevel::Debug, "Hooking Vendor Function start");
	lastUpdateIndex = 0;
	HookVendorFunc();
	LogString(LogLevel::Debug, "Hooking Vendor Function end");

	LoadFilterFrom(root, GetAddonFolder().append(STARTUP_FILTERNAME).c_str());
	copyItemKeyBind->plugin = GetName();
	openWindow->plugin = GetName();
	openInfoWindow->plugin = GetName();
	copyItemKeyBind->name = "CopyHoveredItem";
	openWindow->name = "OpenFilter";
	openInfoWindow->name = "OpenFilteredInfo";
	copyItemKeyBind->keys = Config::LoadKeyBinds(copyItemKeyBind->plugin, copyItemKeyBind->name, { VK_MENU,VK_SHIFT,'4' });
	openWindow->keys = Config::LoadKeyBinds(openWindow->plugin, openWindow->name, { VK_MENU,VK_SHIFT,'2' });
	openInfoWindow->keys = Config::LoadKeyBinds(openInfoWindow->plugin, openInfoWindow->name, { VK_MENU,VK_SHIFT,'3' });
	copyItemKeyBind->func = std::bind(&FilterPlugin::AddHoveredItemToFilter, this);
	openWindow->func = std::bind(&Window::ChangeState, window);
	openInfoWindow->func = std::bind(&Window::ChangeState, infoWindow);
	maxTooltipSize = (int)Config::LoadLong(GetName(), MAX_TOOLTIP_SIZE, maxTooltipSize);
	maxTooltipSize = CLAMP(maxTooltipSize, MIN_TOOLTIP, MAX_TOOLTIP);
	advancedOptions = Config::LoadBool(GetName(), ENABLE_ADVANCED_OPTIONS, false);
	sellDisabled = Config::LoadBool(GetName(), DISABLE_SELL, false);
	maxRetry = (int)Config::LoadLong(GetName(), MAX_SELL_RETRY, maxRetry);
	maxRetry = CLAMP(maxRetry, MIN_RETRY, MAX_RETRY);
	ascendedRarity = Config::LoadBool(GetName(), ENABLE_ASCENDED_RARITY, ascendedRarity);
	pluginBase->RegisterKeyBind(copyItemKeyBind);
	pluginBase->RegisterKeyBind(openWindow);
	pluginBase->RegisterKeyBind(openInfoWindow);
	LogString(LogLevel::Info, std::string(GetName())+ " initialization finished");
	ReloadFilterFiles();
}
const char * FilterPlugin::GetName()
{
	return "Filters";
}
void FilterPlugin::PluginMain()
{
	UpdateFilter();

	if (sellDisabled) return;
	if (!vendorSuccessful) return;
	if (*lastCallPtr == 0) return;
	uintptr_t func = (uintptr_t)*(lastCallPtr - 1);//-0x8 = 1
	if (func != *proxyVendorLocation) {
		lastCallPtr = new uintptr_t(0);
		return;
	}
	
	if (filteredCollection.size() > 0) {
		hl::ForeignClass vendor = (uintptr_t*)*lastCallPtr;
		FilterData data = (*filteredCollection.begin());
		if (!data->itemData) return;
		if (lastSlot != data->slot) {
			lastSlot = data->slot;
			curRetry = 0;
		}
		vendor.set<int>(0x50, data->slot);
		vendorFunc(new firstParam{ 0x0,0x31,lastCallPtr }, new secondParam{ 0x0,0x2,0x3 });
		curRetry++;
		int max = defaultRetry;
		if (advancedOptions)
			max = maxRetry;
		if (curRetry >= max) {
			LogString(LogLevel::Info, std::string("Skipping unsellable ID : ").append(std::to_string(data->itemData->id)));
			skipUnsellableIds.push_back(data->itemData->id);
		}
	}
	else {
		lastSlot = -1;
	}
}

void FilterPlugin::RenderOptions()
{
	if (ImGui::CollapsingHeader(GetName())) {
		if (RenderCheckbox("Disable selling", &sellDisabled, std::vector<TooltipColor> { TooltipColor("Disables addon selling.")})) {
			Config::SaveBool(GetName(), DISABLE_SELL, sellDisabled);
			Config::Save();
			lastCallPtr = new uintptr_t(0);
		}
		if (RenderSliderInt("Max tooltip size", &maxTooltipSize,MIN_TOOLTIP,MAX_TOOLTIP)) {
			Config::SaveLong(GetName(), MAX_TOOLTIP_SIZE, maxTooltipSize);
			Config::Save();
		}
		PluginBase::RenderKeyBind(openWindow, std::vector<TooltipColor> { TooltipColor("Shows / Hides the main filter window.")});
		PluginBase::RenderKeyBind(openInfoWindow, std::vector<TooltipColor> { TooltipColor("Shows / Hides the small info window.")});
		PluginBase::RenderKeyBind(copyItemKeyBind, std::vector<TooltipColor> { TooltipColor("Copies the item under the mouse and creates a filter."), TooltipColor("\n"),
			TooltipColor("Does work:"), TooltipColor("Inventory"), TooltipColor("Material storage"), TooltipColor("Guild Inventory")});
		if (RenderCheckbox("Advanced opt.", &advancedOptions, std::vector<TooltipColor> { TooltipColor("Enables the advanced options.")})) {
			Config::SaveBool(GetName(), ENABLE_ADVANCED_OPTIONS, advancedOptions);
			Config::Save();
			lastUpdateIndex = 0; //force to reload
		}
		if (advancedOptions) {
			ImGui::Separator();
			if (RenderCheckbox("Enable ascended", &ascendedRarity, std::vector<TooltipColor> { TooltipColor("Enables the possibility to sell ascended items."), TooltipColor("\n"), TooltipColor("Use at your own risk. Selling ascended items is normally not useful.",Addon::Colors[AddonColor_NegativeText])})) {
				Config::SaveBool(GetName(), ENABLE_ASCENDED_RARITY, ascendedRarity);
				Config::Save();
				lastUpdateIndex = 0; //force to reload
			}
			if (RenderSliderInt("Max sell retries", &maxRetry, MIN_RETRY, MAX_RETRY, 
				std::vector<TooltipColor> { TooltipColor("General rule : Too many unsellable items? Then just increase the retry value."),
				TooltipColor("\n"),
				TooltipColor("Retries depends on FPS and Connection (Normally fine with 10)."),
				TooltipColor("Lower FPS or low Connection -> Increase Retries"),
				TooltipColor("Higher FPS and good Connection -> Decrease Retries")})) {
				Config::SaveLong(GetName(), MAX_SELL_RETRY, maxRetry);
				Config::Save();
			}
			ImGui::Text("Unsellable items : ");
			ImGui::SameLine();
			ImGui::Text(std::to_string(skipUnsellableIds.size()).c_str());
			if (ImGui::Button("Clear unsellable items")) {
				skipUnsellableIds.clear();
			}
		}
	}
}

void FilterPlugin::UpdateFilter() {
	uint updateIndex;
	InventoryData* inventory = PluginBase::GetInstance()->GetInventory(&updateIndex);
	bool defaultFilterUpdated = root->Updated();
	root->ResetUpdateState();
	if (defaultFilterUpdated) {
		std::string folder = GetAddonFolder();
		SHCreateDirectoryExA(nullptr, folder.c_str(), nullptr);
		folder.append(STARTUP_FILTERNAME);
		SaveFilterAs(root, folder.c_str());
		lastCallPtr = new uintptr_t(0);
	}
	if ((lastUpdateIndex != updateIndex || defaultFilterUpdated) && inventory) {
		lastUpdateIndex = updateIndex;
		std::set<ItemStackData*> collection(inventory->itemStackDatas, inventory->itemStackDatas + inventory->realSize);
		if (!advancedOptions || !ascendedRarity) {
			collection = ascFilter->Filter(collection);
		}
		else {
			collection = stdFilter->Filter(collection);
		}
		filteredCollection = root->Filter(collection);
		for (auto it = filteredCollection.begin(); it != filteredCollection.end(); ) {
			FilterData data = (*filteredCollection.begin());
			if (std::find(skipUnsellableIds.begin(), skipUnsellableIds.end(), data->itemData->id) != skipUnsellableIds.end()) {
				filteredCollection.erase(it++);
			}
			else {
				++it;
			}
		}
		filteredItemDatas.clear();
		for (auto it = filteredCollection.begin(); it != filteredCollection.end(); ++it) {
			FilterData data = (*it);
			filteredItemDatas.push_back(data->itemData);
		}
	}
}

void FilterPlugin::Render() {
	if (window->Begin()) {
		RenderMenu();
		root->Render();
		root->CheckForDeletion();
		window->End();
	}
	if (infoWindow->Begin()) {
		char* text = "Total items: %3u";
		RenderItemsFiltered(text);
		infoWindow->End();
	}
}

void FilterPlugin::RenderItemsFiltered(const char* text) {
	int size = (int)filteredItemDatas.size();
	if (size > 0) {
		filteredItemDatas.sort(ItemData::sortName);
		filteredItemDatas.unique();
	}
	size = (int)filteredItemDatas.size();
	ImGui::PushStyleColor(ImGuiCol_Text, Addon::Colors[AddonColor_PositiveText]);
	ImGui::Text(text, size);
	ImGui::PopStyleColor();
	if (ImGui::IsItemHovered() && size > 0) {
		ImGui::BeginTooltip();
		int limit = min(size, maxTooltipSize);
		if (size > limit) {
			ImGui::Text("Use Ctrl + Mouse Wheel to scroll");
			ImGui::Text("");
		}
		ImGuiIO io = ImGui::GetIO();
		if (io.KeyCtrl)
			filteredItemDatasStartY -= (int)io.MouseWheel;
		if (filteredItemDatasStartY + maxTooltipSize >= size) filteredItemDatasStartY = size - maxTooltipSize;
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
						if (SaveFilterAs(root,folder.append(".filter").c_str())) {
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
						if (LoadFilterFrom(root,GetFilterFolder().append(filesToLoad[loadIndex]).c_str(), appendLoad)) {
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
		if (ImGui::BeginMenu("Root Filter")) {
			root->CustomMenu();
			if (ImGui::Selectable("Expand all")) {
				root->SetOpen(true);
			}
			if (ImGui::Selectable("Collapse all")) {
				root->SetOpen(false);
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DRAG_DROP_PAYLOAD_TYPE_FILTER)) {
				uintptr_t* f = (uintptr_t*)payload->Data;
				IFilter* filter = (IFilter*)*f;
				if (filter && !root->HasParent(filter)) {
					if (filter->parent) {
						GroupFilter* groupFilter = dynamic_cast<GroupFilter*>(filter->parent);
						if (groupFilter) {
							groupFilter->RemoveFilter(filter);
						}
					}
					root->AddFilter(filter);
				}
			}
			ImGui::EndDragDropTarget();
		}

		float textSize = ImGui::CalcTextSize("Total items : 125").x;
		char* text = "Total items: %3u";
		float c = ImGui::CalcItemWidth();
		float a = ImGui::GetContentRegionAvailWidth();
		if (a > textSize) {
			ImGui::SameLine(0, a - textSize + ImGui::GetStyle().ColumnsMinSpacing * 2);
			RenderItemsFiltered(text);
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
		ItemData* data = PluginBase::GetInstance()->GetHoveredItem();
		if (!data) return;
		if (!data->sellable) return;
		GroupFilter* groupFilter = new GroupFilter();
		IDItemFilter* idFilter = new IDItemFilter();
		LevelItemFilter* levelFilter = new LevelItemFilter();
		RarityItemFilter* rarityFilter = new RarityItemFilter();
		TypeItemFilter* typeFilter = new TypeItemFilter();
		groupFilter->SetName(std::string(data->name));
		idFilter->SetValue(data->id);
		levelFilter->SetValue(data->level);
		rarityFilter->SetValue(data->rarity);
		typeFilter->SetValue(data->itemtype);
		levelFilter->SetActive(false);
		rarityFilter->SetActive(false);
		typeFilter->SetActive(false);
		groupFilter->AddFilter(idFilter);
		groupFilter->AddFilter(levelFilter);
		groupFilter->AddFilter(rarityFilter);
		groupFilter->AddFilter(typeFilter);
		groupFilter->SetOpen(false);
		root->AddFilter(groupFilter);
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
