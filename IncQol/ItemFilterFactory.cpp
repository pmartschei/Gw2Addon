#include "ItemFilterFactory.h"
#include "IDItemFilter.h"
#include "IDRangeItemFilter.h"
#include "LevelItemFilter.h"
#include "LevelRangeItemFilter.h"
#include "RarityItemFilter.h"
#include "RarityRangeItemFilter.h"
#include "TypeItemFilter.h"
#include "GroupFilter.h"
#include "TradingPostValueFilter.h"
#include "TypeItemMultiFilter.h"
#include "SubTypeItemFilter.h"
#include <vector>

ItemFilterFactory::ItemFilterFactory()
{
	std::vector<IFilter*> filters = {
		new IDItemFilter(),
		new IDRangeItemFilter(),
		new LevelItemFilter(),
		new LevelRangeItemFilter(),
		new RarityItemFilter(),
		new RarityRangeItemFilter(),
		new TypeItemFilter(),
		new GroupFilter(),
		new TradingPostValueFilter(),
		new TypeItemMultiFilter(),
		new SubTypeItemFilter(ItemType::None),
	};
	for (int i = 0; i < filters.size(); i++) {
		IFilter* filter = filters[i];
		set(filter->GetSerializeName(), filter);
	}
}


ItemFilterFactory ItemFilterFactory::GetItemFilterFactory()
{
	static ItemFilterFactory ImportItemFilterFactory = ItemFilterFactory();
	return ImportItemFilterFactory;
}
