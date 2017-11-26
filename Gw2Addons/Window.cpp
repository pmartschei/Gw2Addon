#include "Window.h"

Window::Window(char * name,bool defaultOpened) : _name(name)
{
	_opened = Config::LoadBool("Windows", _name, defaultOpened);
}

Window::~Window()
{
}

void Window::SetOpen(bool open)
{
	_opened = open;
	Config::SaveBool("Windows", _name, _opened);
	Config::Save();
	if (!_opened) _focused = false;
}

bool Window::IsOpen()
{
	return _opened;
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

bool Window::Begin()
{
	bool entered = false;
	bool hitExit = _opened;
	ImGui::SetNextWindowSizeConstraints(minSize, ImVec2(10000, 10000));
	if (_opened && ImGui::Begin(_name,&hitExit,ImVec2(400,300),-1.0f,flags))
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