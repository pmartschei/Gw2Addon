#include "PluginBase.h"
#include <iterator>
#include <sstream>
#include "Plugin.h"
#include "RequestTradingpostTask.h"
#include "Window.h"
#include "Config.h"
#include "Utility.h"
#include "AddonColors.h"
#include "Logger.h"
#include "windows.h"
#include "psapi.h"

void __fastcall hkGameThread(uintptr_t, int, int);
void __fastcall cbDecodeText(uintptr_t* ctx, wchar_t* decodedText);

bool PluginBase::IsCloseWindowBindDown() {
	return std::includes(_frameDownKeys.begin(), _frameDownKeys.end(), _closeWindowKeys.begin(), _closeWindowKeys.end());
}
bool PluginBase::KeysDown(std::set<uint> keys)
{
	bool identically = std::includes(_downKeys.begin(), _downKeys.end(), keys.begin(), keys.end()); // _downKeys == keys;
	if (!identically) return false;
	std::set<uint> diff;
	std::set_difference(_frameDownKeys.begin(), _frameDownKeys.end(), keys.begin(), keys.end(), std::inserter(diff, diff.begin()));
	return diff.size() < _frameDownKeys.size();
}
bool PluginBase::PushKeys(std::list<EventKey> eventKeys)
{
	_frameDownKeys.clear();
	if (eventKeys.size() == 0) return false;
	bool keyUp = false;
	bool escape = false;
	std::set<uint> lastKeys = std::set<uint>(_downKeys.begin(), _downKeys.end());
	for (const auto& k : eventKeys) {
		if (k.down) {
			if (k.vk == VK_ESCAPE)
				escape = true;
			size_t current = _downKeys.count(k.vk);
			_downKeys.insert(k.vk);
			lastKeys.insert(k.vk);
			if (current != _downKeys.count(k.vk)) {
				_frameDownKeys.insert(k.vk);
			}
		}
		else {
			size_t current = _downKeys.count(k.vk);
			_downKeys.erase(k.vk);
			if (current != _downKeys.count(k.vk)) {
				keyUp = true;
			}
		}
	}
	lastKeys.erase(VK_LBUTTON);
	if (keybindVisual) {
		if (escape) {
			keybindVisual->isSetMode = false;
			keybindVisual = nullptr;
			return true;
		}
		keybindVisual->isSetMode = !keyUp;
		std::set<uint> keys = std::set<uint>(lastKeys.begin(), lastKeys.end());
		keybindVisual->keys = keys;
		if (keyUp)
			keybindVisual = nullptr;
		return true;
	}
	return false;
}

bool PluginBase::KeybindText(std::string suffix,KeyBindData* data) {
	std::string text = "";
	for (auto it = data->keys.begin(), end = data->keys.end(); it != end;) {
		text.append(GetKeyName((*it)));
		++it;
		if (it != end) {
			text.append(" + ");
		}
	}
	float buttonWidth = 50.f;
	ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() - buttonWidth - ImGui::GetStyle().ItemInnerSpacing.x);
	int popcount = 0;
	if (data->isSetMode) {
		popcount = 2;
		ImGui::PushStyleColor(ImGuiCol_FrameBg, Addon::Colors[AddonColor_FrameBgHighlighted]);
		ImGui::PushStyleColor(ImGuiCol_Text, Addon::Colors[AddonColor_TextHighlighted]);
	}
	ImGui::InputText(suffix.c_str(),&text[0],text.size(),ImGuiInputTextFlags_ReadOnly);
	ImGui::PopStyleColor(popcount);
	ImGui::PopItemWidth();
	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
	if (!data->isSetMode && ImGui::Button(("Set" + suffix).c_str(),ImVec2(buttonWidth,0)))
	{
		data->isSetMode = true;
		if (keybindVisual) {
			keybindVisual->isSetMode = false;
		}
		keybindVisual = data;
		return true;
	}
	else if (data->isSetMode && ImGui::Button(("Clear" + suffix).c_str(), ImVec2(buttonWidth, 0)))
	{
		keybindVisual = nullptr;
		data->isSetMode = false;
		data->keys = std::set<uint>();
		return true;
	}
	if (data->isSetMode) {
		return true;
	}
	return false;
}

