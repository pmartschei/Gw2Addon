#ifndef PLUGIN_BASE_H
#define PLUGIN_BASE_H
#include <set>
#include <vector>
#include <imgui.h>
#include "InventoryData.h"
#include "ItemData.h"
#include "main.h"
#include "PluginBaseState.h"
#include "hacklib\PatternScanner.h"
#include "hacklib\Hooker.h"
#include "ForeignFunction.h"
#include "Logger.h"
#include "ThreadTaskQueue.h"
#include "Singleton.h"

class Window; 
class Plugin;

#define ADDON_EXCEPTION(msg) ExceptHandler(msg, GetExceptionCode(), GetExceptionInformation(), __FILE__, __FUNCTION__, __LINE__)
#define IGNORE_EXCEPTION IgnoreExceptHandler()

inline DWORD ExceptHandler(const char *msg, DWORD code, EXCEPTION_POINTERS *ep, const char *file, const char *func, int line) {
	EXCEPTION_RECORD *er = ep->ExceptionRecord;
	CONTEXT *ctx = ep->ContextRecord;

#ifdef _DEBUG
	Logger::LogString(LogLevel::Critical, "Exception", "File : " + std::string(file));
	Logger::LogString(LogLevel::Critical, "Exception", "Func : " + std::string(func));
	Logger::LogString(LogLevel::Critical, "Exception", "Line : " + std::to_string(line));
	Logger::LogString(LogLevel::Critical, "Exception", "Message : " + std::string(msg));
#else
	Logger::LogString(LogLevel::Critical, "Exception", "Message : " + std::string(msg));
#endif
	return EXCEPTION_EXECUTE_HANDLER;
}

inline DWORD IgnoreExceptHandler() {
	return EXCEPTION_EXECUTE_HANDLER;
}

struct EventKey
{
	uint vk : 31;
	bool down : 1;
};

struct KeyBindData {
	bool isSetMode = false;
	const char* plugin;
	const char* name;
	std::set<uint> keys;
	std::function<void()> func;
	bool operator<(const KeyBindData& a) {
		return (*this) < a;
	}
};

inline bool operator==(const KeyBindData& lhs, const KeyBindData& rhs)
{
	return lhs.plugin == rhs.plugin && lhs.name == rhs.name;
}
inline bool operator<(const KeyBindData& lhs, const KeyBindData& rhs)
{
	bool d = (lhs.plugin == rhs.plugin);
	if (!d && lhs.plugin == "Main") return true;
	if (!d && rhs.plugin == "Main") return false;
	if (!d) return lhs.plugin < rhs.plugin;
	return lhs.name < rhs.name;
}
class PluginBase : public Singleton<PluginBase>{
friend class Singleton<PluginBase>;
private:
	std::vector<Plugin*> loadedPlugins;
	Window* optionWindow;

	std::set<uint> _frameDownKeys;
	std::set<uint> _downKeys;
	std::set<uint> _closeWindowKeys = { VK_ESCAPE };
	std::list<EventKey> eventKeys;

	InventoryData inventory;
	uint32_t inventoryUpdateIndex = 0;
	ItemData hoveredItem;
	std::vector<KeyBindData*> keyBinds;

	std::string chainLoad;
	KeyBindData* keybindVisual;
	PluginBaseState pluginBaseState = PluginBaseState::CREATED;
	ThreadTaskQueue* ttq;

	ForeignFunction<void*> GetContext;
	ForeignFunction<void*> GetCodedTextFromHashId;
	ForeignFunction<void> DecodeText;
	hl::Hooker m_hooker;
	const hl::IHook *m_hkAlertCtx = nullptr;
	std::mutex m_gameDataMutex;
	void* pCtx;
	void* pAlertCtx;
	std::map<uintptr_t, std::string> decodeIDs;
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

	std::string configItemsUrl;
	std::string configPricesUrl;

	void SetupContext();
	void SetupPlayer();
	void SetupMisc();
	void SetupGuild();
	void ReadItemData(ItemStackData* data, hl::ForeignClass pBase);
	void ReadItemBase(ItemData* data, hl::ForeignClass pBase);
	bool KeysDown(std::set<uint> keys);
	bool KeybindText(std::string suffix, KeyBindData* data);
	void SetInventory(InventoryData data);
	void SetHoveredItem(ItemData data);
	void RenderKeyBinds();
	void RenderColors(const char* id, int size, std::function<const char*(int)> nameFunc, ImVec4* colors);
	void LoadColors(int size, std::function<const char*(int)> nameFunc, ImVec4* colors);
protected:
	PluginBase() {};
	std::list<Window*> _windows;
	Window* _focusedWindow = 0;
public:
	void Init();

	void AddPlugin(Plugin* plugin);
	void AddWindow(Window* window);
	void CloseFocusedWindow();
	bool HasFocusWindow();
	bool IsCloseWindowBindDown();
	void GameHook();

	void CheckInitialize();

	bool PushKeys(std::list<EventKey> eventKeys);
	bool CheckKeyBinds();

	void Render();
	//METHODS FOR PLUGINS
	InventoryData GetInventory(uint* updateIndex);
	ItemData GetHoveredItem();
	bool HasHoveredItem();
	void ProcessTask(Task* task);

	void ReloadConfig();
	std::string GetItemInfoUrl();
	std::string GetPricesUrl();
	//END

	void RegisterKeyBind(KeyBindData* keybind);
	void UnregisterKeyBind(KeyBindData* keybind);
	void AddDecodeID(uintptr_t key, std::string value);

	const hl::IHook* GetAlertHook();
	std::mutex* GetDataMutex();
};
#endif