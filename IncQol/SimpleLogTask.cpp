#include "SimpleLogTask.h"
#include "Logger.h"

SimpleLogTask::SimpleLogTask(std::string msg) : msg(msg)
{
}

void SimpleLogTask::run()
{
	Logger::LogString(LogLevel::Info, "SimpleLogTask", msg);
}
