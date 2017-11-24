#include "ItemFilterFactory.h"

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
		new GroupFilter()
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
