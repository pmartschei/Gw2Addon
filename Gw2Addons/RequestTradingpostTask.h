#ifndef REQUEST_TRADINGPOST_TASK_H
#define REQUEST_TRADINGPOST_TASK_H
#include "Task.h"
class ItemData;

class RequestTradingpostTask : public Task {
public:
	RequestTradingpostTask(ItemData* data);
private:
	ItemData* data;
	// Geerbt �ber Task
	virtual void run() override;
};

#endif