#include "PluginBase.h"
#include <iterator>
#include <sstream>
#include "Plugin.h"
#include "RequestTradingpostTask.h"
#include "Window.h"
#include "Config.h"
#include "AddonColors.h"
#include "Logger.h"
#include "windows.h"
#include "psapi.h"

void __fastcall hkGameThread(uintptr_t, int, int);
void __fastcall cbDecodeText(std::string* ctx, wchar_t* decodedText);
void __fastcall cbDecodeTextStripColor(std::string* ctx, wchar_t* decodedText);

int PluginBase::tpUpdateInterval = 6;
bool PluginBase::tpApiEnabled = false;

KeyBindData* PluginBase::keybindVisual;


inline void Symbol(ULONG64 dwAddr)
{
	typedef struct _IMAGEHLP_SYMBOL64_L {
		DWORD   SizeOfStruct;
		DWORD64 Address;
		DWORD   Size;
		DWORD   Flags;
		DWORD   MaxNameLength;
		CHAR    Name[1];
	} IMAGEHLP_SYMBOL64_L, *PIMAGEHLP_SYMBOL64_L;

	typedef struct _IMAGEHLP_LINE64 {
		DWORD   SizeOfStruct;
		PVOID   Key;
		DWORD   LineNumber;
		PTSTR   FileName;
		DWORD64 Address;
	} IMAGEHLP_LINE64, *PIMAGEHLP_LINE64;

	typedef BOOL(__stdcall *pfnSymInitialize) (HANDLE hProcess,
		PCSTR UserSearchPath, BOOL fInvadeProcess);
	typedef BOOL(__stdcall *pfnSymGetSymFromAddr64) (HANDLE hProcess,
		DWORD64 qwAddr, PDWORD64 pdwDisplacement, PIMAGEHLP_SYMBOL64_L  Symbol);
	typedef BOOL(__stdcall *pfnSymGetLineFromAddr64) (HANDLE hProcess,
		DWORD64 qwAddr, PDWORD64 pdwDisplacement, PIMAGEHLP_LINE64  Symbol);

	static BOOL	bSymLoad;
	static char symName[1024];
	static IMAGEHLP_SYMBOL64_L *pSym = NULL;
	static IMAGEHLP_LINE64 pLine;
	static pfnSymInitialize SymInit;
	static pfnSymGetSymFromAddr64 SymName;
	static pfnSymGetLineFromAddr64 SymLine;

	if (!bSymLoad)
	{
		HMODULE hMod;
		if ((hMod = LoadLibrary(L"Dbghelp.dll")) &&
			(SymInit = (pfnSymInitialize)GetProcAddress(hMod, "SymInitialize")) &&
			(SymName = (pfnSymGetSymFromAddr64)GetProcAddress(hMod, "SymGetSymFromAddr64")) &&
			(SymLine = (pfnSymGetLineFromAddr64)GetProcAddress(hMod, "SymGetLineFromAddr64")))
		{
			bSymLoad = SymInit(GetCurrentProcess(), NULL, true);
			pSym = (IMAGEHLP_SYMBOL64_L *)malloc(sizeof(IMAGEHLP_SYMBOL64_L) + 2000);
			//pLine = (IMAGEHLP_LINE64*)malloc(sizeof(IMAGEHLP_LINE64));
		}
	}

	if (bSymLoad)
	{
		DWORD64  dwDisplacement = 0;
		pSym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64_L);
		/*pLine->SizeOfStruct = sizeof(IMAGEHLP_LINE64);
		pLine->FileName = new WCHAR[2000];*/
		pSym->MaxNameLength = 2000;
		if (SymName(GetCurrentProcess(), dwAddr, &dwDisplacement, pSym))
		{
			if (SymLine(GetCurrentProcess(), dwAddr, &dwDisplacement, &pLine)) {
				Logger::LogString(LogLevel::Critical, "Exception", "Func: " + std::string(pSym->Name) + " Line: " + std::to_string(pLine.LineNumber));
				return;
			}
			Logger::LogString(LogLevel::Critical, "Exception", "Func: " + std::string(pSym->Name));
		}
	}
}

