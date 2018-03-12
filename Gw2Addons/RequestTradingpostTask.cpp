#include "RequestTradingpostTask.h"
#include "HttpDownloader.h"
#include "ItemData.h"

RequestTradingpostTask::RequestTradingpostTask(ItemData * data) : data(data)
{
}
void RequestTradingpostTask::run()
{
	HttpDownloader downloader;
	std::string json = downloader.download(std::string("https://api.guildwars2.com/v2/items/")+std::to_string(data->id));
	if (!json.empty()) {
		
	}
}
