#include "TradingPostValueFilter.h"

TradingPostValueFilter::TradingPostValueFilter()
{
	value = 0;
	name = GetName();
}

char * TradingPostValueFilter::GetSerializeName()
{
	return "TradingPostValueFilter";
}

IFilter * TradingPostValueFilter::CreateNew()
{
	return new TradingPostValueFilter();
}

void TradingPostValueFilter::SerializeContent(tinyxml2::XMLPrinter & printer)
{
	printer.PushAttribute("value", value);
}

void TradingPostValueFilter::DeserializeContent(tinyxml2::XMLElement * element)
{
	value = CLAMP(element->IntAttribute("value", 20), 20, 1000);
	name = GetName();
}

std::string TradingPostValueFilter::GetName()
{
	return "Trading Post Filter";
}

bool TradingPostValueFilter::IsFiltered(FilterData data)
{
	if (!data->tradingpostSellable) return false;

	float tpValue = data->itemData->buyTradingPost;
	if (modus == TradingPostModus::Sell) {
		tpValue = data->itemData->sellTradingPost;
	}

	if (tpValue == 0.0f) return false;

	if (data->itemData->vendorValue * (1+value / 100.0f) > tpValue) {
		return true;
	}

	return false;
}

void TradingPostValueFilter::RenderInput(int & value)
{
	gotUpdated |= ImGui::SliderInt(UNIQUE_NO_DELIMITER("##level", id), &value,20,1000,"%.0f %%");
}

void TradingPostValueFilter::RenderContent()
{
	ImGui::Text("Modus : ");
	ImGui::SameLine(tabSpace);
	ImGui::PushItemWidth(-1);
	int v = modus;
	gotUpdated |= ImGui::Combo(UNIQUE_NO_DELIMITER("##modus", id), &v, &TradingModus[0], TradingPostModus::ModusCount);
	modus = (TradingPostModus)v;
	ImGui::PopItemWidth();

	ImGui::Text("Value (Percent): ");
	ImGui::SameLine(tabSpace);
	ImGui::PushItemWidth(-1);
	RenderInput(value);
	ImGui::PopItemWidth();
	name = GetName();
}
