#ifndef ITEM_FILTER_FACTORY_H
#define ITEM_FILTER_FACTORY_H
#include <string>
#include <map>
#include "IDItemFilter.h"
#include "IDRangeItemFilter.h"
#include "LevelItemFilter.h"
#include "LevelRangeItemFilter.h"
#include "RarityItemFilter.h"
#include "RarityRangeItemFilter.h"
#include "TypeItemFilter.h"
#include "GroupFilter.h"
#include "FilterFactory.h"

class ItemFilterFactory : public FilterFactory<IFilter>
{
public:
	ItemFilterFactory();
	static ItemFilterFactory GetItemFilterFactory();
};
#endif
