#include "RequestTradingpostTask.h"
#include "HttpDownloader.h"
#include "ItemData.h"
#include "PluginBase.h"
#include "JSON\JSON.h"
#include "JSON\JSONValue.h"
#include <string>

RequestTradingpostTask::RequestTradingpostTask(ItemData* data) : data(data)
{
}
void RequestTradingpostTask::run()
{
	if (!data) return;
	if (!data->IsOldTradingPostData()) {
		data->updateTaskActive = false;
		return;
	}
	data->lastTradingPostUpdate = GetCurrentTime();
	data->updateTaskActive = false;
	std::string idStr = std::to_string(data->id);
	HttpDownloader downloader;
	std::string jsonInfo = downloader.download(PluginBase::GetInstance()->GetItemInfoUrl() + idStr);
	if (jsonInfo.empty()) {
		Logger::LogString(LogLevel::Error, MAIN_INFO, "Could not download ItemInfo for Item : " + idStr);
		return;
	}

	JSONValue* value = JSON::Parse(jsonInfo.c_str());
	if (!value) return;
	JSONObject root = value->AsObject();
	if (root.find(L"vendor_value") == root.end()) {
		delete value;
		return;
	}

	data->vendorValue = (uint) root[L"vendor_value"]->AsNumber();
	data->minTPValue = (int)(data->vendorValue / 0.85f);

	delete value;
	//can theoretically parse anything on api

	jsonInfo = downloader.download(PluginBase::GetInstance()->GetPricesUrl() + idStr);
	if (jsonInfo.empty()) {
		Logger::LogString(LogLevel::Error, MAIN_INFO, "Could not download Prices for Item : " + idStr);
		return;
	}
	jsonInfo.erase(std::remove(jsonInfo.begin(), jsonInfo.end(), '\n'), jsonInfo.end());
	value = JSON::Parse(jsonInfo.c_str());
	if (!value) return;
	root = value->AsObject();
	if (root.find(L"buys") == root.end() || root.find(L"sells") == root.end()) {
		delete value;
		return;
	}

	JSONArray sells = root[L"sells"]->AsArray();

	uintptr_t quantity = 0;
	uintptr_t price = 0;
	int limit = min(sells.size(), 3);
	for (int i = 0; i < limit; i++) {
		JSONObject sell = sells[i]->AsObject();
		if (sell.find(L"unit_price") != sell.end() && sell.find(L"quantity") != sell.end()) {
			int localQ = (int)sell[L"quantity"]->AsNumber();
			price += (int)sell[L"unit_price"]->AsNumber() * localQ;
			quantity += localQ;
		}
	}
	if (quantity>0) {
		data->sellTradingPost = price / (float)quantity;
	}
	else {
		data->sellTradingPost = 0.0f;
	}

	JSONArray buys = root[L"buys"]->AsArray();

	quantity = 0;
	price = 0;
	limit = min(buys.size(), 3);
	for (int i = 0; i < limit; i++) {
		JSONObject buy = buys[i]->AsObject();
		if (buy.find(L"unit_price") != buy.end() && buy.find(L"quantity") != buy.end()) {
			int localQ = (int)buy[L"quantity"]->AsNumber();
			price += (int)buy[L"unit_price"]->AsNumber() * localQ;
			quantity += localQ;
		}
	}

	if (quantity > 0) {
		data->buyTradingPost = price / (float)quantity;
	}
	else {
		data->buyTradingPost = data->sellTradingPost;
	}
	
	data->validTradingPostData = true;
	delete value;
}