bool PluginBase::IsCloseWindowBindDown() {
	if (!escCloseEnabled) return false;
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
void PluginBase::PushKeys(std::list<EventKey> eventKeys)
{
	_frameDownKeys.clear();
	frameKeysOneUp = false;
	bool escape = false;
	lastKeys = std::set<uint>(_downKeys.begin(), _downKeys.end());
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
				if (k.vk != VK_LBUTTON)
					frameKeysOneUp = true;
			}
		}
	}
	for (auto i = _downKeys.begin(); i != _downKeys.end();) {
		if (!GetAsyncKeyState(*i)) {
			if (*i != VK_LBUTTON)
				frameKeysOneUp = true;
			i = _downKeys.erase(i);
		}
		else {
			i++;
		}
	}
	lastKeys.erase(VK_LBUTTON);
}

bool PluginBase::InputKeyBind() {
	if (keybindVisual) {
		if (_frameDownKeys.find(VK_ESCAPE) != _frameDownKeys.end()) {
			keybindVisual->isSetMode = false;
			keybindVisual = nullptr;
			return true;
		}
		keybindVisual->isSetMode = !frameKeysOneUp;
		std::set<uint> keys = std::set<uint>(lastKeys.begin(), lastKeys.end());
		keybindVisual->newKeys = keys;
		if (frameKeysOneUp) {
			keybindVisual->isSetMode = false;
			keybindVisual->finished = true;
			keybindVisual = nullptr;
		}
		return true;
	}
	return false;
}

bool PluginBase::KeybindText(std::string suffix,KeyBindData* data) {
	std::string text = "";
	bool res = false;
	if (data->finished) {
		data->finished = false;
		data->keys = data->newKeys;
		res = true;
	}
	std::set<uint> keys = data->keys;
	if (keybindVisual == data) {
		keys = data->newKeys;
	}
	for (auto it = keys.begin(), end = keys.end(); it != end;) {
		text.append(GetKeyName((*it)));
		++it;
		if (it != end) {
			text.append(" + ");
		}
	}
	bool noKeyBindMode = PluginBase::GetInstance()->NoKeyBindMode();
	float buttonWidth = 50.f;
	if (noKeyBindMode) {
		ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() - buttonWidth - ImGui::GetStyle().ColumnsMinSpacing);
	}
	else {
		ImGui::PushItemWidth(-1);
	}
	int popcount = 0;
	if (data->isSetMode) {
		popcount = 2;
		ImGui::PushStyleColor(ImGuiCol_FrameBg, Addon::Colors[AddonColor_FrameBgHighlighted]);
		ImGui::PushStyleColor(ImGuiCol_Text, Addon::Colors[AddonColor_TextHighlighted]);
	}
	ImGui::InputText(suffix.c_str(),&text[0],text.size(),ImGuiInputTextFlags_ReadOnly);
	ImGui::PopStyleColor(popcount);
	ImGui::PopItemWidth();
	if (noKeyBindMode) {
		ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
		if (!data->isSetMode && ImGui::Button(("Set" + suffix).c_str(), ImVec2(buttonWidth, 0)))
		{
			data->isSetMode = true;
			if (keybindVisual) {
				keybindVisual->isSetMode = false;
				keybindVisual->newKeys = std::set<uint>();
			}
			keybindVisual = data;
			keybindVisual->newKeys = std::set<uint>();
			return false;
		}
		else if (data->isSetMode && ImGui::Button(("Clear" + suffix).c_str(), ImVec2(buttonWidth, 0)))
		{
			keybindVisual = nullptr;
			data->isSetMode = false;
			data->keys = std::set<uint>();
			data->newKeys = std::set<uint>();
			return true;
		}
	}
	return res;
}

