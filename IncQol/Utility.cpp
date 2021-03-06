#include "Utility.h"
#include <cctype>
#include <imgui.h>
#include <algorithm>
#include <sstream>
#include <codecvt>
#include <mutex>

uint GameWidth, GameHeight;
/// <summary>
/// Converts a std::string into the equivalent std::wstring.
/// </summary>
/// <param name="str">The std::string.</param>
/// <returns>The con	verted std::wstring.</returns>
std::wstring s2ws(const std::string& str)
{
	typedef std::codecvt_utf8<wchar_t> convert_typeX;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.from_bytes(str);
}

/// <summary>
/// Converts a std::wstring into the equivalent std::string. Possible loss of character information.
/// </summary>
/// <param name="wstr">The std::wstring.</param>
/// <returns>The converted std::string.</returns>
std::string ws2s(const std::wstring& wstr)
{
	typedef std::codecvt_utf8<wchar_t> convert_typeX;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

std::string ToHex(uintptr_t val)
{
	std::stringstream stream;
	stream << "0x" << std::uppercase << std::hex << val;
	return stream.str();
}

std::string GetKeyName(unsigned int virtualKey)
{
	unsigned int scanCode = MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC);

	// because MapVirtualKey strips the extended bit for some keys
	switch (virtualKey)
	{
	case VK_LBUTTON:
		return "M1";
	case VK_RBUTTON:
		return "M2";
	case VK_MBUTTON:
		return "M3";
	case VK_XBUTTON1:
		return "M4";
	case VK_XBUTTON2:
		return "M5";
	case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN: // arrow keys
	case VK_PRIOR: case VK_NEXT: // page up and page down
	case VK_END: case VK_HOME:
	case VK_INSERT: case VK_DELETE:
	case VK_DIVIDE: // numpad slash
	case VK_NUMLOCK:
		scanCode |= 0x100; // set extended bit
		break;
	}

	char keyName[50];
	if (GetKeyNameTextA(scanCode << 16, keyName, sizeof(keyName)) != 0)
		return keyName;
	else
		return "Key " + std::to_string(virtualKey);
}

void SplitFilename(const tstring& str, tstring* folder, tstring* file)
{
	size_t found = str.find_last_of(TEXT("/\\"));
	if (folder) *folder = str.substr(0, found);
	if (file) *file = str.substr(found + 1);
}

bool FileExists(const char* path)
{
	DWORD dwAttrib = GetFileAttributesA(path);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
wchar_t *convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);	
	return wString;
}

std::string GetExeFolder() {
	TCHAR exeFullPath[MAX_PATH];
	GetModuleFileName(0, exeFullPath, MAX_PATH);
	tstring exeFolder;
	SplitFilename(exeFullPath, &exeFolder, nullptr);
	return std::string(exeFolder.begin(), exeFolder.end()).append("\\");
}

std::string GetAddonFolder() {
	return GetExeFolder().append("addons\\incqol\\");
}
std::vector<std::string> ListFilesInDir(std::string dir, std::string filter)
{
	WIN32_FIND_DATA FindFileData;
	std::vector<std::string> vs;
	HANDLE hFind;
	hFind = FindFirstFile(s2ws(dir.append(filter)).c_str(), &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do {
			vs.push_back(ws2s(FindFileData.cFileName));
		} while (FindNextFile(hFind, &FindFileData));
		FindClose(hFind);
	}
	return vs;
}

// trim from start (in place)
void ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(),
		std::not1(std::ptr_fun<int, int>(std::isspace))));
}

// trim from end (in place)
void rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(),
		std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

// trim from both ends (in place)
void trim(std::string &s) {
	ltrim(s);
	rtrim(s);
}


void RenderReadonly(const char* label, char* value, int size) {
	ImGui::Text(label);
	ImGui::SameLine(TEXTSPACE);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
	ImGui::InputText(std::string("##").append(label).c_str(), value, size, ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_AutoSelectAll);
	ImGui::PopStyleColor();
	delete value;
}
void RenderReadonlyValue(const char* label, uintptr_t value) {
	char* inputTxt;
	std::string str = ToHex(value);
	int size = (int)str.size() + 1;
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
void RenderTextPre(const char* label, std::vector<TooltipColor> tooltips) {
	ImGui::Text(label);
	RenderTooltip(tooltips);
	ImGui::SameLine(TEXTSPACE);
}
bool RenderInputText(const char* label, std::string& value, int size, std::vector<TooltipColor> tooltips) {
	RenderTextPre(label, tooltips);
	char* buf = new char[size];
	strcpy_s(buf, size, value.c_str());
	bool changed = false;
	ImGui::PushItemWidth(-1);
	changed = ImGui::InputText(std::string("##").append(label).c_str(), buf, size);
	ImGui::PopItemWidth();
	value = std::string(buf);
	delete[] buf;
	return changed;
}


bool RenderCheckbox(const char * label, bool * b, std::vector<TooltipColor> tooltips){
	RenderTextPre(label, tooltips);
	return ImGui::Checkbox(std::string("##").append(label).c_str(), b);
}

bool RenderInputInt(const char * label, int * value, int min, int max, std::vector<TooltipColor> tooltips)
{
	RenderTextPre(label, tooltips);
	return ImGui::InputInt(std::string("##").append(label).c_str(), value, min, max);
}

bool RenderSliderInt(const char * label, int * value, int min, int max, std::vector<TooltipColor> tooltips)
{
	RenderTextPre(label, tooltips);
	ImGui::PushItemWidth(-1);
	int before = *value;
	bool changed = ImGui::SliderInt(std::string("##").append(label).c_str(), value, min, max);
	*value = CLAMP(*value, min, max);
	if (*value == before) changed = false;
	ImGui::PopItemWidth();
	return changed;
}

void RenderTooltip(std::vector<TooltipColor> tooltips) {
	float maxTooltipSize = 400.0f;
	float maxTextSize = 0.0f;
	for (int i = 0; i < tooltips.size(); ++i) {
		ImVec2 tsize = ImGui::CalcTextSize(tooltips[i].tooltip);
		if (tsize.x > maxTextSize) {
			maxTextSize = tsize.x;
			if (maxTextSize > maxTooltipSize) break;
		}
	}
	maxTooltipSize = min(maxTextSize, maxTooltipSize);
	if (tooltips.size()>0 && ImGui::IsItemHovered()) {
		ImGui::SetNextWindowSize(ImVec2(maxTooltipSize+ImGui::GetStyle().WindowPadding.x*2+ImGui::GetStyle().FramePadding.x*2, 0));
		ImGui::BeginTooltip();
		for (int i = 0; i < tooltips.size(); ++i) {
			if (tooltips[i].tooltip) {
				ImGui::PushStyleColor(ImGuiCol_Text, tooltips[i].color);
				ImGui::TextWrapped(tooltips[i].tooltip);
				ImGui::PopStyleColor();
			}
		}
		ImGui::EndTooltip();
	}
}