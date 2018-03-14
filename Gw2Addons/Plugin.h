#ifndef PLUGIN_H
#define PLUGIN_H
#include "PluginBase.h"
#include <queue>
#include <mutex>

class Plugin {
protected:
	void LogString(LogLevel level, std::string log);
	std::queue < std::function<void()> > dispatchQueue;
	std::mutex dispatchMutex;
	virtual void Dispatch();
public:
	virtual const char* GetName() = 0;
	virtual void Init() {
		LogString(LogLevel::Info, std::string(GetName()) + " initialization started");
	};
	virtual void PreCreateDevice() {};
	virtual void PostCreateDevice() {};
	virtual void PreReset() {};
	virtual void PostReset() {};
	virtual void Render() {};
	virtual void PreRender();
	virtual void PluginMain() {};
	virtual void RenderOptions() {};
	virtual void AddDispatchWork(std::function<void()> func);
};
#endif