#include "ThreadTaskQueue.h"
#include "Logger.h"

void run(void* params) {
	Task* t = 0;
	TaskQueue* queue = (TaskQueue*)params;
	while (t = queue->nextTask()) {
		try {
			t->run();
		}
		catch (...) {
			//if a task has a error
			Logger::LogString(LogLevel::Error, "ThreadTaskQueue", "A task ("+std::string(typeid(*t).name())+") could not be computed");
		}
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
