#ifndef SUB_TYPE_ITEM_FILTER_H
#define SUB_TYPE_ITEM_FILTER_H
#include "SingleFilter.h"

class SubTypeItemFilter : public SingleFilter<int> {
protected:
	// Geerbt �ber SingleFilter
	virtual std::string GetName() override;
	// Geerbt �ber Filter
	virtual IFilter* CreateNew() override;
	virtual void SerializeContent(tinyxml2::XMLPrinter &printer) override;
	virtual bool DeserializeContent(tinyxml2::XMLElement* element) override;

	ItemType baseType;
public:
	SubTypeItemFilter(ItemType baseType);

	// Geerbt �ber SingleFilter
	virtual void RenderInput(int &value) override;
	virtual bool IsFiltered(FilterData data) override;
	// Geerbt �ber Filter
	virtual char* GetSerializeName() override;
};
#endif