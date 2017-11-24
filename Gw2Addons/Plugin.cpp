#include "Plugin.h"

void Plugin::LogString(LogLevel level,std::string log)
{
	Logger::LogString(level,std::string(GetName()),log);
}
