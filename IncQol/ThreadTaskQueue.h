#ifndef THREAD_TASK_QUEUE_H
#define THREAD_TASK_QUEUE_H
#include "TaskQueue.h"
#include <thread>
class ThreadTaskQueue
{
public:
	ThreadTaskQueue(int size = 4);
	~ThreadTaskQueue();
	void addTask(Task* task);
private:
	TaskQueue queue;
	std::thread* threads;
	int threadSize;
};
#endif