void PluginBase::Init() {
	ReloadConfig();
	Logger::LogString(LogLevel::Debug, MAIN_INFO, "Creating option window");
	optionWindow = new Window("Options");
	optionWindow->SetMinSize(ImVec2(300, 300));
	AddWindow(optionWindow);
	Logger::LogString(LogLevel::Debug, MAIN_INFO, "Creating keybind for option window");
	KeyBindData* openOptions = new KeyBindData();
	openOptions->plugin = MAIN_INFO;
	openOptions->name = "OpenOptions";
	openOptions->keys = Config::LoadKeyBinds(openOptions->plugin, openOptions->name, { VK_MENU,VK_SHIFT,'O' });
	openOptions->func = std::bind(&Window::ChangeState, optionWindow);
	Logger::LogString(LogLevel::Debug, MAIN_INFO, "Registering keybind for option window");
	RegisterKeyBind(openOptions);

	chainLoad = Config::LoadText(MAIN_INFO, "chainload", "d3d9_incqol_chain.dll");
	LoadColors(AddonColor_COUNT, Addon::GetStyleColorName, Addon::Colors);
	LoadColors(ImGuiCol_COUNT, ImGui::GetStyleColorName, ImGui::GetStyle().Colors);
}


void PluginBase::AddWindow(Window * window)
{
	_windows.push_back(window);
}

void PluginBase::AddPlugin(Plugin* plugin) {
	loadedPlugins.push_back(plugin);
}

void PluginBase::CloseFocusedWindow()
{
	if (_focusedWindow) {
		_focusedWindow->SetOpen(false);
	}
}

bool PluginBase::HasFocusWindow()
{
	_focusedWindow = 0;
	for (auto iterator = _windows.begin(), end = _windows.end(); iterator != end; ++iterator) {
		Window* window = *iterator;
		if (window->IsFocus()) {
			_focusedWindow = window;
			return true;
		}
	}
	return false;
}

void PluginBase::ReloadConfig()
{
	configItemsUrl = Config::LoadText(MAIN_INFO, "UrlItemInfo", "https://api.guildwars2.com/v2/items/");
	configPricesUrl = Config::LoadText(MAIN_INFO, "UrlPricesInfo", "https://api.guildwars2.com/v2/commerce/listings/");
}

void PluginBase::SetupContext()
{
	pCtx = GetContext();

	SetupPlayer();
	SetupGuild();
	SetupMisc();

	if (currentPointers.itemPtr) {
		SetHoveredItem(currentPointers.hoveredItemData);
	}
	else {
		SetHoveredItem(nullptr);
	}
}

