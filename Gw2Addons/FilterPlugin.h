#ifndef FILTER_PLUGIN_H
#define FILTER_PLUGIN_H
#include "Plugin.h"
#include "Window.h"
#include "RootGroupFilter.h"
#include "IDItemFilter.h"
#include "LevelItemFilter.h"
#include "RarityItemFilter.h"
#include "TypeItemFilter.h"
#include "hacklib\PatternScanner.h"
#include "main.h"
#include "SellableItemFilter.h"
#include "ItemType.h"
#include "Config.h"
#include "FilterPluginUtility.h"

class Window;
class FilterPlugin : public Plugin
{
private:
	RootGroupFilter* root;
	RootGroupFilter* rootCopy;
	RootGroupFilter* stdFilter;

	Window* window;
	uint32_t lastUpdateIndex;
	void HookVendorFunc();
	uintptr_t* proxyVendorLocation;
	bool vendorSuccessful = false;
	std::set<ItemStackData> filteredCollection;
	KeyBindData* copyItemKeyBind = new KeyBindData();
	KeyBindData* openWindow = new KeyBindData();

	char fileName[64] = "Unnamed Filter";

	bool extraMessageSave = false;
	bool extraMessageLoad = false;
	bool firstOpenedImportFilter = true;
	std::string extraMessage = "";
	ImVec4 extraMessageColor = Addon::Colors[AddonColor_PositiveText];

	int loadIndex = -1;
	const char** filesToLoad;
	int fileCount = 0;
	bool appendLoad = false;

	int lastItemsFilteredCount;
	int curRetry = 0;
	int maxRetry = 100;
	int lastSlot = -1;
	std::vector<int> skipUnsellableIds;
	void RenderMenu();
	void AddHoveredItemToFilter();
	void ReloadFilterFiles();
public:
	virtual void Init() override;
	virtual void Render() override;
	virtual void PluginMain() override;
	bool SaveFilterAs(RootGroupFilter* filter,const char* name);
	bool LoadFilterFrom(RootGroupFilter* filter, const char *name,bool append = false);
	static uintptr_t* vendorSource;
	static uintptr_t* lastCallPtr;

	// Geerbt über Plugin
	virtual const char * GetName() override;
};
struct firstParam {
	int _;
	int id;
	uintptr_t* ptr;
};

struct secondParam {
	int _;
	int id;
	int id2;
};
void __stdcall vendorFunc(firstParam* a, secondParam* b);

#endif