void PluginBase::Init() {
	ReloadConfig();
	Logger::LogString(LogLevel::Debug, MAIN_INFO, "Creating option window");
	char* buf = new char[128];
	sprintf(buf, "Options - IncQol %s ###Options", VERSION);
	optionWindow = new Window(buf);
	optionWindow->SetMinSize(ImVec2(300, 300));
	AddWindow(optionWindow);
	Logger::LogString(LogLevel::Debug, MAIN_INFO, "Creating keybind for option window");
	openOptions = new KeyBindData();
	openOptions->plugin = MAIN_INFO;
	openOptions->name = "OpenOptions";
	openOptions->keys = Config::LoadKeyBinds(openOptions->plugin, openOptions->name, { VK_MENU,VK_SHIFT,'1' });
	openOptions->func = std::bind(&Window::ChangeState, optionWindow);
	RegisterKeyBind(openOptions);
	Logger::LogString(LogLevel::Debug, MAIN_INFO, "Registering keybind for option window");
	interactionKeyBind = new KeyBindData();
	interactionKeyBind->plugin = MAIN_INFO;
	interactionKeyBind->name = "InteractWithGUI";
	interactionKeyBind->keys = Config::LoadKeyBinds(interactionKeyBind->plugin, interactionKeyBind->name, { VK_MENU,VK_SHIFT });
	requiresInteraction = Config::LoadBool(MAIN_INFO, REQUIRES_INTERACTION_KEYBIND, false);

	chainLoad = Config::LoadText(MAIN_INFO, CHAINLOAD_DLL, "d3d9_incqol_chain.dll");
	tpApiEnabled = Config::LoadBool(MAIN_INFO, ENABLE_TP_API, tpApiEnabled);
	escCloseEnabled = Config::LoadBool(MAIN_INFO, ENABLE_ESC_WINDOW_CLOSE, escCloseEnabled);
	tpUpdateInterval = (int)Config::LoadLong(MAIN_INFO, TP_UPDATE_INTERVAL, tpUpdateInterval);
	tpUpdateInterval = CLAMP(tpUpdateInterval, MIN_TP_INTERVAL, MAX_TP_INTERVAL);
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
	configItemsUrl = Config::LoadText(MAIN_INFO, URL_ITEMS_INFO, "https://api.guildwars2.com/v2/items/");
	configItemsUrl = configItemsUrl.substr(0, min(configItemsUrl.size(), 127));
	configPricesUrl = Config::LoadText(MAIN_INFO, URL_LISTINGS_INFO, "https://api.guildwars2.com/v2/commerce/listings/");
	configPricesUrl = configPricesUrl.substr(0, min(configPricesUrl.size(), 127));
}

