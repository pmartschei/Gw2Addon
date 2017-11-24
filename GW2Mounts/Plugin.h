#ifndef PLUGIN_H
#define PLUGIN_H
#include "PluginBase.h"

class Plugin {
protected:
	PluginBase* _base;
	void LogString(LogLevel level, std::string log);
public:
	virtual const char* GetName() = 0;
	virtual void Init(PluginBase* base) {
		_base = base; 
		LogString(LogLevel::Info, std::string(GetName()) + " initialization started");
	};
	virtual void PreCreateDevice() {};
	virtual void PostCreateDevice() {};
	virtual void PreReset() {};
	virtual void PostReset() {};
	virtual void Render() {};
	virtual void PluginMain() {};
};
#endif