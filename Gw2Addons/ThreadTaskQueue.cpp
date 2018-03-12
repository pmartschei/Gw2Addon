#include "ThreadTaskQueue.h"

void run(void* params) {
	Task* t = 0;
	TaskQueue* queue = (TaskQueue*)params;
	while (t = queue->nextTask()) {
		t->run();
		delete t;
	}
}
ThreadTaskQueue::ThreadTaskQueue()
{
	t = std::thread(run,&queue);
}
ThreadTaskQueue::~ThreadTaskQueue()
{
	queue.finish();
}
void ThreadTaskQueue::addTask(Task * task)
{
	queue.addTask(task);
}
