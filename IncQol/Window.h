#ifndef _WINDOW_H
#define _WINDOW_H
#include <set>
#include <imgui.h>
#include "Utility.h"

class PluginBase;

class Window {
protected:
	bool _opened = false;
	bool _focused = false;
	ImVec2 minSize = ImVec2(0,0);
	ImVec2 maxSize = ImVec2(10000, 10000);
	ImVec2 firstSize = ImVec2(400, 300);
	ImVec2 position = ImVec2(0.0f,0.0f);
	char* _name;
	char* saveName;
public:
	Window(char* name,bool defaultOpened = true,char* saveName = "");
	~Window();
	void SetOpen(bool open);
	bool IsOpen();
	void ChangeState();
	void SetMinSize(ImVec2 size);
	void SetMaxSize(ImVec2 size);
	void SetFirstSize(ImVec2 size);
	void SetStartPosition(ImVec2 pos);
	bool Begin();
	void End();
	void ReleaseFocus();
	bool IsFocus();
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing;
};
#endif