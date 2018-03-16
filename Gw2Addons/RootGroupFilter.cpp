#include "RootGroupFilter.h"

RootGroupFilter::RootGroupFilter() {
	flags = FilterFlags::Or;
	name = "Root Filter";
	nameCalculated = true;
}

void RootGroupFilter::Render()
{
	RenderChildren();
}
void RootGroupFilter::SerializeContent(tinyxml2::XMLPrinter & printer)
{
	std::vector<IFilter*>::iterator iter;
	for (int i = 0; i < filterIndex; i++) {
		if (subFilters[i])
			subFilters[i]->Serialize(printer);
	}
}
void RootGroupFilter::Serialize(tinyxml2::XMLPrinter & printer)
{
	printer.OpenElement("Filter");
	SerializeContent(printer);
	printer.CloseElement();
}

void RootGroupFilter::Deserialize(tinyxml2::XMLElement * element)
{
	DeserializeContent(element);
}


