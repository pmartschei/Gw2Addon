#include "main.h"
#include "d3d9.h"
#include <imgui_impl_dx9.h>
#include "Config.h"
#include "Utility.h"
#include "FilterPlugin.h"
#include "PluginBase.h"
/*
	TODO: 
	Tradingpost values
	Filtered Items wegwerfen
	s_styleData[..] 26 colors for ui etc size 20 


	shift modifier eventuell option dafür
*/
HWND GameWindow = 0;
WNDPROC BaseWndProc;
HMODULE OriginalD3D9 = nullptr;
IDirect3DDevice9* RealDevice = nullptr;

// Rendering

Plugin* plugin= (Plugin*)new FilterPlugin();

PluginBase* pluginBase;

IDirect3D9 *WINAPI Direct3DCreate9(UINT SDKVersion)
{
	assert(SDKVersion == D3D_SDK_VERSION);
	if (!OriginalD3D9)
	{
		char path[MAX_PATH];
		// Try to chainload first with config
		GetCurrentDirectoryA(MAX_PATH, path);
		const char* chainPath = Config::LoadText(MAIN_INFO, CHAINLOAD_DLL, "d3d9_incqol_chain.dll");
		strcat_s(path, "\\bin64\\");
		strcat_s(path,chainPath);

		if (!FileExists(path))//check system d3d9 after
		{
			GetSystemDirectoryA(path, MAX_PATH);
			strcat_s(path, "\\d3d9.dll");
		}

		OriginalD3D9 = LoadLibraryA(path);//load d3d9 from path
	}
	orig_Direct3DCreate9 = (D3DC9)GetProcAddress(OriginalD3D9, "Direct3DCreate9");//orig proc address

	return new f_iD3D9(orig_Direct3DCreate9(SDKVersion));
}

/*
Initializes the DLL 
*/
void InitDLL() {
	Logger::Init("IncQol.log");
	Logger::LogString(LogLevel::Info, MAIN_INFO, "Version : " + std::string(VERSION));
	Logger::LogString(LogLevel::Info, MAIN_INFO, "Addon Started");
	Logger::SetMinLevel(LOG_LVL);
	Addon::ClassicColors();
	Config::Init();
	if (Config::Load()) {
		Logger::LogString(LogLevel::Info, MAIN_INFO, "Config successfully loaded");
	}
	else {
		Logger::LogString(LogLevel::Info, MAIN_INFO, "Config not found, default loaded");
	}
	pluginBase = PluginBase::GetInstance();
	pluginBase->Init();
	Logger::LogString(LogLevel::Info, MAIN_INFO, "Addon initialization finished");
}
/*
DLL Main, thats the start of our code
*/
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

/*
Mouse/Keyboard Handler 
*/
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
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
			/*if (msg == WM_SYSKEYDOWN || msg == WM_SYSKEYUP)
			{
				if (((lParam >> 29) & 1) == 1)
					eventKeys.push_back({ VK_MENU, true });
				else
					eventKeys.push_back({ VK_MENU, false });
			}*/
			//eventKeys.push_back({ VK_MENU, alt });
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
	pluginBase->PushKeys(eventKeys);//just collect the keys

	if (msg != WM_LBUTTONDOWN && msg != WM_LBUTTONUP &&  msg!=WM_MOUSEMOVE && pluginBase->InputKeyBind()) return true;//process everything but no mouse left and move
	bool imguiInteraction = pluginBase->CheckInteractionKeyBind();
	if (imguiInteraction || msg== WM_MOUSEMOVE)//imgui still needs the mouse to update the gui
		ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
	else
		ImGui::GetIO().MouseDown[0] = false;//when you press the interaction and let it stop you need to set the mouse0 to false then any drag is stopped

	if (pluginBase->HasFocusWindow() && pluginBase->IsCloseWindowBindDown() ) {//window closed keybinds
		pluginBase->CloseFocusedWindow();
		return true;
	}
	if (pluginBase->CheckKeyBinds()) return true;//every other keybind
	// Prevent game from receiving input if ImGui requests capture
	const auto& io = ImGui::GetIO();
	if (imguiInteraction && io.WantCaptureMouse) {
		switch (msg) {
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONDBLCLK:
		case WM_MOUSEMOVE:
		case WM_MOUSEWHEEL:
			return true;
		case WM_INPUT: //Dont move the camera ingame when holding mouse
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
		if (io.WantTextInput) {
			if (!imguiInteraction) {//when you forget to hold modifier but still in a input field
				ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
			}
			return true;
		}
		break;
	}
	// Whatever's left should be sent to the game
	switch (msg) {
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		ImGui::SetWindowFocus(NULL);
	default:
		break;
	}
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

	//Logger::LogString(LogLevel::Info, MAIN_INFO, "Initializing plugins");
	plugin->Init();
	pluginBase->AddPlugin(plugin);
	//Logger::LogString(LogLevel::Info, MAIN_INFO, "Initializing plugins completed");
	//Logger::LogString(LogLevel::Info, MAIN_INFO, "PreCreateDevice() plugins");
	plugin->PreCreateDevice();
	//Logger::LogString(LogLevel::Info, MAIN_INFO, "PreCreateDevice() plugins completed");

	// Create and initialize device
	IDirect3DDevice9* temp_device = nullptr;
	HRESULT hr = f_pD3D->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, &temp_device);
	RealDevice = temp_device;
	*ppReturnedDeviceInterface = new f_IDirect3DDevice9(temp_device);

	// Init ImGui
	auto& imio = ImGui::GetIO();
	//load imgui ini
	std::string location = GetAddonFolder().append("imgui.ini");
	size_t size = location.size() + 1;
	char *cptr = new char[size];
	strcpy_s(cptr, size, location.c_str());
	imio.IniFilename = cptr;

	// Setup ImGui binding
	ImGui_ImplDX9_Init(hFocusWindow, temp_device);

	ImGui::GetStyle().WindowMinSize = ImVec2(0, 0);
	ImGui::GetStyle().WindowRounding = 0.0f;
	ImGui::GetStyle().WindowPadding = ImVec2(4, 4);

	// Initialize graphics
	GameWidth = pPresentationParameters->BackBufferWidth;
	GameHeight = pPresentationParameters->BackBufferHeight;

	//Logger::LogString(LogLevel::Info, MAIN_INFO, "PostCreateDevice() plugins");
	plugin->PostCreateDevice();
	//Logger::LogString(LogLevel::Info, MAIN_INFO, "PostCreateDevice() plugins completed");


	return hr;
}

HRESULT f_IDirect3DDevice9::Reset(D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	ImGui_ImplDX9_InvalidateDeviceObjects();

	plugin->PreReset();

	HRESULT hr = f_pD3DDevice->Reset(pPresentationParameters);

	GameWidth = pPresentationParameters->BackBufferWidth;
	GameHeight = pPresentationParameters->BackBufferHeight;

	plugin->PostReset();

	ImGui_ImplDX9_CreateDeviceObjects();

	return hr;
}

HRESULT f_IDirect3DDevice9::Present(CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion)
{

	// We have to use Present rather than hooking EndScene because the game seems to do final UI compositing after EndScene
	// This unfortunately means that we have to call Begin/EndScene before Present so we can render things, but thankfully for modern GPUs that doesn't cause bugs
	f_pD3DDevice->BeginScene();

	ImGui_ImplDX9_NewFrame();

	pluginBase->CheckInitialize();
	pluginBase->Render();

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
	/*if (GameRefCount == 0)
		Shutdown();*/

	return f_pD3DDevice->Release();
}

