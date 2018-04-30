#ifndef GROUP_FILTER_H
#define GROUP_FILTER_H

#include "IFilter.h"
#include <vector>

class GroupFilter : public IFilter {
protected:
	IFilter** subFilters = new IFilter*[1];
	int filterSize = 1;
	int filterIndex = 0;
	int activeChildFilters = 0;
	bool showNameInput = true;
	bool supportDrop = true;
	// Geerbt über Filter
	virtual void RenderContent() override;
	virtual void RenderChildren();
	virtual void SerializeContent(tinyxml2::XMLPrinter & printer) override;
	virtual bool DeserializeContent(tinyxml2::XMLElement * element) override;
	virtual IFilter* CreateNew() override;
public:
	GroupFilter();
	~GroupFilter();
	virtual void AddFilter(IFilter* filter);
	virtual void RemoveFilter(IFilter* filter);
	void RemoveAndDeleteAll();

	virtual void CheckForDeletion();

	// Geerbt über IFilter
	virtual void CustomMenu() override;
	virtual std::set<FilterData> Filter(std::set<FilterData> d) override;
	virtual bool Updated() override;
	virtual void ResetUpdateState() override;
	virtual char* GetSerializeName() override;
	virtual void SetOpen(bool open) override;
	virtual void DragDropTarget() override;
};
#endif
