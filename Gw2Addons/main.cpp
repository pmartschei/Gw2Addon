#include "main.h"
#include "d3d9.h"
#include <imgui_impl_dx9.h>
#include "Config.h"
#include "Utility.h"
#include "FilterPlugin.h"
#include "PluginBase.h"
#include "Plugin.h"
#include "Process.h"
#include "ForeignFunction.h"
#include "hacklib\Hooker.h"
#include "hacklib\PatternScanner.h"
#include "hacklib\Memory.h"
#include "hacklib\ForeignClass.h"
#include "ItemStackData.h"
#include "Logger.h"

/*
	TODO: 
	Tradingpost values
	Filtered Items wegwerfen
*/
HWND GameWindow = 0;
WNDPROC BaseWndProc;
HMODULE OriginalD3D9 = nullptr;
IDirect3DDevice9* RealDevice = nullptr;

// Rendering
uint ScreenWidth, ScreenHeight;

Plugin* plugin =(Plugin*)new FilterPlugin();
PluginBase* pluginBase = new PluginBase();

KeyBindData* openOptions = new KeyBindData();
Window* optionWindow;
 
#define ARCH_64BIT

void* pCtx;
hl::Hooker m_hooker;
const hl::IHook *m_hkAlertCtx = nullptr;
std::mutex m_gameDataMutex;
ForeignFunction<void*> GetContext; 
ForeignFunction<void*> GetCodedTextFromHashId;
ForeignFunction<bool> DecodeText;
std::map<uintptr_t, std::string> decodeIDs;

#ifdef DEBUG
#define LOG_LVL (LogLevel::Debug)
#else
#define LOG_LVL (LogLevel::Info)
#endif

struct GamePointers {
	uintptr_t ctx;
	uintptr_t charctx;
	uintptr_t player;
	uintptr_t inventory;

	uintptr_t guildctx;
	uintptr_t guildInv;

	uintptr_t mouseFocusBase;
	uintptr_t hoveredElement;
	uint elementParam;
	uintptr_t objOnElement;
	uintptr_t LocationPtr;
	uintptr_t itemPtr;
	ItemData hoveredItemData;
} currentPointers;

#define ADDON_EXCEPTION(msg) ExceptHandler(msg, GetExceptionCode(), GetExceptionInformation(), __FILE__, __FUNCTION__, __LINE__)

DWORD ExceptHandler(const char *msg, DWORD code, EXCEPTION_POINTERS *ep, const char *file, const char *func, int line) {
	EXCEPTION_RECORD *er = ep->ExceptionRecord;
	CONTEXT *ctx = ep->ContextRecord;

#ifdef DEBUG
	Logger::LogString(LogLevel::Critical, "Exception", "File : " + std::string(file));
	Logger::LogString(LogLevel::Critical, "Exception", "Func : " + std::string(func));
	Logger::LogString(LogLevel::Critical, "Exception", "Line : " + std::to_string(line));
	Logger::LogString(LogLevel::Critical, "Exception", "Message : " + std::string(msg));
#else
	Logger::LogString(LogLevel::Critical, "Exception", "Message : " + std::string(msg));
#endif
	return EXCEPTION_EXECUTE_HANDLER;
}

void __fastcall hkGameThread(uintptr_t, int, int);

struct KeybindSettingsMenu
{
	char DisplayString[256];
	bool Setting = false;
	std::function<void(const std::set<uint>&)> SetCallback;

	void SetDisplayString(const std::set<uint>& keys)
	{
		std::string keybind = "";
		for (const auto& k : keys)
		{
			keybind += GetKeyName(k) + std::string(" + ");
		}

		strcpy_s(DisplayString, (keybind.size() > 0 ? keybind.substr(0, keybind.size() - 3) : keybind).c_str());
	}

	void CheckSetKeybind(const std::set<uint>& keys, bool apply)
	{
		if (Setting)
		{
			SetDisplayString(keys);
			if (apply)
			{
				Setting = false;
				SetCallback(keys);
			}
		}
	}
};

ImVec4 operator/(const ImVec4& v, float f)
{
	return ImVec4(v.x / f, v.y / f, v.z / f, v.w / f);
}

