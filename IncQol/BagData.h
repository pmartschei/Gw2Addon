#ifndef BAG_DATA_H
#define BAG_DATA_H
#include "ItemStackData.h"
class BagData : public ItemStackData {
public:
	int bagSize = 0;
	bool noSellOrSort = false;
};
#endif