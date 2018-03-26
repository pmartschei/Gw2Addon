#include "InventoryData.h"
#include "Logger.h"
InventoryData::InventoryData() 
{
}

//InventoryData::InventoryData(const InventoryData & copy)
//{
//	this->bagCount = copy.bagCount;
//	this->slotsPerBag = copy.slotsPerBag;
//	this->realSize = copy.realSize;
//	this->size = copy.size;
//	if (realSize > 0) {
//		itemStackDatas = new ItemStackData*[realSize];
//		for (int i = 0; i < realSize; i++) {
//			itemStackDatas[i] = new ItemStackData(*copy.itemStackDatas[i]);
//		}
//	}
//	if (bagCount > 0) {
//		bagDatas = new BagData*[bagCount];
//		for (int i = 0; i < bagCount; i++) {
//			bagDatas[i] = new BagData(*copy.bagDatas[i]);
//		}
//	}
//}

InventoryData::~InventoryData()
{
	if (itemStackDatas) {
		for (int i = 0; i < realSize; i++) {
			delete itemStackDatas[i];
		}
		delete[] itemStackDatas;
	}
	if (bagDatas) {
		for (int i = 0; i < bagCount; i++) {
			delete bagDatas[i];
		}
		delete[] bagDatas;
	}
}