void PluginBase::CheckInitialize()
{
	switch (pluginBaseState) {
	case PluginBaseState::CREATED: {
		GetContext = hl::FindPattern("65 48 8b 04 25 58 00 00 00 ba 08 00 00 00");
		pluginBaseState = PluginBaseState::CONTEXT_FINISHED;
		if (!GetContext.data()) {
			//can not find any context (aob is invalid)
			pluginBaseState = PluginBaseState::FAILURE;
			Logger::LogString(LogLevel::Error, MAIN_INFO, "Pattern for Context is invalid");
			return;
		}
		GetContext = (uintptr_t)GetContext.data() - 0x6;
		Logger::LogString(LogLevel::Info, MAIN_INFO, "GetContext addr: " + ToHex((uintptr_t)GetContext.data()));
	}
	case PluginBaseState::CONTEXT_FINISHED: {
		currentPointers.mouseFocusBase = hl::FindPattern("33 DB 41 B9 22 00 00 00 48 8D 0D");
		pluginBaseState = PluginBaseState::MOUSE_FOCUS_FINISHED;
		if (currentPointers.mouseFocusBase) {
			currentPointers.mouseFocusBase += 0xB;
			currentPointers.mouseFocusBase = hl::FollowRelativeAddress(currentPointers.mouseFocusBase);
			Logger::LogString(LogLevel::Info, MAIN_INFO, "MouseHover addr: " + ToHex(currentPointers.mouseFocusBase));
		}
		else {
			Logger::LogString(LogLevel::Info, MAIN_INFO, "Pattern for Mouse Hover is invalid, no Mouse Hover available");
		}
	}
	case PluginBaseState::MOUSE_FOCUS_FINISHED: {
		GetCodedTextFromHashId = hl::FindPattern("53 57 48 83 EC 48 8B D9 E8 ?? ?? ?? ?? 48 8B 48 50 E8 ?? ?? ?? ?? 44 8B 4C 24 68 48 8D 4C 24 30 48 8B F8");
		pluginBaseState = PluginBaseState::CODED_TEXT_FINISHED;
		if (GetCodedTextFromHashId.data()) {
			GetCodedTextFromHashId = (uintptr_t)GetCodedTextFromHashId.data() - 0xE;
			Logger::LogString(LogLevel::Info, MAIN_INFO, "GetCodedTextFromHashId addr: " + ToHex((uintptr_t)GetCodedTextFromHashId.data()));
		}
		else {
			Logger::LogString(LogLevel::Info, MAIN_INFO, "Pattern for CodedText is invalid, no CodedText available");
		}
	}
	case PluginBaseState::CODED_TEXT_FINISHED: {
		DecodeText = hl::FindPattern("49 8B E8 48 8B F2 48 8B F9 48 85 C9 75 19 41 B8 ?? ?? ?? ?? 48");
		pluginBaseState = PluginBaseState::DECODE_TEXT_FINISHED;
		if (DecodeText.data()) {
			DecodeText = (uintptr_t)DecodeText.data() - 0x14;
			Logger::LogString(LogLevel::Info, MAIN_INFO, "DecodeText addr: " + ToHex((uintptr_t)DecodeText.data()));
		}
		else {
			Logger::LogString(LogLevel::Info, MAIN_INFO, "Pattern for DecodeText is invalid, no DecodeText available");
		}
	}
	case PluginBaseState::DECODE_TEXT_FINISHED: {
		hl::PatternScanner scanner;
		auto results = scanner.find({
			"ViewAdvanceDevice"/*,
							   "ViewAdvanceAgentSelect",
							   "ViewAdvanceAgentView",
							   "ViewAdvanceWorldView",
							   "CompassManager()->IsCompassFixed()",
							   "ViewAdvanceUi",
							   "ultimateMasterCharacter",
							   "m_currCamera",
							   "guid != MEM_CATEGORY_INVALID && guid < m_headGUID"*/
		}, "Gw2-64.exe");
		pluginBaseState = PluginBaseState::SCANNER_FINISHED;

		if (!results[0]) {
			pluginBaseState = PluginBaseState::FAILURE;
			Logger::LogString(LogLevel::Error, MAIN_INFO, "Scan for AlertCtx was invalid");
			break;
		}
		pAlertCtx = (void*)hl::FollowRelativeAddress(hl::FollowRelativeAddress(results[0] + 0xa) + 0x3);
		if (!pAlertCtx) {
			pluginBaseState = PluginBaseState::FAILURE;
			Logger::LogString(LogLevel::Error, MAIN_INFO, "FollowAddress for AlertCtx was invalid");
			break;
		}
		Logger::LogString(LogLevel::Info, MAIN_INFO, "GameThread Hook addr: " + ToHex((uintptr_t)pAlertCtx));
	}
	case PluginBaseState::SCANNER_FINISHED: {
		m_hkAlertCtx = m_hooker.hookVT(*(uintptr_t*)pAlertCtx, 0, (uintptr_t)hkGameThread);
		if (m_hkAlertCtx) {
			pluginBaseState = PluginBaseState::THREAD_INITIALIZED;
		}
		break;
	}
	case PluginBaseState::THREAD_INITIALIZED: {
		ttq = new ThreadTaskQueue();
		pluginBaseState = PluginBaseState::INITIALIZED;
	}
	default:
		break;
	}

}