void PluginBase::SetupContext()
{
	pCtx = GetContext();

	SetupPlayer();
	SetupGuild();
	SetupMisc();

	if (!currentPointers.itemPtr) {
		SetHoveredItem(nullptr);
	}
	else {
		currentPointers.hoveredItemData = hoveredItem;
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
			Logger::LogString(LogLevel::Error, MAIN_INFO, "Pattern for Mouse Hover is invalid, no Mouse Hover available");
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
		GetCodedItemName = hl::FindPattern("57 41 54 41 55 41 56 41 57 48 83 EC ?? 4D 8B E1 4D 8B E8 4C 8B FA 48 8B F1 48 85 C9 75");
		pluginBaseState = PluginBaseState::DECODE_TEXT_FINISHED;
		if (DecodeText.data()) {
			DecodeText = (uintptr_t)DecodeText.data() - 0x14;
			Logger::LogString(LogLevel::Info, MAIN_INFO, "DecodeText addr: " + ToHex((uintptr_t)DecodeText.data()));
		}
		else {
			Logger::LogString(LogLevel::Info, MAIN_INFO, "Pattern for DecodeText is invalid, no DecodeText available");
		}
		if (GetCodedItemName.data()) {
			GetCodedItemName = (uintptr_t)GetCodedItemName.data() - 0xf;
			Logger::LogString(LogLevel::Info, MAIN_INFO, "GetCodedItemName addr: " + ToHex((uintptr_t)GetCodedItemName.data()));
		}
		else {
			Logger::LogString(LogLevel::Info, MAIN_INFO, "Pattern for GetCodedItemName is invalid, no GetCodedItemName available");
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

		if (results.size()==0 || !results[0]) {
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
		else {
			pluginBaseState = PluginBaseState::FAILURE;
		}
		break;
	}
	case PluginBaseState::THREAD_INITIALIZED: {
		ttq = new ThreadTaskQueue(8);
		pluginBaseState = PluginBaseState::INITIALIZED;
	}
	default:
		break;
	}

}

bool PluginBase::NoKeyBindMode() {
	return (!requiresInteraction || interactionKeyBind->keys.size() == 0);
}

bool PluginBase::CheckInteractionKeyBind() {
	return (NoKeyBindMode() || std::includes(_downKeys.begin(), _downKeys.end(), interactionKeyBind->keys.begin(), interactionKeyBind->keys.end())); // _downKeys == keys;);
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

InventoryData* PluginBase::GetInventory(uint* updateIndex)
{
	if (updateIndex != NULL) *updateIndex = inventoryUpdateIndex;
	return inventory;
}

void PluginBase::SetInventory(InventoryData* data)
{
	inventoryUpdateIndex++;
	if (inventoryUpdateIndex == 0) inventoryUpdateIndex++;
	InventoryData* old = inventory;
	inventory = data;
	delete old;
}

void PluginBase::SetHoveredItem(ItemData* data)
{
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
		RenderReadonlyValue("Cur. Tick",std::to_string(GetCurrentTime()));
		RenderReadonlyValue("task count", std::to_string(test));
		RenderReadonlyValue("GlobalCtx", currentPointers.ctx);
		ImGui::Separator();
		RenderReadonlyValue("CharCtx", currentPointers.charctx);
		RenderReadonlyValue("Player", currentPointers.player);
		RenderReadonlyValue("Inventory", currentPointers.inventory);
		RenderReadonlyValue("Inv. Update ID", inventoryUpdateIndex);
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
		if (currentPointers.itemPtr != 0 && currentPointers.hoveredItemData) {
			ImGui::Text("Item");
			RenderReadonlyValue("Name", currentPointers.hoveredItemData->name);
			RenderReadonlyValue("ID", currentPointers.hoveredItemData->id);
			RenderReadonlyValue("ItemType", currentPointers.hoveredItemData->itemtype);
			RenderReadonlyValue("Rarity", currentPointers.hoveredItemData->rarity);
			RenderReadonlyValue("Level", currentPointers.hoveredItemData->level);
			RenderReadonlyValue("Sellable?", currentPointers.hoveredItemData->sellable);
			RenderReadonlyValue("lastUpd", std::to_string(currentPointers.hoveredItemData->lastTradingPostUpdate));
			RenderReadonlyValue("Vendor", std::to_string(currentPointers.hoveredItemData->vendorValue));
			RenderReadonlyValue("Buy", std::to_string(currentPointers.hoveredItemData->buyTradingPost));
			RenderReadonlyValue("Sell", std::to_string(currentPointers.hoveredItemData->sellTradingPost));
		}
	}

	ImGui::End();
#endif
	for (std::vector<Plugin*>::iterator it = loadedPlugins.begin(); it != loadedPlugins.end(); ++it) {
		Plugin* plugin = *it;
		plugin->PreRender();
	}

	if (optionWindow->Begin()) {
		if (pluginBaseState == PluginBaseState::FAILURE) {
			ImGui::PushStyleColor(ImGuiCol_Text, Addon::Colors[AddonColor_NegativeText]);
			ImGui::Text("APPLICATION IS NOT FUNCTIONING CORRECTLY, UPDATE REQUIRED");
			ImGui::PopStyleColor();
		}
		if (ImGui::CollapsingHeader(MAIN_INFO)) {
			if (RenderInputText("chainload dll", chainLoad, 64, std::vector<TooltipColor> { TooltipColor("File name for chainloading .dll in the bin64 folder."),TooltipColor("\n"),TooltipColor("Requires a restart.")})) {
				Config::SaveText(MAIN_INFO, CHAINLOAD_DLL, chainLoad.c_str());
				Config::Save();
			}
			RenderKeyBind(openOptions, std::vector<TooltipColor> { TooltipColor("Shows / Hides the options window.")});
			if (RenderCheckbox("Enable ESC Closing", &escCloseEnabled, std::vector<TooltipColor> { TooltipColor("When enabled, you can close any focused window with ESCAPE.")})) {
				Config::SaveBool(MAIN_INFO, ENABLE_ESC_WINDOW_CLOSE, escCloseEnabled);
				Config::Save();
			}
			if (RenderCheckbox("Require keybind", &requiresInteraction, std::vector<TooltipColor> {TooltipColor("Not activated : The GUI will always accept input."), TooltipColor("Activated : The GUI will only accept input (mouse clicks etc.) when InteractWithGUI keybind is hold.")})) {
				Config::SaveBool(MAIN_INFO, REQUIRES_INTERACTION_KEYBIND, requiresInteraction);
				Config::Save();
			}
			RenderKeyBind(interactionKeyBind, std::vector<TooltipColor> { TooltipColor("Keybind for interaction with the GUI (if enabled).")});
			if (RenderCheckbox("Enable TP api", &tpApiEnabled, std::vector<TooltipColor> { TooltipColor("Enables the Trading Post API used to generate buy and sell values per item.")})) {
				Config::SaveBool(MAIN_INFO, ENABLE_TP_API, tpApiEnabled);
				Config::Save();
			}
			if (tpApiEnabled) {
				if (RenderSliderInt("TP upd. interval", &tpUpdateInterval, MIN_TP_INTERVAL, MAX_TP_INTERVAL, std::vector<TooltipColor> { TooltipColor("Trading Post update interval in minutes, how often the addon wants to request buy / sell values"), TooltipColor("\n"), TooltipColor("Technically the minimum update interval is 5 minutes, because thats the cache limit of the API.")})) {
					Config::SaveLong(MAIN_INFO, TP_UPDATE_INTERVAL, tpUpdateInterval);
					Config::Save();
				}
				if (RenderInputText("URL items info", configItemsUrl, 128, std::vector<TooltipColor> { TooltipColor("URL for the items info."), TooltipColor("\n"), TooltipColor("Do NOT change, if you do not have to.",Addon::Colors[AddonColor_NegativeText])})) {
					Config::SaveText(MAIN_INFO, URL_ITEMS_INFO, configItemsUrl.c_str());
					Config::Save();
				}
				if (RenderInputText("URL listings info", configPricesUrl, 128,std::vector<TooltipColor> { TooltipColor("URL for the listings(buy/sell)."), TooltipColor("\n"), TooltipColor("Do NOT change, if you do not have to.", Addon::Colors[AddonColor_NegativeText])})) {
					Config::SaveText(MAIN_INFO, URL_LISTINGS_INFO, configPricesUrl.c_str());
					Config::Save();
				}
			}
		}
		for (std::vector<Plugin*>::iterator it = loadedPlugins.begin(); it != loadedPlugins.end(); ++it) {
			Plugin* plugin = *it;
			plugin->RenderOptions();
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
				try {
					int val = std::stoi(substr);
					CLAMP(val, 0, 255);
					result.push_back(val / 255.0f);
				}
				catch (...) {
					//ignore wrong input
				}
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
		ImVec2(0, CLAMP(size * ImGui::GetTextLineHeightWithSpacing(), 0.0f, 300.0f)
		), false);
	for (int i = 0; i < size; i++)
	{
		const char* name = nameFunc(i);
		ImGui::PushID(i);
		ImGui::BeginChild(std::string("##clip").append(id).c_str(), ImVec2(TEXTSPACE-padding.x, ImGui::GetTextLineHeightWithSpacing()), false, ImGuiWindowFlags_NoInputs);
		ImGui::Text(name);
		if (ImGui::IsItemHovered()) {
			ImGui::PopStyleVar(1);
			ImGui::BeginTooltip();
			ImGui::Text(name);
			ImGui::EndTooltip();
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		}
		ImGui::EndChild();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, padding);
		ImGui::SameLine(TEXTSPACE-padding.x);
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

void PluginBase::RenderKeyBind(KeyBindData* keyBind,std::vector<TooltipColor> tooltips)
{
	if (!keyBind) return;
	RenderTextPre(keyBind->name, tooltips);
	if (KeybindText("##" + std::string(keyBind->plugin) + std::string(keyBind->name), keyBind)) {
		Config::SaveKeyBinds(keyBind->plugin, keyBind->name, keyBind->keys);
		Config::Save();
	}
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
	savedData->id = id;
	savedData->pItemData = pBase;
	savedData->level = pBase.get<int>(0x74);
	savedData->rarity = (ItemRarity)pBase.get<int>(0x60);
	savedData->sellable = !(pBase.get<byte>(0x39) & 0x40);
	savedData->pExtendedType = pBase.get<void*>(0x30);
	if (savedData->sellable) {
		savedData->sellable = (pBase.get<byte>(0x88) > 0x0 || pBase.get<byte>(0x4c) > 0x0);
	}
	savedData->itemtype = (ItemType)pBase.get<int>(0x2C);
	if (tpApiEnabled && !savedData->updateTaskActive && savedData->IsOldTradingPostData()) {
		test++;
		savedData->updateTaskActive = true;
		PluginBase::GetInstance()->ProcessTask(new RequestTradingpostTask(savedData));
	}
	if (!savedData->name.empty()) return;
	if (!GetCodedTextFromHashId || !DecodeText || !GetCodedItemName) {
		savedData->name = ("Item " + std::to_string(id)).c_str();
		return;
	}
	uint hashId = pBase.get<uint>(0x80);
	if (hashId == 0) {
		//hl::ForeignClass hash = pBase.get<void*>(0xA8);
		//hashId = hash.get<uint>(0x58);
		//uintptr_t* d = (uintptr_t*)GetCodedItemName(pBase.data(), skin,prefixname,suffixname,1,0,0)
		uintptr_t* d = (uintptr_t*)GetCodedItemName(pBase.data(), (uintptr_t)0, 0/*savedData->prefixName*/, 0/*savedData->suffixName*/, 1, (uintptr_t)0, 0);
		DecodeText(d, cbDecodeTextStripColor, &savedData->name);
	//}else if (decodeIDs.find(hashId) == decodeIDs.end()) {
	}else{
		uintptr_t* d = (uintptr_t*)GetCodedTextFromHashId(hashId, 0);
		DecodeText(d, cbDecodeText, &savedData->name);
	}
}
void PluginBase::ReadItemData(ItemStackData* data, hl::ForeignClass pBase) {
	if (!pBase) return;
	hl::ForeignClass itemPtr = pBase.call<void*>(0x20);
	if (!itemPtr) return;
	data->pItem = pBase;
	data->count = pBase.call<int>(0x78);
	data->accountBound = (pBase.get<int>(0x50) & 0x40);
	data->tradingpostSellable = !(pBase.get<int>(0x50) & 0x08);

	data->pSkin = pBase.call<void*>(0x70);

	hl::ForeignClass prepost = pBase.call<void*>(0x210);
	if (prepost) {

		data->pPrefix = prepost.call<void*>(0x0);
		hl::ForeignClass suffix = prepost.call<void*>(0x10, 1);
		if (suffix) {
			data->pSuffix = suffix.get<void*>(0x30);
		}
	}
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
	ItemData* data;
	ItemStackData stackData;
	if (inventory && (uintptr_t)inventory.data() == currentPointers.inventory) {//its a normal slot in the inventory/shared/bank
		currentPointers.LocationPtr = (uintptr_t)inventory.data();
		hl::ForeignClass item = objOnElement.get<void*>(0xC8);
		if (item) {
			ReadItemData(&stackData, item);
			currentPointers.itemPtr = (uintptr_t)stackData.itemData->pItemData.data();
			SetHoveredItem(stackData.itemData);
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
				ReadItemBase(&data, itemData);
				currentPointers.itemPtr = (uintptr_t)data->pItemData.data();
				SetHoveredItem(data);
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
			ReadItemBase(&data, itemData);
			currentPointers.itemPtr = (uintptr_t)data->pItemData.data();
			SetHoveredItem(data);
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
			if (inventory) {
				SetInventory(nullptr);
			}
			return;
		}

		hl::ForeignClass inventoryC = player.call<void*>(0x80);
		currentPointers.inventory = (uintptr_t)inventoryC.data();
		if (!inventoryC) return;
		InventoryData* oldData = inventory;
		bool changed = false;
		if (wasTPEnabled != tpApiEnabled) {
			changed = true;//make sure to update inventory after api enabling/disabling
		}
		if (hasTPUpdates) {//make sure we update the inventory after getting tp updates
			hasTPUpdates = false;
			changed = true;
		}
		wasTPEnabled = tpApiEnabled;
		InventoryData* inventoryData = new InventoryData();
		inventoryData->size = inventoryC.call<int>(0x1A0);//get count slots for bagcount
		inventoryData->bagCount = inventoryC.call<int>(0x118);
		inventoryData->slotsPerBag = inventoryData->size / inventoryData->bagCount;
		BagData** oldBagData = nullptr;
		if (oldData)
			oldBagData = oldData->bagDatas;
		BagData** bagData = new BagData*[inventoryData->bagCount];
		if (!oldData || inventoryData->bagCount != oldData->bagCount) {
			changed = true;
		}
		for (int i = 0; i < inventoryData->bagCount; i++) {
			hl::ForeignClass itemStackPtr = inventoryC.call<void*>(0x108, i);
			BagData* data = new BagData();
			bagData[i] = data;
			ReadItemData(data, itemStackPtr);
			if (data->itemData->pExtendedType) {
				data->bagSize = data->itemData->pExtendedType.get<int>(0x28);
				data->noSellOrSort = data->itemData->pExtendedType.get<bool>(0x0);
			}
			inventoryData->realSize += data->bagSize;
			if (changed) continue;
			ItemStackData* old = oldBagData[i];
			if (data->pItem != old->pItem || data->slot != old->slot) {
				changed = true;
			}
		}
		if (!oldData || inventoryData->realSize != oldData->realSize) {
			changed = true;
		}
		inventoryData->bagDatas = bagData;
		//int slotCount = inventoryC.get<int>(0xD4);//unsafe but nothing better
		ItemStackData** oldItemData = nullptr;
		if (oldData)
			oldItemData = oldData->itemStackDatas;
		ItemStackData** itemData = new ItemStackData*[inventoryData->realSize];
		int bagOffset = 0;
		int currentBagIndex = 0;
		for (int i = 0; i < inventoryData->realSize; ++i, ++currentBagIndex, ++bagOffset) {
			hl::ForeignClass itemStackPtr = inventoryC.call<void*>(0x168, bagOffset);
			ItemStackData* data = new ItemStackData();
			itemData[i] = data;
			data->slot = bagOffset;
			ReadItemData(data, itemStackPtr);
			int bagSlot = bagOffset / inventoryData->slotsPerBag;
			if (bagSlot < inventoryData->bagCount) {
				BagData* bag = bagData[bagSlot];
				if (currentBagIndex == bag->bagSize - 1) {
					bagOffset += (inventoryData->slotsPerBag - bag->bagSize);
					currentBagIndex = -1;
				}
				if (data->itemData && bag->pItem && data->itemData->sellable)  data->sellable = !bag->noSellOrSort;
			}
			if (changed) continue;
			ItemStackData* old = oldItemData[i];
			if (data->pItem != old->pItem || data->slot != old->slot) {
				changed = true;
			}
		}
		inventoryData->itemStackDatas = itemData;
		if (changed) {
			SetInventory(inventoryData);
		}
		else {
			delete inventoryData;
		}
}
void PluginBase::GameHook()
{
		SetupContext();

		for (std::vector<Plugin*>::iterator it = loadedPlugins.begin(); it != loadedPlugins.end(); ++it) {
			(*it)->PluginMain();
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
		}	
	}();

	orgFunc(pInst, frame_time);
}
static void __fastcall cbDecodeText(std::string* ctx, wchar_t* decodedText)
{
	//PluginBase* base = PluginBase::GetInstance();
	if (ctx && decodedText && decodedText[0] != 0) {
		//uintptr_t v = (uintptr_t)ctx;
		//base->AddDecodeID(v,ws2s(std::wstring(decodedText)));
		*ctx = ws2s(std::wstring(decodedText));
	}
}
static void __fastcall cbDecodeTextStripColor(std::string* ctx, wchar_t* decodedText)
{
	//PluginBase* base = PluginBase::GetInstance();
	if (ctx && decodedText && decodedText[0] != 0) {
		//uintptr_t v = (uintptr_t)ctx;
		//base->AddDecodeID(v,ws2s(std::wstring(decodedText)));
		*ctx = ws2s(std::wstring(decodedText));
		if (ctx) {
			if (ctx->find_first_of("<c=#") != std::string::npos) {
				ctx->assign(ctx->substr(11, ctx->size() - 15));
			}
		}
	}
}

