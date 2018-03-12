#include "RequestTradingpostTask.h"
#include "HttpDownloader.h"
#include "ItemData.h"
#include "PluginBase.h"

RequestTradingpostTask::RequestTradingpostTask(ItemData * data) : data(data)
{
}
void RequestTradingpostTask::run()
{
	std::string idStr = std::to_string(data->id);
	HttpDownloader downloader;
	std::string jsonInfo = downloader.download(PluginBase::GetInstance()->GetItemInfoUrl() + idStr);
	if (jsonInfo.empty()) {
		Logger::LogString(LogLevel::Error, MAIN_INFO, "Could not download ItemInfo for Item : " + idStr);
		return;
	}

	jsonInfo = downloader.download(PluginBase::GetInstance()->GetPricesUrl() + idStr);
	if (jsonInfo.empty()) {
		Logger::LogString(LogLevel::Error, MAIN_INFO, "Could not download Prices for Item : " + idStr);
		return;
	}
}
