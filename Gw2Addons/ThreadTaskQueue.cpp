#include "ThreadTaskQueue.h"

void run(void* params) {
	Task* t = 0;
	TaskQueue* queue = (TaskQueue*)params;
	while (t = queue->nextTask()) {
		t->run();
		delete t;
	}
}
ThreadTaskQueue::ThreadTaskQueue(int size) : threadSize(size)
{
	threads = new std::thread[size];
	for (int i = 0; i < threadSize; i++) {
		threads[i] = std::thread(run, &queue);
	}
}
ThreadTaskQueue::~ThreadTaskQueue()
{
	queue.finish(); 
	for (int i = 0; i < threadSize; i++) {
		threads[i].join();
	}
	delete[] threads;
}
void ThreadTaskQueue::addTask(Task * task)
{
	queue.addTask(task);
}
