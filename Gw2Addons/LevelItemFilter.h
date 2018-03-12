#ifndef LEVEL_ITEM_FILTER_H
#define LEVEL_ITEM_FILTER_H
#include "SingleFilter.h"

class LevelItemFilter : public SingleFilter<int> {
protected:
	// Geerbt über Filter
	virtual IFilter* CreateNew() override;
	virtual void SerializeContent(tinyxml2::XMLPrinter &printer) override;
	virtual void DeserializeContent(tinyxml2::XMLElement* element) override;
public:
	LevelItemFilter();

	// Geerbt über SingleFilter
	virtual std::string GetName() override;
	virtual void RenderInput(int & value) override;
	virtual bool IsFiltered(ItemStackData data) override;

	// Geerbt über Filter
	virtual char* GetSerializeName() override;
};
#endif