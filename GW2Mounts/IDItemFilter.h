#ifndef ID_ITEM_FILTER_H
#define ID_ITEM_FILTER_H
#include "SingleFilter.h"
#include "ItemStackData.h"

class IDItemFilter : public SingleFilter<int> {
protected:
	virtual void SerializeContent(tinyxml2::XMLPrinter &printer) override;
	virtual void DeserializeContent(tinyxml2::XMLElement* element) override;
	virtual IFilter* CreateNew() override;
public:
	IDItemFilter();

	// Geerbt über SingleFilter
	virtual std::string GetName() override;
	virtual bool IsFiltered(ItemStackData data) override;
	virtual void RenderInput(int & value) override;

	// Geerbt über Filter
	virtual char* GetSerializeName() override;


};
#endif