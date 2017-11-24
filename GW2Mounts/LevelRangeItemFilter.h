#ifndef LEVEL_RANGE_ITEM_FILTER_H
#define LEVEL_RANGE_ITEM_FILTER_H
#include "RangeFilter.h"

class LevelRangeItemFilter : public RangeFilter<int> {
protected:
	// Geerbt über Filter
	virtual IFilter* CreateNew() override;
	virtual void SerializeContent(tinyxml2::XMLPrinter &printer) override;
	virtual void DeserializeContent(tinyxml2::XMLElement* element) override;
public:
	LevelRangeItemFilter();

	// Geerbt über RangeFilter
	virtual std::string GetName() override;
	virtual void RenderInput(int & value, std::string s) override;
	virtual bool IsFiltered(ItemStackData data) override;

	// Geerbt über Filter
	virtual char* GetSerializeName() override;
};
#endif