bool PluginBase::CheckKeyBinds() {
	if (_frameDownKeys.size() == 0) return false;
	bool found = false;
	for (auto iterator = keyBinds.begin(), end = keyBinds.end(); iterator != end; ++iterator) {
		KeyBindData* keyBind = *iterator;
		 if (keyBind->keys.size()>0 && KeysDown(keyBind->keys)) {
			 keyBind->func();
			 found = true;
		 }
	}
	return found;
}

InventoryData PluginBase::GetInventory(uint* updateIndex)
{
	if (updateIndex != NULL) *updateIndex = inventoryUpdateIndex;
	return inventory;
}

void PluginBase::SetInventory(InventoryData data)
{
	inventoryUpdateIndex++;
	if (inventoryUpdateIndex == 0) inventoryUpdateIndex++;
	inventory = data;
}

void PluginBase::SetHoveredItem(ItemData* data)
{
	delete hoveredItem;
	hoveredItem = data;
}

ItemData* PluginBase::GetHoveredItem()
{
	return hoveredItem;
}

bool PluginBase::HasHoveredItem()
{
	return (hoveredItem && hoveredItem->id != 0);
}

void PluginBase::RegisterKeyBind(KeyBindData* keybind)
{
	keyBinds.push_back(keybind);
	//std::sort(keyBinds.begin(), keyBinds.end());
}

void PluginBase::UnregisterKeyBind(KeyBindData* keybind)
{
	//TODO	
}

void PluginBase::Render()
{
#ifdef _DEBUG
	if (ImGui::Begin("IncQol-Debug", 0, ImGuiWindowFlags_AlwaysAutoResize)) {
		RenderReadonlyValue("GlobalCtx", currentPointers.ctx);
		ImGui::Separator();
		RenderReadonlyValue("CharCtx", currentPointers.charctx);
		RenderReadonlyValue("Player", currentPointers.player);
		RenderReadonlyValue("Inventory", currentPointers.inventory);
		ImGui::Separator();
		RenderReadonlyValue("GuildCtx", currentPointers.guildctx);
		RenderReadonlyValue("GuildInv", currentPointers.guildInv);
		ImGui::Separator();
		RenderReadonlyValue("Mouse Ptr", currentPointers.mouseFocusBase);
		RenderReadonlyValue("Hov. Element", currentPointers.hoveredElement);
		RenderReadonlyValue("Param", currentPointers.elementParam);
		RenderReadonlyValue("Obj", currentPointers.objOnElement);
		RenderReadonlyValue("LocationPtr", currentPointers.LocationPtr);
		RenderReadonlyValue("ItemPtr", currentPointers.itemPtr);
		ImGui::Separator();
		if (currentPointers.itemPtr != 0) {
			ImGui::Text("Item");
			RenderReadonlyValue("Name", currentPointers.hoveredItemData->name);
			RenderReadonlyValue("ID", currentPointers.hoveredItemData->id);
			RenderReadonlyValue("ItemType", currentPointers.hoveredItemData->itemtype);
			RenderReadonlyValue("Rarity", currentPointers.hoveredItemData->rarity);
			RenderReadonlyValue("Level", currentPointers.hoveredItemData->level);
			RenderReadonlyValue("Sellable?", currentPointers.hoveredItemData->sellable);
		}
	}

	ImGui::End();
#endif
	for (std::vector<Plugin*>::iterator it = loadedPlugins.begin(); it != loadedPlugins.end(); ++it) {
		Plugin* plugin = *it;
		plugin->Render();
	}

	if (optionWindow->Begin()) {
		if (pluginBaseState == PluginBaseState::FAILURE) {
			ImGui::PushStyleColor(ImGuiCol_Text, Addon::Colors[AddonColor_NegativeText]);
			ImGui::Text("APPLICATION IS NOT FUNCTIONING CORRECTLY, UPDATE REQUIRED");
			ImGui::PopStyleColor();
		}
		if (RenderInputText("chainload dll", chainLoad, 64, 130)) {
			Config::SaveText(MAIN_INFO, "chainload", chainLoad.c_str());
			Config::Save();
		}
		for (std::vector<Plugin*>::iterator it = loadedPlugins.begin(); it != loadedPlugins.end(); ++it) {
			Plugin* plugin = *it;
			plugin->RenderOptions();
		}
		if (ImGui::CollapsingHeader("Keybinds")) {
			RenderKeyBinds();
		}
		if (ImGui::CollapsingHeader("Addon Colors")) {
			RenderColors("addon", AddonColor_COUNT, Addon::GetStyleColorName, Addon::Colors);
		}
		if (ImGui::CollapsingHeader("ImGui Colors")) {
			RenderColors("imgui", ImGuiCol_COUNT, ImGui::GetStyleColorName, ImGui::GetStyle().Colors);
		}
		optionWindow->End();
	}
}

