#ifndef Rarity_RANGE_ITEM_FILTER_H
#define Rarity_RANGE_ITEM_FILTER_H
#include "RangeFilter.h"
#include "ItemStackData.h"
#include "ItemRarity.h"

class RarityRangeItemFilter : public RangeFilter<ItemRarity> {
protected:
	// Geerbt über Filter
	virtual IFilter* CreateNew() override;
	virtual void SerializeContent(tinyxml2::XMLPrinter &printer) override;
	virtual void DeserializeContent(tinyxml2::XMLElement* element) override;
public:
	RarityRangeItemFilter();


	// Geerbt über RangeFilter
	virtual std::string GetName() override;
	virtual void RenderInput(ItemRarity & value, std::string s) override;
	virtual bool IsFiltered(ItemStackData data) override;

	// Geerbt über Filter
	virtual char* GetSerializeName() override;
};
#endif