IDirect3D9 *WINAPI Direct3DCreate9(UINT SDKVersion)
{
	assert(SDKVersion == D3D_SDK_VERSION);
	if (!OriginalD3D9)
	{
		char path[MAX_PATH];

		// Try to chainload first
		GetCurrentDirectoryA(MAX_PATH, path);
		strcat_s(path, "\\d3d9_mchain.dll");

		if (!FileExists(path))
		{
			GetCurrentDirectoryA(MAX_PATH, path);
			strcat_s(path, "\\bin64\\d3d9_mchain.dll");
		}

		if (!FileExists(path))
		{
			GetSystemDirectoryA(path, MAX_PATH);
			strcat_s(path, "\\d3d9.dll");
		}

		OriginalD3D9 = LoadLibraryA(path);
	}
	orig_Direct3DCreate9 = (D3DC9)GetProcAddress(OriginalD3D9, "Direct3DCreate9");

	return new f_iD3D9(orig_Direct3DCreate9(SDKVersion));
}

void Shutdown()
{
	ImGui_ImplDX9_Shutdown();
	__try {
		m_hooker.unhook(m_hkAlertCtx);
	}
	__except (ADDON_EXCEPTION("[hkGameThread] Exception in game thread")) {

	}
}

void InitDLL() {
	Logger::Init("IncQol.log");
	Logger::SetMinLevel(LOG_LVL);
	Logger::LogString(LogLevel::Info, MAIN_INFO, "Addon Started");
	if (Config::Load()) {
		Logger::LogString(LogLevel::Info, MAIN_INFO, "Config successfully loaded");
	}
	else {
		Logger::LogString(LogLevel::Info, MAIN_INFO, "Config not found, default loaded");
	}
	Logger::LogString(LogLevel::Debug, MAIN_INFO, "Creating option window");
	optionWindow = new Window("Options");
	pluginBase->AddWindow(optionWindow);
	Logger::LogString(LogLevel::Debug, MAIN_INFO, "Creating keybind for option window");
	openOptions->plugin = MAIN_INFO;
	openOptions->name = "OpenOptions";
	openOptions->keys = Config::LoadKeyBinds(openOptions->plugin, openOptions->name, { VK_MENU,VK_SHIFT,'O' });
	openOptions->func = std::bind(&Window::ChangeState, optionWindow);
	Logger::LogString(LogLevel::Debug, MAIN_INFO, "Registering keybind for option window");
	pluginBase->RegisterKeyBind(openOptions);
	Logger::LogString(LogLevel::Info, MAIN_INFO, "Addon initialization finished");
}

bool WINAPI DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		InitDLL();
		break;
	}
	case DLL_PROCESS_DETACH:
	{
		/*if (OriginalD3D9)
		{
			FreeLibrary(OriginalD3D9);
			OriginalD3D9 = nullptr;
		}*/
		break;
	}
	}
	return true;
}


