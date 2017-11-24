#ifndef SELLALE_ITEM_FILTER_H
#define SELLALE_ITEM_FILTER_H
#include "SingleFilter.h"
#include "ItemStackData.h"

class SellableItemFilter : public SingleFilter<bool> {
protected:
	// Geerbt über Filter
	virtual IFilter* CreateNew() override;
public:
	SellableItemFilter();

	// Geerbt über SingleFilter
	virtual std::string GetName() override;
	virtual void RenderInput(bool & value) override;
	virtual bool IsFiltered(ItemStackData data) override;
	// Geerbt über Filter
	virtual char* GetSerializeName() override;
};
#endif