void PluginBase::LoadColors(int size, std::function<const char*(int)> nameFunc, ImVec4* colors) {
	for (int i = 0; i < size; i++) {
		const char* name = nameFunc(i);
		const char* value = Config::LoadText("Colors", name, '\0');
		if (value == '\0') continue;
		std::vector<float> result;
		std::stringstream ss(value);
		while (ss.good())
		{
			std::string substr;
			std::getline(ss, substr, ',');
			if (substr.size() > 0) {
				int val = std::stoi(substr);
				CLAMP(val, 0, 255);
				result.push_back(val / 255.0f);
			}
		}
		if (result.size() == 4) {
			colors[i] = ImVec4(result[0], result[1], result[2], result[3]);
		}
	}
}

void PluginBase::RenderColors(const char* id,int size, std::function<const char*(int)> nameFunc,ImVec4* colors) {

	ImVec2 padding = ImGui::GetStyle().WindowPadding;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::BeginChild(std::string("##Child").append(id).c_str(),
		ImVec2(0, CLAMP(size * ImGui::GetItemsLineHeightWithSpacing(), 0.0f, 300.0f)
		), false);
	for (int i = 0; i < size; i++)
	{
		const char* name = nameFunc(i);
		ImGui::PushID(i);
		ImGui::BeginChild(std::string("##clip").append(id).c_str(), ImVec2(115, ImGui::GetItemsLineHeightWithSpacing() - ImGui::GetStyle().ItemSpacing.y), false, ImGuiWindowFlags_NoInputs);
		ImGui::TextUnformatted(name);
		ImGui::EndChild();
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::TextUnformatted(name);
			ImGui::EndTooltip();
		}
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, padding);
		ImGui::SameLine(130-padding.x);
		ImGui::PushItemWidth(-1);
		if (ImGui::ColorEdit4(std::string("##colors").append(id).c_str(), (float*)&colors[i], ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview)) {
			Config::SaveText("Colors", name, std::to_string((int)(colors[i].x*255))
				.append("," + std::to_string((int)(colors[i].y*255)))
				.append("," + std::to_string((int)(colors[i].z*255)))
				.append("," + std::to_string((int)(colors[i].w*255))).c_str());
			Config::Save();
		}
		ImGui::PopStyleVar(1);
		ImGui::PopItemWidth();
		ImGui::PopID();
	}
	ImGui::EndChild();
	ImGui::PopStyleVar(1);
}

