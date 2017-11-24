#include "PluginBase.h"

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
	ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth()- buttonWidth -ImGui::GetStyle().ColumnsMinSpacing);
	int popcount = 0;
	if (data->isSetMode) {
		popcount = 2;
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(201 / 255.f, 215 / 255.f, 255 / 255.f, 200 / 255.0f));
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
	}
	ImGui::InputText(suffix.c_str(),&text[0],text.size(),ImGuiInputTextFlags_ReadOnly);
	ImGui::PopStyleColor(popcount);
	ImGui::PopItemWidth();
	ImGui::SameLine();
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

std::set<uint> PluginBase::GetKeys()
{
	return _downKeys;
}

void PluginBase::AddWindow(Window * window)
{
	_windows.push_back(window);
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

InventoryData PluginBase::GetInventory(uint32_t* updateIndex)
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

void PluginBase::SetHoveredItem(ItemData data)
{
	hoveredItem = data;
}

ItemData PluginBase::GetHoveredItem()
{
	return hoveredItem;
}

bool PluginBase::HasHoveredItem()
{
	return hoveredItem.id != 0;
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

void PluginBase::RenderKeyBinds()
{
	const char* lastPlugin = nullptr;
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
		ImGui::SameLine(150);
		if (KeybindText("##" + std::string(keyBind->plugin) + std::string(keyBind->name), keyBind)) {
			Config::SaveKeyBinds(keyBind->plugin, keyBind->name, keyBind->keys);
			Config::Save();
		}
	}
}

