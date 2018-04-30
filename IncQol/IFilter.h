#ifndef I_FILTER_H
#define I_FILTER_H

#include <set>
#include <string>
#include <list>
#include <mutex>
#include "ItemStackData.h"
#include "imgui.h"
#include "tinyxml\tinyxml2.h"
#include "FilterFlags.h"
#include "AddonColors.h"

typedef ItemStackData* FilterData;

#define UNIQUE(text,id) ((std::string(text)+"##"+std::to_string(id)).c_str())

#define UNIQUE_NO_DELIMITER(t,id) ((std::string(t)+std::to_string(id)).c_str())

#define DRAG_DROP_PAYLOAD_TYPE_FILTER "_PAYFILT"

class IFilter{
protected:
	std::string name = "Unnamed Filter"; 
	std::list<ItemData*> filteredItemDatas;
	int filteredItemDatasStartY = 0;
	std::recursive_mutex mutex;
	bool nameCalculated = true;
	bool markedForDelete = false;
	bool gotUpdated = false;
	bool isActive = true;
	bool isOpened = true;
	bool supportDrag = true;
	float tabSpace = 0.0f;
	FilterFlags flags = FilterFlags::Null;
	int id;
	virtual void RenderContent() = 0;
	virtual void CustomMenu();
	virtual void SaveFilteredItemDatas(std::set<FilterData> filteredSet);
	virtual void SerializeContent(tinyxml2::XMLPrinter& printer);
	virtual bool DeserializeContent(tinyxml2::XMLElement* element);
	virtual std::string GetName() { return name; }
	std::set<FilterData> InvertSet(std::set<FilterData> fullData,std::set<FilterData> selectedData);
public:
	IFilter();
	virtual std::set<FilterData> Filter(std::set<FilterData> collection);
	virtual bool IsFiltered(FilterData data);
	virtual void Render();
	virtual bool Updated();
	virtual void ResetUpdateState();
	virtual void Serialize(tinyxml2::XMLPrinter& printer);
	virtual bool Deserialize(tinyxml2::XMLElement* element);
	virtual char* GetSerializeName() = 0;
	virtual IFilter* CreateNew() = 0;	
	virtual void DragDropSource();
	virtual void DragDropTarget() {}
	virtual bool HasParent(IFilter* filter);

	virtual void SetOpen(bool open);
	virtual void SetActive(bool active);
	bool IsActive();
	bool IsMarkedForDeletion();
	void SetName(std::string name);

	IFilter* parent;
	static int ID;
};
#endif
