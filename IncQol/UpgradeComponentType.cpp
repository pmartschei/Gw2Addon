#include "UpgradeComponentType.h"
#include <Windows.h>

const char* UpgradeComponentType::types[] = { "Default", "Gem", "Rune", "Sigil", "Undefined" };

const char ** UpgradeComponentType::GetSubTypes()
{
	return UpgradeComponentType::types;
}
int UpgradeComponentType::GetSubSize()
{
	return SubTypes::Count;
}
bool UpgradeComponentType::IsFiltered(void* data, FilterBy filteredBy) {
	switch (filteredBy) {
	case FilterBy::Type:
		int value = *(intptr_t*)data;
		return type == value;
	}
	return false;
};