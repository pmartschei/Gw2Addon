#ifndef FILTER_PLUGIN_H
#define FILTER_PLUGIN_H
#include "Plugin.h"
#include "Window.h"
#include "RootGroupFilter.h"
#include "Config.h"

#define MAX_TOOLTIP_SIZE "maxToolTipSize"
#define ENABLE_ADVANCED_OPTIONS "enableAdvancedOptions"
#define ENABLE_ASCENDED_RARITY "enableAscendedRarity"
#define MAX_SELL_RETRY "maxSellRetry"

class Window;
class FilterPlugin : public Plugin
{
private:

	RootGroupFilter* root;
	RootGroupFilter* stdFilter;
	RootGroupFilter* ascFilter;

	Window* window;
	uint lastUpdateIndex;
	uintptr_t* proxyVendorLocation;
	bool vendorSuccessful = false;
	std::set<FilterData> filteredCollection;
	std::list<ItemData*> filteredItemDatas;
	int filteredItemDatasStartY = 0;

	////SAVEDATA
	KeyBindData* copyItemKeyBind = new KeyBindData();
	KeyBindData* openWindow = new KeyBindData();
	bool advancedOptions = false;
	bool ascendedRarity = false;
	static const int MAX_RETRY = 200;
	static const int MIN_RETRY = 10;
	int maxRetry = 100;

	////FILE IMPORT EXPORT
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

	////SKIP IDS
	int curRetry = 0;
	const int defaultRetry = 100;
	int lastSlot = -1;
	std::vector<int> skipUnsellableIds;

	void HookVendorFunc();
	void RenderMenu();
	void UpdateFilter();
	void AddHoveredItemToFilter();
	void ReloadFilterFiles();
	bool SaveFilterAs(RootGroupFilter* filter, const char* name);
	bool LoadFilterFrom(RootGroupFilter* filter, const char *name, bool append = false);
public:
	static uintptr_t* vendorSource;
	static uintptr_t* lastCallPtr;

	// Geerbt über Plugin
	virtual const char * GetName() override;
	virtual void Init() override;
	virtual void Render() override;
	virtual void PluginMain() override;
	virtual void RenderOptions() override;

	////SAVEDATA
	static int maxTooltipSize;
	static const int MAX_TOOLTIP = 50;
	static const int MIN_TOOLTIP = 10;
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