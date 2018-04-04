#include "NoDeleteWarningPlugin.h"
#include "InventoryData.h"

void NoDeleteWarningPlugin::Init()
{
	Plugin::Init();
}

void NoDeleteWarningPlugin::PluginMain()
{
	uint32_t updateIndex;
	InventoryData* inventory = PluginBase::GetInstance()->GetInventory(&updateIndex);
	if (updateIndex != lastUpdateIndex && inventory && activated) {
		lastUpdateIndex = updateIndex;
		for (int i = 0; i < inventory->realSize; i++) {
			ItemStackData* item = inventory->itemStackDatas[i];
			if (item && item->itemData) {
				if (item->itemData->HasFlags(ItemFlags::DeleteWarning)) {
					item->itemData->SetFlags(item->itemData->GetFlags() ^ ItemFlags::DeleteWarning);
					changedItems.push_back(item->itemData);
				}
			}
		}
	}
	if (!activated) {
		ClearChanges();
	}
}

void NoDeleteWarningPlugin::ClearChanges() {
	for (auto it = changedItems.begin(); it != changedItems.end(); ++it) {
		ItemData* item = (*it);
		item->SetFlags(item->GetFlags() | ItemFlags::DeleteWarning);
	}
	changedItems.clear();
}

void NoDeleteWarningPlugin::RenderOptions()
{
	if (ImGui::CollapsingHeader(GetName())) {
		if (RenderCheckbox("Activated", &activated)) {
			lastUpdateIndex = 0; //force update
		}
	}
}

const char * NoDeleteWarningPlugin::GetName()
{
	return "NoDeleteWarning";
}
