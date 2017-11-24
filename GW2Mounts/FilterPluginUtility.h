#ifndef FILTER_PLUGIN_UTILITY_H
#define FILTER_PLUGIN_UTILITY_H
#include <string>
#include "Utility.h"

#define STARTUP_FILTERNAME ("startup.filter")

std::string GetFilterFolder();

struct TextFilters {
	static int FilterImGuiLetters(ImGuiTextEditCallbackData* data) {
		int c = tolower(data->EventChar);
		if ((c >= 'a' &&c <= 'z') || (c >= '0' && c <= '9') || c == 32 || c == 95 || c == 45)/// 95 = '_' , 45 = '-'
			return 0;
		return 1;
	}
};
#endif