#include "Plugin.h"

void Plugin::LogString(LogLevel level,std::string log)
{
	Logger::LogString(level,std::string(GetName()),log);
}

void Plugin::PreRender()
{
	Dispatch();
	Render();
}

void Plugin::Dispatch()
{
	{
		std::unique_lock<std::mutex> lock(dispatchMutex);
		while (!dispatchQueue.empty()) {
			std::function<void()> func = dispatchQueue.front();
			dispatchQueue.pop();
			func();
		}
	}
}

void Plugin::AddDispatchWork(std::function<void()> func)
{
	{
		std::unique_lock<std::mutex> lock(dispatchMutex);
		dispatchQueue.push(func);
	}
}
