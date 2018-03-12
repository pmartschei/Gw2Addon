#ifndef GROUP_FILTER_H
#define GROUP_FILTER_H

#include "IFilter.h"
#include <vector>

class GroupFilter : public IFilter {
protected:
	std::vector<IFilter*> subFilters;
	// Geerbt über Filter
	virtual void RenderContent() override;
	virtual void RenderChildren();
	virtual void SerializeContent(tinyxml2::XMLPrinter & printer) override;
	virtual void DeserializeContent(tinyxml2::XMLElement * element) override;
	virtual IFilter* CreateNew() override;
public:
	GroupFilter();
	~GroupFilter();
	void AddFilter(IFilter* filter);
	std::vector<IFilter*>::iterator RemoveFilter(IFilter* filter);
	void RemoveAndDeleteAll();

	virtual void CheckForDeletion();

	// Geerbt über IFilter
	virtual void CustomMenu() override;
	virtual std::set<ItemStackData> Filter(std::set<ItemStackData> d) override;
	virtual bool Updated() override;
	virtual void ResetUpdateState() override;
	virtual char* GetSerializeName() override;
	virtual void SetOpen(bool open) override;
};
#endif
