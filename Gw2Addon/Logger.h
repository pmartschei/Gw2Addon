#ifndef LOGGER_H
#define LOGGER_H
#include "Utility.h"
#include "main.h"
#include <time.h>

enum LogLevel {
	Debug = 0,
	Info = 1,
	Error = 2,
	Critical = 3
};
class Logger {
private:
	Logger() {};
protected:
	static std::fstream file;
	static LogLevel minLevel;
	static void LogTime();
	static void LogInfo(LogLevel level);
public:
	static void SetMinLevel(LogLevel level);
	static void Init(const char* filePath);
	static void Close();
	static void LogString(LogLevel level, std::string info, std::string log);
};
#endif