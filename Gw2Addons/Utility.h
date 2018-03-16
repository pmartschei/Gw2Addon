#ifndef _UTILITY_H
#define _UTILITY_H
#include <string>
#include <vector>
#include <windows.h>
#include "main.h"
#include <imgui.h>

#define TEXTSPACE (150.0f)

struct TooltipColor {
	const char* tooltip;
	ImVec4 color;
public:
	TooltipColor(const char* tooltip, ImVec4 color) :tooltip(tooltip), color(color) {}
	TooltipColor(const char* tooltip) :tooltip(tooltip), color(ImGui::GetStyle().Colors[ImGuiCol_Text]) {}
};

std::wstring s2ws(const std::string & str);

std::string ws2s(const std::wstring & wstr);

std::string ToHex(uintptr_t val);

std::string GetKeyName(unsigned int virtualKey);

void SplitFilename(const tstring & str, tstring * folder, tstring * file);

bool FileExists(const char* path);

std::string GetExeFolder();

std::string GetAddonFolder();

wchar_t *convertCharArrayToLPCWSTR(const char* charArray);

std::vector<std::string> ListFilesInDir(std::string dir, std::string filter = "*.*");

// trim from start (in place)
void ltrim(std::string &s);

// trim from end (in place)
void rtrim(std::string &s);
// trim from both ends (in place)
void trim(std::string &s);

void RenderReadonly(const char* label, char* value, int size);
void RenderReadonlyValue(const char* label, uintptr_t value);
void RenderReadonlyValue(const char* label, std::string value);

bool RenderInputText(const char* label, std::string& value, int size,float posX = 0.0f);
bool RenderCheckbox(const char* label,bool* b, std::vector<TooltipColor> tooltips = std::vector<TooltipColor>());
bool RenderInputInt(const char*label, int* value, int min = 0, int max = 0);
bool RenderSliderInt(const char*label, int* value, int min = 0, int max = 0);
void RenderTooltip(std::vector<TooltipColor> tooltips);
#endif