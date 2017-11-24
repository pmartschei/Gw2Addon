#ifndef SELLALE_ITEM_FILTER_H
#define SELLALE_ITEM_FILTER_H
#include "SingleFilter.h"
#include "ItemStackData.h"

class SellableItemFilter : public SingleFilter<bool> {
protected:
	// Geerbt �ber Filter
	virtual IFilter* CreateNew() override;
public:
	SellableItemFilter();

	// Geerbt �ber SingleFilter
	virtual std::string GetName() override;
	virtual void RenderInput(bool & value) override;
	virtual bool IsFiltered(ItemStackData data) override;
	// Geerbt �ber Filter
	virtual char* GetSerializeName() override;
};
#endif