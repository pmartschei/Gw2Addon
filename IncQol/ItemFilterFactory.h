#ifndef ITEM_FILTER_FACTORY_H
#define ITEM_FILTER_FACTORY_H
#include "FilterFactory.h"
#include "IFilter.h"

class ItemFilterFactory : public FilterFactory<IFilter>
{
public:
	ItemFilterFactory();
	static ItemFilterFactory GetItemFilterFactory();
};
#endif
