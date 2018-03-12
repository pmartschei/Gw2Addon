#ifndef ROOT_GROUP_FILTER_H
#define ROOT_GROUP_FILTER_H
#include "GroupFilter.h"

class RootGroupFilter : public GroupFilter
{
protected:
	virtual void SerializeContent(tinyxml2::XMLPrinter & printer) override;
public:
	RootGroupFilter();
	RootGroupFilter(const RootGroupFilter& copy);
	virtual void Render() override;
	virtual void Serialize(tinyxml2::XMLPrinter& printer) override;
	virtual void Deserialize(tinyxml2::XMLElement* element) override;
};
#endif