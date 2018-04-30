#ifndef UPGRADE_COMPONENT_TYPE_H
#define UPGRADE_COMPONENT_TYPE_H

#include "ExtendedItemType.h"

class UpgradeComponentType : public ExtendedItemType {
public:
	enum SubTypes {
		Default=0,
		Gem = 1,
		Rune = 2,
		Sigil = 3,
		Count = 4,
	};

	int type = 0;

	static const char * types[];

	const char** GetSubTypes() override;
	int GetSubSize() override;
	bool IsFiltered(void* data,FilterBy filteredBy) override;
};

#endif