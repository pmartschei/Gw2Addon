#ifndef TYPE_ITEM_FILTER_H
#define TYPE_ITEM_FILTER_H
#include "SingleFilter.h"

class TypeItemFilter : public SingleFilter<ItemType> {
protected:
	// Geerbt �ber SingleFilter
	virtual std::string GetName() override;
	// Geerbt �ber Filter
	virtual IFilter* CreateNew() override;
	virtual void SerializeContent(tinyxml2::XMLPrinter &printer) override;
	virtual bool DeserializeContent(tinyxml2::XMLElement* element) override;
public:
	TypeItemFilter();

	// Geerbt �ber SingleFilter
	virtual void RenderInput(ItemType &value) override;
	virtual bool IsFiltered(FilterData data) override;
	// Geerbt �ber Filter
	virtual char* GetSerializeName() override;

	virtual void CustomMenu() override;

	virtual void AddFilter(IFilter* filter) override;
	virtual void RemoveFilter(IFilter* filter) override;

};
#endif