extern LRESULT ImGui_ImplDX9_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	std::list<EventKey> eventKeys;
	// Generate our EventKey list for the current message
	{
		bool alt = ImGui::GetIO().KeyAlt;
		bool eventDown = false;
		switch (msg)
		{
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			eventDown = true;
			if (((lParam >> 30) & 1) == 1) {
				break;//ignore continues presses
			}
		case WM_SYSKEYUP:
		case WM_KEYUP:
			if (msg == WM_SYSKEYDOWN || msg == WM_SYSKEYUP)
			{
				if (((lParam >> 29) & 1) == 1)
					eventKeys.push_back({ VK_MENU, true });
				else
					eventKeys.push_back({ VK_MENU, false });
			}
			eventKeys.push_back({ VK_MENU, alt });
			eventKeys.push_back({ (uint)wParam, eventDown });
			break;

		case WM_LBUTTONDOWN:
			eventDown = true;
		case WM_LBUTTONUP:
			eventKeys.push_back({ VK_LBUTTON, eventDown });
			break;
		case WM_MBUTTONDOWN:
			eventDown = true;
		case WM_MBUTTONUP:
			eventKeys.push_back({ VK_MBUTTON, eventDown });
			break;
		case WM_RBUTTONDOWN:
			eventDown = true;
		case WM_RBUTTONUP:
			eventKeys.push_back({ VK_RBUTTON, eventDown });
			break;
		case WM_XBUTTONDOWN:
			eventDown = true;
		case WM_XBUTTONUP:
			eventKeys.push_back({ (uint)(GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2), eventDown });
			break;
		}
	}

	ImGui_ImplDX9_WndProcHandler(hWnd, msg, wParam, lParam);

	if (pluginBase->PushKeys(eventKeys)) return true;

	if (pluginBase->HasFocusWindow() && pluginBase->IsCloseWindowBindDown() ) {
		pluginBase->CloseFocusedWindow();
		return true;
	}
	pluginBase->CheckKeyBinds();
	// Prevent game from receiving input if ImGui requests capture
	const auto& io = ImGui::GetIO();
	if (io.WantCaptureMouse) {
		switch (msg) {
		case WM_LBUTTONDOWN:
		case WM_MOUSEMOVE:
		case WM_MOUSEWHEEL:
			return true;
		case WM_INPUT:
		{
			UINT dwSize = 40;
			static BYTE lpb[40];

			GetRawInputData((HRAWINPUT)lParam, RID_INPUT,
				lpb, &dwSize, sizeof(RAWINPUTHEADER));

			RAWINPUT* raw = (RAWINPUT*)lpb;

			if (raw->header.dwType == RIM_TYPEMOUSE)
				return true;

			break;
		}
		}
	}
	switch (msg)
	{
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_CHAR:
		if (io.WantTextInput)
			return true;
		break;
	}


	// Whatever's left should be sent to the game
	return CallWindowProc(BaseWndProc, hWnd, msg, wParam, lParam);
}

/*************************
Augmented Callbacks
*************************/

ULONG GameRefCount = 0;

HRESULT f_iD3D9::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType,
	HWND hFocusWindow, DWORD BehaviorFlags,
	D3DPRESENT_PARAMETERS *pPresentationParameters,
	IDirect3DDevice9 **ppReturnedDeviceInterface)
{
	GameWindow = hFocusWindow;

	// Hook WndProc
	BaseWndProc = (WNDPROC)GetWindowLongPtr(hFocusWindow, GWLP_WNDPROC);
	SetWindowLongPtr(hFocusWindow, GWLP_WNDPROC, (LONG_PTR)&WndProc);

	Logger::LogString(LogLevel::Info, MAIN_INFO, "Initializing plugins");
	plugin->Init(pluginBase);
	Logger::LogString(LogLevel::Info, MAIN_INFO, "Initializing plugins completed");
	Logger::LogString(LogLevel::Info, MAIN_INFO, "PreCreateDevice() plugins");
	plugin->PreCreateDevice();
	Logger::LogString(LogLevel::Info, MAIN_INFO, "PreCreateDevice() plugins completed");

	// Create and initialize device
	IDirect3DDevice9* temp_device = nullptr;
	HRESULT hr = f_pD3D->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, &temp_device);
	RealDevice = temp_device;
	*ppReturnedDeviceInterface = new f_IDirect3DDevice9(temp_device);

	// Init ImGui
	auto& imio = ImGui::GetIO();
	std::string location = GetAddonFolder().append("imgui.ini");
	size_t size = location.size() + 1;
	char *cptr = new char[size];
	strcpy_s(cptr, size, location.c_str());
	imio.IniFilename = cptr;

	// Setup ImGui binding
	ImGui_ImplDX9_Init(hFocusWindow, temp_device);

	// Initialize graphics
	ScreenWidth = pPresentationParameters->BackBufferWidth;
	ScreenHeight = pPresentationParameters->BackBufferHeight;

	Logger::LogString(LogLevel::Info, MAIN_INFO, "PostCreateDevice() plugins");
	plugin->PostCreateDevice();
	Logger::LogString(LogLevel::Info, MAIN_INFO, "PostCreateDevice() plugins completed");


	return hr;
}