void PluginBase::RenderKeyBinds()
{
	/*const char* lastPlugin = nullptr;
	for (auto iterator = keyBinds.begin(), end = keyBinds.end(); iterator != end; ++iterator) {
		KeyBindData* keyBind = *iterator;
		if (!lastPlugin || lastPlugin != keyBind->plugin) {
			if (lastPlugin) {
				ImGui::Separator();
			}
			lastPlugin = keyBind->plugin;
			ImGui::Text(lastPlugin);
		}
		ImGui::Text("%s", keyBind->name);
		ImGui::SameLine(130);
		if (KeybindText("##" + std::string(keyBind->plugin) + std::string(keyBind->name), keyBind)) {
			Config::SaveKeyBinds(keyBind->plugin, keyBind->name, keyBind->keys);
			Config::Save();
		}
	}*/
	for (auto iterator = keyBinds.begin(), end = keyBinds.end(); iterator != end; ++iterator) {
		KeyBindData* keyBind = *iterator;
		ImGui::Text("%s", keyBind->name);
		ImGui::SameLine(130);
		if (KeybindText("##" + std::string(keyBind->plugin) + std::string(keyBind->name), keyBind)) {
			Config::SaveKeyBinds(keyBind->plugin, keyBind->name, keyBind->keys);
			Config::Save();
		}
	}
}

void PluginBase::AddDecodeID(uintptr_t key, std::string value)
{
	decodeIDs[key] = value;
}

void PluginBase::ProcessTask(Task * task)
{
	ttq->addTask(task);
}

const hl::IHook* PluginBase::GetAlertHook()
{
	return m_hkAlertCtx;
}

