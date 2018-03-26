#ifndef TYPE_ITEM_MULTI_FILTER_H
#define TYPE_ITEM_MULTI_FILTER_H
#include "SingleFilter.h"
#include <vector>

class TypeItemMultiFilter : public SingleFilter<std::vector<bool>> {
protected:
	int typeCount = 0;
	// Geerbt �ber SingleFilter
	virtual std::string GetName() override;
	// Geerbt �ber Filter
	virtual IFilter* CreateNew() override;
	virtual void SerializeContent(tinyxml2::XMLPrinter &printer) override;
	virtual void DeserializeContent(tinyxml2::XMLElement* element) override;
public:
	TypeItemMultiFilter();

	// Geerbt �ber SingleFilter
	virtual void RenderInput(std::vector<bool> &value) override;
	virtual bool IsFiltered(FilterData data) override;
	// Geerbt �ber Filter
	virtual char* GetSerializeName() override;

};
#endif