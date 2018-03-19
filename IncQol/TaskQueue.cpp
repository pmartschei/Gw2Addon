#include "TaskQueue.h"

void TaskQueue::addTask(Task * task)
{
	{
		std::unique_lock<std::mutex> lock(qMtx);
		tasks.push(task);
		wCond.notify_one();
	}
}

Task * TaskQueue::nextTask()
{
	Task* res = 0;
	{
		std::unique_lock<std::mutex> lock(qMtx);
		while (!hasWork() || finished) {
			wCond.wait(lock);
		}
		if (finished) return res;
		res = tasks.front();
		tasks.pop();
	}
	return res;
}

bool TaskQueue::hasWork()
{
	return (tasks.size() > 0);
}

void TaskQueue::finish() 
{
	finished = true;
	wCond.notify_all();
}
