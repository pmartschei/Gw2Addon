#ifndef Rarity_ITEM_FILTER_H
#define Rarity_ITEM_FILTER_H
#include "SingleFilter.h"

class RarityItemFilter : public SingleFilter<ItemRarity> {
protected:
	// Geerbt über SingleFilter
	virtual std::string GetName() override;
	// Geerbt über Filter
	virtual IFilter* CreateNew() override;
	virtual void SerializeContent(tinyxml2::XMLPrinter &printer) override;
	virtual bool DeserializeContent(tinyxml2::XMLElement* element) override;
public:
	RarityItemFilter();

	// Geerbt über SingleFilter
	virtual void RenderInput(ItemRarity &value) override;
	virtual bool IsFiltered(FilterData data) override;

	// Geerbt über Filter
	virtual char* GetSerializeName() override;
};
#endif