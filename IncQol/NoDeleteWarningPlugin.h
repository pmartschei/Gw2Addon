#ifndef NO_DELETE_WARNING_PLUGIN_H
#define NO_DELETE_WARNING_PLUGIN_H

#include "Plugin.h"
#include <vector>
class NoDeleteWarningPlugin : public Plugin
{
private:
	std::vector<ItemData*> changedItems;
	void ClearChanges();
	bool activated = false;
	uint32_t lastUpdateIndex;
public:
	// Geerbt über Plugin
	virtual void Init() override;
	virtual void PluginMain() override;
	virtual void RenderOptions() override;
	virtual const char * GetName() override;
};
#endif

