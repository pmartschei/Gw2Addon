#ifndef INVENTORY_DATA_H
#define INVENTORY_DATA_H
#include <vector>
#include "ItemStackData.h"
#include "BagData.h"
class InventoryData {
public:
	InventoryData();
	~InventoryData();
	int size = 0;
	int bagCount = 0;
	int slotsPerBag = 0;
	std::vector<ItemStackData> itemStackDatas;
	std::vector<BagData> bagDatas;
};
#endif