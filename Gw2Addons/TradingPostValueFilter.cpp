#include "TradingPostValueFilter.h"

TradingPostValueFilter::TradingPostValueFilter()
{
	value = 120;
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
	printer.PushAttribute("tradingModus",tradingModus);
	printer.PushAttribute("tradingType", tradingType);
	printer.PushAttribute("value", value);
	printer.PushAttribute("sellable", onlyTradingPostSellable);
}

void TradingPostValueFilter::DeserializeContent(tinyxml2::XMLElement * element)
{
	tradingModus = (TradingPostModus)element->IntAttribute("tradingModus", TradingPostModus::Percent);
	tradingType = (TradingPostType)element->IntAttribute("tradingType", TradingPostType::Sell);
	int min = 120;
	int max = 1000;
	if (tradingModus == TradingPostModus::Flat) {
		min = 50;
		max = 10000;
	}
	value = CLAMP(element->IntAttribute("value", min), min, max);
	onlyTradingPostSellable = element->BoolAttribute("sellable", false);
	name = GetName();
}

std::string TradingPostValueFilter::GetName()
{
	return "TP Filter ("+std::to_string(value)+(tradingModus == TradingPostModus::Percent?" %":"")+")";
}

bool TradingPostValueFilter::IsFiltered(FilterData data)
{
	if (onlyTradingPostSellable && !data->tradingpostSellable) return false;

	float tpValue = data->itemData->buyTradingPost;
	if (tradingType == TradingPostType::Sell) {
		tpValue = data->itemData->sellTradingPost;
	}

	if (tpValue == 0.0f) return false;

	if (data->itemData->minTPValue > tpValue) {
		return false;
	}
	if (tradingModus == TradingPostModus::Percent) {
		if ((float)data->itemData->vendorValue * (value / 100.0f) > tpValue) {
			return true;
		}
	}
	else if (tradingModus == TradingPostModus::Flat) {
		if (data->itemData->vendorValue + value > tpValue) {
			return true;
		}
	}

	return false;
}

void TradingPostValueFilter::RenderInput(int & value)
{
	const char* format = "%.0f %%";
	int min = 120;
	int max = 1000;
	if (tradingModus == TradingPostModus::Flat) {
		format = "%.0f";
		min = 50;
		max = 10000;
	}
	gotUpdated |= ImGui::SliderInt(UNIQUE_NO_DELIMITER("##value", id), &value,min,max,format);
	float wheel = ImGui::GetIO().MouseWheel;
	if (wheel != 0.0f && ImGui::IsItemHovered() && !ImGui::IsAnyItemActive()) {
		value += wheel;
		value = CLAMP(value, min, max);
		gotUpdated = true;
	}
	/*ImVec2 c = ImGui::CalcTextSize("1g 3s 30c");
	ImGui::SameLine(tabSpace +
		(ImGui::GetContentRegionAvailWidth())/2-c.x);
	ImVec2 winPos = ImGui::GetWindowPos();
	ImGui::PushClipRect(ImVec2(winPos.x + tabSpace, winPos.y + ImGui::GetCursorPosY()), ImVec2(winPos.x+ImGui::GetWindowContentRegionWidth(), winPos.y+ImGui::GetCursorPosY()+ImGui::GetTextLineHeightWithSpacing()), true);
	ImGui::Text("1g 3s 30c");
	ImGui::PopClipRect();*/
}

void TradingPostValueFilter::RenderContent()
{
	ImGui::Text("Modus : ");
	ImGui::SameLine(tabSpace);
	ImGui::PushItemWidth(-1);
	int v = tradingModus;
	if( ImGui::Combo(UNIQUE_NO_DELIMITER("##modus", id), &v, &TradingModus[0], TradingPostModus::ModusCount)){
		int min = 120;
		int max = 1000;
		if (v == TradingPostModus::Flat) {
			min = 50;
			max = 10000;
		}
		value = CLAMP(value, min, max);
		gotUpdated = true;
	}
	tradingModus = (TradingPostModus)v;
	ImGui::PopItemWidth();

	ImGui::Text("Types : ");
	ImGui::SameLine(tabSpace);
	ImGui::PushItemWidth(-1);
	v = tradingType;
	gotUpdated |= (ImGui::Combo(UNIQUE_NO_DELIMITER("##types", id), &v, &TradingTypes[0], TradingPostType::TypesCount));
	tradingType = (TradingPostType)v;
	ImGui::PopItemWidth();

	ImGui::Text("Only Sellable : ");
	ImGui::SameLine(tabSpace);
	ImGui::PushItemWidth(-1);

	if (ImGui::Checkbox(UNIQUE_NO_DELIMITER("##onlySellable", id), &onlyTradingPostSellable)) {
		gotUpdated = true;
	}
	ImGui::PopItemWidth();

	ImGui::Text("Value (Percent): ");
	ImGui::SameLine(tabSpace);
	ImGui::PushItemWidth(-1);
	RenderInput(value);
	ImGui::PopItemWidth();
	name = GetName();
}
