#include "Window.h"
#include "Config.h"

Window::Window(char * name,bool defaultOpened,char* saveName) : _name(name), saveName(saveName)
{
	_opened = Config::LoadBool("Windows", saveName==""? _name : saveName, defaultOpened);
}

Window::~Window()
{
}

void Window::SetOpen(bool open)
{
	_opened = open;
	Config::SaveBool("Windows", saveName == "" ? _name : saveName, _opened);
	Config::Save();
	if (!_opened) ReleaseFocus();
}

bool Window::IsOpen()
{
	return _opened;
}
void Window::ReleaseFocus() {
	if (_focused) {
		_focused = false;
		ImGui::SetWindowFocus(NULL);
	}
}
bool Window::IsFocus()
{
	return _focused;
}
void Window::ChangeState() {
	SetOpen(!IsOpen());
}

void Window::SetMinSize(ImVec2 size)
{
	minSize = size;
}
void Window::SetMaxSize(ImVec2 size)
{
	maxSize = size;
}
void Window::SetFirstSize(ImVec2 size) {
	firstSize = size;
}
void Window::SetStartPosition(ImVec2 pos) {
	position = pos;
}
bool Window::Begin()
{
	bool entered = false;
	bool hitExit = _opened;
	ImGui::SetNextWindowSizeConstraints(minSize, maxSize);
	ImGui::SetNextWindowPos(ImVec2((position.x==0.0f?GameWidth/2.0f:position.x)-firstSize.x/2.0f, (position.y == 0.0f ? GameHeight / 2.0f : position.y) -firstSize.y/2.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(firstSize, ImGuiCond_FirstUseEver);
	if (_opened && ImGui::Begin(_name,&hitExit,flags))
	{		
		entered = true;
		_focused = ImGui::IsWindowFocused();
		if (!hitExit) {
			SetOpen(hitExit);
		}
	}
	return entered;
}

void Window::End()
{
	ImGui::End();
}