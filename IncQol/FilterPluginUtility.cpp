#include "FilterPluginUtility.h"
#include "Utility.h"

std::string GetFilterFolder() {
	return GetAddonFolder().append("filters\\");
}