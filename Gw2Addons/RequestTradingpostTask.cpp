#include "RequestTradingpostTask.h"
#include "HttpDownloader.h"
#include "ItemData.h"
#include "PluginBase.h"
#include "JSON\JSON.h"
#include "JSON\JSONValue.h"

RequestTradingpostTask::RequestTradingpostTask(ItemData data) : data(data)
{
}
void RequestTradingpostTask::run()
{
	std::string idStr = std::to_string(data.id);
	HttpDownloader downloader;
	std::string jsonInfo = downloader.download(PluginBase::GetInstance()->GetItemInfoUrl() + idStr);
	if (jsonInfo.empty()) {
		Logger::LogString(LogLevel::Error, MAIN_INFO, "Could not download ItemInfo for Item : " + idStr);
		return;
	}
	return;
	JSONValue* value = JSON::Parse(jsonInfo.c_str());
	JSONObject root = value->AsObject();
	if (root.find(L"vendor_value") == root.end())
		return;

	data.vendorValue = (uint) root[L"vendor_value"]->AsNumber();

	//can theoretically parse anything on api

	jsonInfo = downloader.download(PluginBase::GetInstance()->GetPricesUrl() + idStr);
	if (jsonInfo.empty()) {
		Logger::LogString(LogLevel::Error, MAIN_INFO, "Could not download Prices for Item : " + idStr);
		return;
	}

	value = JSON::Parse(jsonInfo.c_str());
	root = value->AsObject();
	if (root.find(L"buys") == root.end() || root.find(L"sells") == root.end())
		return;

	JSONArray buys = root[L"buys"]->AsArray();

	int limit = min(buys.size(), 3);
	for (int i = 0; i < limit; i++) {

	}

	data.buyTradingPost = 123;
	data.sellTradingPost = 123;
	data.validTradingPostData = true;
	data.lastTradingPostUpdate = GetCurrentTime();
}
