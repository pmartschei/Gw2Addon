#ifndef SIMPLE_LOG_TASK_H
#define SIMPLE_LOG_TASK_H

#include "Task.h"
#include <string>

class SimpleLogTask : public Task {
public:
	SimpleLogTask(std::string msg);
private:
	std::string msg;
	// Geerbt über Task
	virtual void run() override;
};

#endif