#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include "Task.h"

class TaskQueue {
public:
	void addTask(Task* task);
	Task* nextTask();
	void finish();
private:
	bool hasWork();
	std::queue<Task*> tasks;
	std::mutex qMtx;
	std::condition_variable wCond;
	bool finished = false;
};

#endif