HRESULT f_IDirect3DDevice9::Reset(D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	ImGui_ImplDX9_InvalidateDeviceObjects();

	plugin->PreReset();

	HRESULT hr = f_pD3DDevice->Reset(pPresentationParameters);

	ScreenWidth = pPresentationParameters->BackBufferWidth;
	ScreenHeight = pPresentationParameters->BackBufferHeight;

	plugin->PostReset();

	ImGui_ImplDX9_CreateDeviceObjects();

	return hr;
}

void RenderReadonly(const char* label, char* value, int size) {
	ImGui::Text(label);
	ImGui::SameLine(110);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
	ImGui::InputText(std::string("##").append(label).c_str(), value, size, ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_AutoSelectAll);
	ImGui::PopStyleColor();
}
void RenderReadonlyValue(const char* label,uintptr_t value) {
	char* inputTxt;
	std::string str = ToHex(value);
	int size = (int)str.size() +1;
	inputTxt = new char[size];
	strcpy_s(inputTxt, size, str.c_str());
	RenderReadonly(label, inputTxt, size);
}
void RenderReadonlyValue(const char* label, std::string str) {
	char* inputTxt;
	int size = (int)str.size() + 1;
	inputTxt = new char[size];
	strcpy_s(inputTxt, size, str.c_str());
	RenderReadonly(label, inputTxt, size);
}

HRESULT f_IDirect3DDevice9::Present(CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion)
{

	// We have to use Present rather than hooking EndScene because the game seems to do final UI compositing after EndScene
	// This unfortunately means that we have to call Begin/EndScene before Present so we can render things, but thankfully for modern GPUs that doesn't cause bugs
	f_pD3DDevice->BeginScene();

	ImGui_ImplDX9_NewFrame();

	if (pluginBase->GetState() == PluginState::CREATED) {
		if (!GetContext) {
			GetContext = hl::FindPattern("65 48 8b 04 25 58 00 00 00 ba 07 00 00 00");
			if (!GetContext.data()) {
				//can not find any context (aob is invalid)
				pluginBase->SetState(PluginState::FAILURE);
				return S_OK;
			}
			GetContext = (uintptr_t)GetContext.data() - 0x6;
		}
		currentPointers.mouseFocusBase = hl::FollowRelativeAddress(hl::FindPattern("33 DB 41 B9 22 00 00 00 48 8D 0D") + 0xB);
		GetCodedTextFromHashId = hl::FindPattern("53 57 48 83 EC 48 8B D9 E8 ?? ?? ?? ?? 48 8B 48 50 E8 ?? ?? ?? ?? 44 8B 4C 24 68 48 8D 4C 24 30 48 8B F8") - 0xE;
		DecodeText = hl::FindPattern("49 8B E8 48 8B F2 48 8B F9 48 85 C9 75 19 41 B8 ?? ?? ?? ?? 48") - 0x14;

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

		void* pAlertCtx = (void*)hl::FollowRelativeAddress(hl::FollowRelativeAddress(results[0] + 0xa) + 0x3);
		m_hkAlertCtx = m_hooker.hookVT(*(uintptr_t*)pAlertCtx, 0, (uintptr_t)hkGameThread);
		if (m_hkAlertCtx) {
			pluginBase->SetState(PluginState::INITIALIZED);
			Logger::LogString(LogLevel::Info, MAIN_INFO, "GetContext addr: " + ToHex((uintptr_t)GetContext.data()));
			Logger::LogString(LogLevel::Info, MAIN_INFO, "MouseHover addr: " + ToHex(currentPointers.mouseFocusBase));
			Logger::LogString(LogLevel::Info, MAIN_INFO, "GetCodedTextFromHashId addr: " + ToHex((uintptr_t)GetCodedTextFromHashId.data()));
			Logger::LogString(LogLevel::Info, MAIN_INFO, "DecodeText addr: " + ToHex((uintptr_t)DecodeText.data()));
			Logger::LogString(LogLevel::Info, MAIN_INFO, "GameThread Hook addr: " + ToHex((uintptr_t)pAlertCtx));
		}
		else {
			Logger::LogString(LogLevel::Error, MAIN_INFO, "GameThread Hook invalid addr: " + ToHex((uintptr_t)pAlertCtx));
			Logger::LogString(LogLevel::Error, MAIN_INFO, "results[0] : " + ToHex(results[0]));
		}
	}

	plugin->Render();

#ifdef DEBUG
	if (ImGui::Begin("IncQol-Debug",0,ImGuiWindowFlags_AlwaysAutoResize)) {

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
			RenderReadonlyValue("Name", currentPointers.hoveredItemData.name);
			RenderReadonlyValue("ID", currentPointers.hoveredItemData.id);
			RenderReadonlyValue("ItemType", currentPointers.hoveredItemData.itemtype);
			RenderReadonlyValue("Rarity", currentPointers.hoveredItemData.rarity);
			RenderReadonlyValue("Level", currentPointers.hoveredItemData.level);
			RenderReadonlyValue("Sellable?", currentPointers.hoveredItemData.sellable);
		}
	}
	ImGui::End();
