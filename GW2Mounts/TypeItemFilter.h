#ifndef TYPE_ITEM_FILTER_H
#define TYPE_ITEM_FILTER_H
#include "SingleFilter.h"
#include "ItemStackData.h"
#include "ItemType.h"

class TypeItemFilter : public SingleFilter<ItemType> {
protected:
	// Geerbt �ber SingleFilter
	virtual std::string GetName() override;
	// Geerbt �ber Filter
	virtual IFilter* CreateNew() override;
	virtual void SerializeContent(tinyxml2::XMLPrinter &printer) override;
	virtual void DeserializeContent(tinyxml2::XMLElement* element) override;
public:
	TypeItemFilter();

	// Geerbt �ber SingleFilter
	virtual void RenderInput(ItemType &value) override;
	virtual bool IsFiltered(ItemStackData data) override;
	// Geerbt �ber Filter
	virtual char* GetSerializeName() override;

};
#endif