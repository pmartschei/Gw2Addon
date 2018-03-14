#include "InventoryData.h"

InventoryData::InventoryData() 
{
}

InventoryData::~InventoryData()
{
	for (int i = 0; i < realSize; i++) {
		delete itemStackDatas[i];
	}
	delete[] itemStackDatas;
	for (int i = 0; i < bagCount; i++) {
		delete bagDatas[i];
	}
	delete[] bagDatas;
}
