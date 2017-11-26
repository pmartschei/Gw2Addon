#ifndef _WINDOW_H
#define _WINDOW_H
#include <set>
#include <imgui.h>
#include "Config.h"

class PluginBase;

class Window {
protected:
	bool _opened = false;
	bool _focused = false;
	ImVec2 minSize = ImVec2(0,0);
	char* _name;
public:
	Window(char* name,bool defaultOpened = true);
	~Window();
	void SetOpen(bool open);
	bool IsOpen();
	void ChangeState();
	void SetMinSize(ImVec2 size);

	bool Begin();
	void End();
	bool IsFocus();
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_ShowBorders;
};
#endif