std::mutex* PluginBase::GetDataMutex()
{
	return &m_gameDataMutex;
}
std::string PluginBase::GetItemInfoUrl()
{
	return configItemsUrl;
}
std::string PluginBase::GetPricesUrl()
{
	return configPricesUrl;
}
void PluginBase::ReadItemBase(ItemData** data, hl::ForeignClass pBase) {
	uint id = pBase.get<uint>(0x28);
	ItemData* savedData = ItemData::GetData(id);
	if (!savedData) {
		savedData = new ItemData();
		savedData->id = id;
		ItemData::AddData(savedData);
	}
	*data = savedData;
	savedData->pItemData = pBase;
	savedData->level = pBase.get<int>(0x74);
	savedData->rarity = (ItemRarity)pBase.get<int>(0x60);
	savedData->sellable = !(pBase.get<byte>(0x39) & 0x40);
	savedData->pExtendedType = pBase.get<void*>(0x30);
	savedData->name = std::string("Item ").append(std::to_string(savedData->id));
	if (savedData->sellable) {
		savedData->sellable = (pBase.get<byte>(0x88) > 0x0 || pBase.get<byte>(0x4c) > 0x0);
	}
	savedData->itemtype = (ItemType)pBase.get<int>(0x2C);
	//PluginBase::GetInstance()->ProcessTask(new RequestTradingpostTask(savedData));
	if (!GetCodedTextFromHashId || !DecodeText) return;
	uint hashId = pBase.get<uint>(0x80);
	/*if (hashId == 0) {
		hl::ForeignClass hash = pBase.get<void*>(0xA8);
		hashId = hash.get<uint>(0x58);
	}*/
	if (hashId == 0) return;
	if (decodeIDs.find(hashId) == decodeIDs.end()) {
		uintptr_t* d = (uintptr_t*)GetCodedTextFromHashId(hashId, 0);
		DecodeText(d, cbDecodeText, hashId);
	}
	savedData->name = decodeIDs[hashId];
}
void PluginBase::ReadItemData(ItemStackData* data, hl::ForeignClass pBase) {
	if (!pBase) return;
	hl::ForeignClass itemPtr = pBase.call<void*>(0x20);
	if (!itemPtr) return;
	data->pItem = pBase;
	data->count = pBase.call<int>(0x78);
	data->accountBound = (pBase.get<int>(0x50) & 0x40);
	data->tradingpostSellable = !(pBase.get<int>(0x50) & 0x08);
	ReadItemBase(&data->itemData, itemPtr);
}
void PluginBase::SetupMisc() {
	currentPointers.elementParam = 0;
	currentPointers.LocationPtr = 0;
	currentPointers.itemPtr = 0;
	currentPointers.objOnElement = 0;
	currentPointers.hoveredElement = 0;
	if (!currentPointers.mouseFocusBase) return;
	currentPointers.hoveredElement = *(uintptr_t*)currentPointers.mouseFocusBase;
	hl::ForeignClass element = (void*)currentPointers.hoveredElement;
	if (!element) return;
	currentPointers.elementParam = element.get<uint>(0x68);
	hl::ForeignClass objOnElement = element.get<void*>(0x228);
	currentPointers.objOnElement = (uintptr_t)objOnElement.data();
	if (!objOnElement) return;
	hl::ForeignClass inventory = objOnElement.get<void*>(0xB8);
	if (inventory && (uintptr_t)inventory.data() == currentPointers.inventory) {//its a normal slot in the inventory/shared/bank
		currentPointers.LocationPtr = (uintptr_t)inventory.data();
		hl::ForeignClass item = objOnElement.get<void*>(0xC8);
		if (item) {
			ItemStackData data;
			ReadItemData(&data, item);
			currentPointers.itemPtr = (uintptr_t)data.itemData->pItemData.data();
			currentPointers.hoveredItemData = data.itemData;
		}
		return;
	}
	hl::ForeignClass itemData = objOnElement.get<void*>(0x70);
	hl::ForeignClass itemCollection = objOnElement.get<void*>(0x78);//collection of materials

	[&] { __try {
		if (itemData && itemCollection && (uintptr_t)itemCollection.data() + 0x60 == itemCollection.get<uintptr_t>(0x30)) {
			currentPointers.LocationPtr = (uintptr_t)itemCollection.data();
			itemData = itemData.get<void*>(0x0);
			if (itemData) {
				currentPointers.itemPtr = (uintptr_t)itemData.data();
				ReadItemBase(&currentPointers.hoveredItemData, itemData);
				return;
			}
		}
	}
	__except (IGNORE_EXCEPTION) {
	}}();
	itemCollection = objOnElement.get<void*>(0x80);
	if (itemCollection && ((uintptr_t)itemCollection.data() == currentPointers.guildInv)) {//guild slots
		currentPointers.LocationPtr = (uintptr_t)itemCollection.data();
		itemData = objOnElement.get<void*>(0x98);
		if (itemData) {
			currentPointers.itemPtr = (uintptr_t)itemData.data();
			ReadItemBase(&currentPointers.hoveredItemData, itemData);
			return;
		}
	}
}
void PluginBase::SetupGuild() {
	currentPointers.ctx = 0;
	currentPointers.guildctx = 0;
	currentPointers.guildInv = 0;
	hl::ForeignClass ctx = pCtx;
	currentPointers.ctx = (uintptr_t)ctx.data();
	if (!ctx) return;

	hl::ForeignClass guildctx = ctx.get<void*>(0x148);
	currentPointers.guildctx = (uintptr_t)guildctx.data();
	if (!guildctx) return;

	hl::ForeignClass guildInv = guildctx.get<void*>(0x68);
	if (guildInv) {
		currentPointers.guildInv = (uintptr_t)guildInv.data();
	}
}
void PluginBase::SetupPlayer() {
	currentPointers.ctx = 0;
	currentPointers.charctx = 0;
	currentPointers.player = 0;
	currentPointers.inventory = 0;
	hl::ForeignClass ctx = pCtx;
	currentPointers.ctx = (uintptr_t)ctx.data();
	if (!ctx) return;

	hl::ForeignClass charctx = ctx.get<void*>(0x90);
	currentPointers.charctx = (uintptr_t)charctx.data();
	if (!charctx) return;

	hl::ForeignClass player = charctx.get<void*>(0x90);
	currentPointers.player = (uintptr_t)player.data();
	if (!player) {
		if (inventory.itemStackDatas.size()>0)
			SetInventory(InventoryData());
		return;
	}

	hl::ForeignClass inventoryC = player.call<void*>(0x80);
	currentPointers.inventory = (uintptr_t)inventoryC.data();
	if (!inventoryC) return;
	InventoryData oldData = inventory;
	bool changed = false;
	InventoryData inventoryData = InventoryData();
	inventoryData.size = inventoryC.call<int>(0x1A0);//get count slots for bagcount
	inventoryData.bagCount = inventoryC.call<int>(0x118);
	inventoryData.slotsPerBag = inventoryData.size / inventoryData.bagCount;
	std::vector<BagData> oldBagData = oldData.bagDatas;
	std::vector<BagData> bagData;
	for (int i = 0; i < inventoryData.bagCount; i++) {
		hl::ForeignClass itemStackPtr = inventoryC.call<void*>(0x108, i);
		BagData data;
		ReadItemData(&data, itemStackPtr);
		if (data.itemData->pExtendedType) {
			data.bagSize = data.itemData->pExtendedType.get<int>(0x28);
			data.noSellOrSort = data.itemData->pExtendedType.get<bool>(0x0);
		}
		bagData.push_back(data);
		if (changed) continue;
		if (bagData.size() > oldBagData.size()) {
			changed = true;
		}
		if (changed) continue;
		ItemStackData old = oldBagData[bagData.size() - 1];
		if (data.pItem != old.pItem || data.slot != old.slot) {
			changed = true;
		}
	}
	inventoryData.bagDatas = bagData;
	std::vector<ItemStackData> oldItemData = oldData.itemStackDatas;
	std::vector<ItemStackData> itemData;
	int slotCount = inventoryC.get<int>(0xD4);//unsafe but nothing better
	for (int i = 0; i < slotCount; i++) {
		hl::ForeignClass itemStackPtr = inventoryC.call<void*>(0x168, i);
		ItemStackData data;
		ReadItemData(&data, itemStackPtr);
		int bagSlot = i / inventoryData.slotsPerBag;
		if (bagSlot < bagData.size()) {
			BagData bag = bagData[bagSlot];
			if (data.itemData && bag.pItem && data.itemData->sellable)  data.itemData->sellable = !bag.noSellOrSort;
		}
		data.slot = i;
		itemData.push_back(data);
		if (changed) continue;
		if (itemData.size() > oldItemData.size()) {
			changed = true;
		}
		if (changed) continue;
		ItemStackData old = oldItemData[itemData.size() - 1];
		if (data.pItem != old.pItem || data.slot != old.slot) {
			changed = true;
		}
	}
	if (itemData.size() != oldItemData.size()) {
		changed = true;
	}
	inventoryData.itemStackDatas = itemData;
	if (changed)
		SetInventory(inventoryData);
}
void PluginBase::GameHook()
{
	SetupContext();


	for (std::vector<Plugin*>::iterator it = loadedPlugins.begin(); it != loadedPlugins.end(); ++it) {
		Plugin* plugin = *it;
		plugin->PluginMain();
	}
}

void __fastcall hkGameThread(uintptr_t pInst, int, int frame_time)
{
	PluginBase* base = PluginBase::GetInstance();
	static auto orgFunc = ((void(__thiscall*)(uintptr_t, int))base->GetAlertHook()->getLocation());

	std::lock_guard<std::mutex> lock(*base->GetDataMutex());

	[&] {
		__try {
			base->GameHook();
		}
		__except (ADDON_EXCEPTION("[hkGameThread] Exception in game thread")) {
			;
		}
	}();

	orgFunc(pInst, frame_time);
}
static void __fastcall cbDecodeText(uintptr_t* ctx, wchar_t* decodedText)
{
	PluginBase* base = PluginBase::GetInstance();
	if (ctx && decodedText && decodedText[0] != 0) {
		uintptr_t v = (uintptr_t)ctx;
		base->AddDecodeID(v,ws2s(std::wstring(decodedText)));
	}
}
