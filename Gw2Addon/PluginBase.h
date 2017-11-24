#ifndef PLUGIN_BASE_H
#define PLUGIN_BASE_H
#include "Window.h"
#include "InventoryData.h"
#include "ItemData.h"
#include "main.h"
#include "Logger.h"

class Window; 
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
	//return lhs.plugin < rhs.plugin;
}
class PluginBase {
private:
	std::set<uint> _frameDownKeys;
	std::set<uint> _downKeys;
	std::set<uint> _closeWindowKeys = { VK_ESCAPE };
	std::list<EventKey> eventKeys;
	InventoryData inventory;
	ItemData hoveredItem;
	uint32_t inventoryUpdateIndex = 0;
	std::vector<KeyBindData*> keyBinds;

	KeyBindData* keybindVisual;
protected:
	std::list<Window*> _windows;
	Window* _focusedWindow = 0;
public:
	void AddWindow(Window* window);
	void CloseFocusedWindow();
	bool HasFocusWindow();

	bool IsCloseWindowBindDown();
	bool KeysDown(std::set<uint> keys);
	bool PushKeys(std::list<EventKey> eventKeys);
	std::set<uint> GetKeys();
	bool CheckKeyBinds();

	bool KeybindText(std::string suffix,KeyBindData* data);

	InventoryData GetInventory(uint32_t* updateIndex);
	void SetInventory(InventoryData data);

	void SetHoveredItem(ItemData data);
	ItemData GetHoveredItem();
	bool HasHoveredItem();

	void RegisterKeyBind(KeyBindData* keybind);
	void UnregisterKeyBind(KeyBindData* keybind);

	void RenderKeyBinds();
};
#endif