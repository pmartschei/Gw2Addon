#ifndef EXTENDED_ITEM_TYPE_H
#define EXTENDED_ITEM_TYPE_H

class ExtendedItemType {
public:
	enum FilterBy {
		Type,
	};
	virtual const char** GetSubTypes() {
		return nullptr;
	}
	virtual int GetSubSize() {
		return 0;
	}
	virtual bool IsFiltered(void* data,FilterBy filteredBy) {
		return false;
	}
};

#endif