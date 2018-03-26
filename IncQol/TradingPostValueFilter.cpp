#include "TradingPostValueFilter.h"
#include "PluginBase.h"
#include "Utility.h"

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
	printer.PushAttribute("tradingMode",tradingModus);
	printer.PushAttribute("tradingType", tradingType);
	printer.PushAttribute("value", value);
	printer.PushAttribute("sellable", onlyTradingPostSellable);
}

void TradingPostValueFilter::DeserializeContent(tinyxml2::XMLElement * element)
{
	tradingModus = (TradingPostMode)element->IntAttribute("tradingMode", TradingPostMode::Percent);
	tradingModus = (TradingPostMode)CLAMP(tradingModus, TradingPostMode::Percent, (TradingPostMode)(TradingPostMode::ModesCount - 1));
	tradingType = (TradingPostType)element->IntAttribute("tradingType", TradingPostType::Sell);
	tradingType = (TradingPostType)CLAMP(tradingType, TradingPostType::Sell, (TradingPostType)(TradingPostType::TypesCount - 1));
	int min = 120;
	int max = 1000;
	if (tradingModus == TradingPostMode::Flat) {
		min = 50;
		max = 10000;
	}
	value = CLAMP(element->IntAttribute("value", min), min, max);
	onlyTradingPostSellable = element->BoolAttribute("sellable", false);
	name = GetName();
}

std::string TradingPostValueFilter::GetName()
{
	return "TP Filter ("+std::to_string(value)+(tradingModus == TradingPostMode::Percent?" %":"")+")";
}

bool TradingPostValueFilter::IsFiltered(FilterData data)
{
	if (!PluginBase::tpApiEnabled) return false;
	if (onlyTradingPostSellable && !data->tradingpostSellable) return false;

	float tpValue = data->itemData->buyTradingPost;
	if (tradingType == TradingPostType::Sell) {
		tpValue = data->itemData->sellTradingPost;
	}

	if (tpValue == 0.0f) return false;

	if (data->itemData->minTPValue > tpValue) {
		return true;
	}
	if (tradingModus == TradingPostMode::Percent) {
		if ((float)data->itemData->vendorValue * (value / 100.0f) > tpValue) {
			return true;
		}
	} 
	else if (tradingModus == TradingPostMode::Flat) {
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
	if (tradingModus == TradingPostMode::Flat) {
		format = "%.0f";
		min = 50;
		max = 10000;
	}
	int before = value;
	bool sliderUpdate = ImGui::SliderInt(UNIQUE_NO_DELIMITER("##value", id), &value,min,max,format);
	float wheel = ImGui::GetIO().MouseWheel;
	if (ImGui::GetIO().KeyCtrl && wheel != 0.0f && ImGui::IsItemHovered() && !ImGui::IsAnyItemActive()) {
		value += (int)wheel;
		gotUpdated = true;
	}
	value = CLAMP(value, min, max);
	if (sliderUpdate && value != before) gotUpdated = true;
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
	ImGui::Text("Mode : ");
	RenderTooltip(std::vector<TooltipColor>{TooltipColor("Percentage -> vendorValue * value"), TooltipColor("Added -> vendorValue + value")});
	ImGui::SameLine(tabSpace);
	ImGui::PushItemWidth(-1);
	int v = tradingModus;
	if( ImGui::Combo(UNIQUE_NO_DELIMITER("##mode", id), &v, &TradingModes[0], TradingPostMode::ModesCount)){
		int min = 120;
		int max = 1000;
		if (v == TradingPostMode::Flat) {
			min = 50;
			max = 10000;
		}
		value = CLAMP(value, min, max);
		gotUpdated = true;
	}
	tradingModus = (TradingPostMode)v;
	ImGui::PopItemWidth();

	ImGui::Text("Types : ");
	RenderTooltip(std::vector<TooltipColor>{TooltipColor("When comparing the item vendor value with the trading post value, this option decides whether to use the buy or the sell value."),
		TooltipColor("When there are no buy entries for this item, the sell value will be used automatically.")});
	ImGui::SameLine(tabSpace);
	ImGui::PushItemWidth(-1);
	v = tradingType;
	gotUpdated |= (ImGui::Combo(UNIQUE_NO_DELIMITER("##types", id), &v, &TradingTypes[0], TradingPostType::TypesCount));
	tradingType = (TradingPostType)v;
	ImGui::PopItemWidth();

	ImGui::Text("Only Sellable : ");
	RenderTooltip(std::vector<TooltipColor>{TooltipColor("When activated, only consider items which can be sold on the tradingpost. Soulbound/Account Bound items counts as NOT sellable.")});
	ImGui::SameLine(tabSpace);
	ImGui::PushItemWidth(-1);

	if (ImGui::Checkbox(UNIQUE_NO_DELIMITER("##onlySellable", id), &onlyTradingPostSellable)) {
		gotUpdated = true;
	}
	ImGui::PopItemWidth();

	ImGui::Text("Value : ");
	RenderTooltip(std::vector<TooltipColor>{TooltipColor("Use Ctrl + Left Mouse for digit input"), TooltipColor("Or use Ctrl + Mouse Wheel for alternative input"),TooltipColor("\n"), TooltipColor("Only items are filtered when their vendor value modified by this value is higher than their buy/sell value.")
		, TooltipColor("\n\n"), TooltipColor("As an example:"), TooltipColor("vendor value = 20"), TooltipColor("filter value (Percentage) = 200%%"), TooltipColor("sell value = 30"), TooltipColor("\n"), TooltipColor("Item will be filtered because 20 * 200%% >= 30")});
	ImGui::SameLine(tabSpace);
	ImGui::PushItemWidth(-1);
	RenderInput(value);
	ImGui::PopItemWidth();
	name = GetName();
}
