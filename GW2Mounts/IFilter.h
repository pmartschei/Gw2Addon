#ifndef I_FILTER_H
#define I_FILTER_H

#include <set>
#include "ItemStackData.h"
#include "imgui.h"
#include <string>
#include <algorithm>
#include "tinyxml\tinyxml2.h"
#include "FilterFlags.h"

#define UNIQUE(text,id) ((std::string(text)+"##"+std::to_string(id)).c_str())

#define UNIQUE_NO_DELIMITER(t,id) ((std::string(t)+std::to_string(id)).c_str())

class IFilter{
protected:
	std::string name = "Unnamed Filter";
	bool nameCalculated = true;
	bool markedForDelete = false;
	bool gotUpdated = false;
	bool isActive = true;
	bool isOpened = true;
	FilterFlags flags = FilterFlags::And;
	int filteredItems = 0;
	virtual void RenderContent() = 0;
	virtual void CustomMenu();
	int id;
	virtual void SerializeContent(tinyxml2::XMLPrinter& printer);
	virtual void DeserializeContent(tinyxml2::XMLElement* element);
public:
	IFilter();
	virtual std::set<ItemStackData> Filter(std::set<ItemStackData> collection);
	virtual bool IsFiltered(ItemStackData data);
	virtual void Render();
	virtual bool Updated();
	virtual void ResetUpdateState();
	virtual void Serialize(tinyxml2::XMLPrinter& printer);
	virtual void Deserialize(tinyxml2::XMLElement* element);
	virtual char* GetSerializeName() = 0;
	virtual IFilter* CreateNew() = 0;

	virtual void SetOpen(bool open);
	virtual void SetActive(bool active);
	bool IsActive();
	bool IsMarkedForDeletion();
	void SetName(std::string name);
	int GetFilteredCount();
	static int ID;
};
#endif
