#ifndef TASK_H
#define TASK_H

class Task {
public:
	Task() {}
	virtual ~Task() {}
	virtual void run() = 0;
};

#endif