#endif
	if (optionWindow->Begin()) {
		pluginBase->RenderKeyBinds();
		optionWindow->End();
	}

	ImGui::Render();

	f_pD3DDevice->EndScene();

	return f_pD3DDevice->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

ULONG f_IDirect3DDevice9::AddRef()
{
	GameRefCount++;
	return f_pD3DDevice->AddRef();
}

ULONG f_IDirect3DDevice9::Release()
{
	GameRefCount--;
	if (GameRefCount == 0)
		Shutdown();

	return f_pD3DDevice->Release();
}

static void __fastcall cbDecodeText(uintptr_t* ctx, wchar_t* decodedText)
{
	if (ctx && decodedText && decodedText[0] != 0) {
		uintptr_t v = (uintptr_t)ctx;
		decodeIDs[v] = ws2s(std::wstring(decodedText));
	}
}

void ReadItemBase(ItemData& data, hl::ForeignClass pBase) {
	data.pItemData = pBase;
	data.id = pBase.get<int>(0x28);
	data.level = pBase.get<int>(0x74);
	data.rarity = (ItemRarity)pBase.get<int>(0x60);
	data.sellable = !(pBase.get<byte>(0x39) & 0x40);
	data.pExtendedType = pBase.get<void*>(0x30);
	if (data.sellable) {
		data.sellable = (pBase.get<byte>(0x88) > 0x0 || pBase.get<byte>(0x4c) > 0x0);
	}
	data.itemtype = (ItemType)pBase.get<int>(0x2C);
	uint hashId = pBase.get<uint>(0x80);
	if (hashId == 0) {
		hl::ForeignClass hash = pBase.get<void*>(0xA8);
		hashId = hash.get<uint>(0x58);
	}
	if (hashId == 0) return;
	if (decodeIDs.find(hashId) == decodeIDs.end()) {
		uintptr_t* d = (uintptr_t*)GetCodedTextFromHashId(hashId, 0);
		DecodeText(d, cbDecodeText, hashId);
	}
	data.name = decodeIDs[hashId];
}
void ReadItemData(ItemStackData& data, hl::ForeignClass pBase) {
	if (!pBase) return;
	hl::ForeignClass itemPtr = pBase.call<void*>(0x20);
	if (!itemPtr) return;
	data.pItem = pBase;
	data.count = pBase.call<int>(0x78);
	ReadItemBase(data.itemData, itemPtr);
}
void SetupMisc() {

	currentPointers.elementParam = 0;
	currentPointers.LocationPtr = 0;
	currentPointers.itemPtr = 0;
	currentPointers.objOnElement = 0;
	currentPointers.hoveredElement = 0;
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
			ReadItemData(data, item);
			currentPointers.itemPtr = (uintptr_t)data.itemData.pItemData.data();
			currentPointers.hoveredItemData = data.itemData;
		}
		return;
	}
	hl::ForeignClass itemData = objOnElement.get<void*>(0x70);
	hl::ForeignClass itemCollection = objOnElement.get<void*>(0x78);//collection of materials
	if (itemData && itemCollection && ((uintptr_t)itemData.data() == ((uintptr_t)itemCollection.data() + 0x60 + currentPointers.elementParam * 0x20))) {
		currentPointers.LocationPtr = (uintptr_t)itemCollection.data();
		itemData = itemData.get<void*>(0x0);
		if (itemData) {
			currentPointers.itemPtr = (uintptr_t)itemData.data();
			ReadItemBase(currentPointers.hoveredItemData, itemData);
			return;
		}
	}
	itemCollection = objOnElement.get<void*>(0x80);
	if (itemCollection && ((uintptr_t)itemCollection.data() == currentPointers.guildInv)) {//guild slots
		currentPointers.LocationPtr = (uintptr_t)itemCollection.data();
		itemData = objOnElement.get<void*>(0x98);
		if (itemData) {
			currentPointers.itemPtr = (uintptr_t)itemData.data();
			ReadItemBase(currentPointers.hoveredItemData, itemData);
			return;
		}
	}
}
void SetupGuild() {
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
void SetupPlayer() {
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

	uint32_t t;
	hl::ForeignClass player = charctx.get<void*>(0x98);
	currentPointers.player = (uintptr_t)player.data();
	if (!player) {
		if (pluginBase->GetInventory(&t).itemStackDatas.size()>0)
			pluginBase->SetInventory(InventoryData());
		return;
	}

	hl::ForeignClass inventory = player.call<void*>(0x80);
	currentPointers.inventory = (uintptr_t)inventory.data();
	if (!inventory) return;
	InventoryData oldData = pluginBase->GetInventory(&t);
	bool changed = false;
	InventoryData inventoryData = InventoryData();
	inventoryData.size = inventory.call<int>(0x1A0);//get count slots for bagcount
	inventoryData.bagCount = inventory.call<int>(0x118);
	inventoryData.slotsPerBag = inventoryData.size / inventoryData.bagCount;
	std::vector<BagData> oldBagData = oldData.bagDatas;
	std::vector<BagData> bagData;
	for (int i = 0; i < inventoryData.bagCount; i++) {
		hl::ForeignClass itemStackPtr = inventory.call<void*>(0x108, i);
		BagData data;
		ReadItemData(data, itemStackPtr);
		if (data.itemData.pExtendedType) {
			data.bagSize = data.itemData.pExtendedType.get<int>(0x28);
			data.noSellOrSort = data.itemData.pExtendedType.get<bool>(0x0);
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
	int slotCount = inventory.get<int>(0xD4);//unsafe but nothing better
	for (int i = 0; i < slotCount; i++) {
		hl::ForeignClass itemStackPtr = inventory.call<void*>(0x168, i);
		ItemStackData data;
		ReadItemData(data, itemStackPtr);
		int bagSlot = i / inventoryData.slotsPerBag;
		if (bagSlot < bagData.size()){
			BagData bag = bagData[bagSlot];
			if (bag.pItem && data.itemData.sellable)  data.itemData.sellable = !bag.noSellOrSort;
		}
		data.slot = i;
		itemData.push_back(data);
		if (changed) continue;
		if (itemData.size() > oldItemData.size()) {
			changed = true;
		}
		if (changed) continue;
		ItemStackData old = oldItemData[itemData.size()-1];
		if (data.pItem != old.pItem || data.slot != old.slot) {
			changed = true; 
		}
	}
	if (itemData.size() != oldItemData.size()) {
		changed = true;
	}
	inventoryData.itemStackDatas = itemData;
	if (changed)
		pluginBase->SetInventory(inventoryData);
}
void GameHook()
{
	pCtx = GetContext();

	SetupPlayer();

	SetupGuild();

	SetupMisc();

	if (currentPointers.itemPtr) {
		pluginBase->SetHoveredItem(currentPointers.hoveredItemData);
	}
	else {
		pluginBase->SetHoveredItem(ItemData());
	}

	plugin->PluginMain();
	
}
void __fastcall hkGameThread(uintptr_t pInst, int, int frame_time)
{

	static auto orgFunc = ((void(__thiscall*)(uintptr_t, int))m_hkAlertCtx->getLocation());

	std::lock_guard<std::mutex> lock(m_gameDataMutex);

	[&] {
		__try {
			GameHook();
		}
		__except (ADDON_EXCEPTION("[hkGameThread] Exception in game thread")) {
			;
		}
	}();

	orgFunc(pInst, frame_time);
}
