#include "Logger.h"
#include "Utility.h"
#include <codecvt>

LogLevel Logger::minLevel = LogLevel::Info;
std::fstream Logger::file;

void Logger::LogTime()
{
	char time[20];
	auto t = std::time(nullptr);
	tm tm;
	localtime_s(&tm, &t);
	strftime(time, sizeof(time), "[%H:%M:%S]", &tm);
	file << time;
}

void Logger::LogInfo(LogLevel level)
{
	switch (level)
	{
	case Info:
		file << "[Info]";
		break;
	case Error:
		file << "[Error]";
		break;
	case Critical:
		file << "[Critical]";
		break;
	case Debug:
		file << "[Debug]";
		break;
	default:
		break;
	}
}


void Logger::SetMinLevel(LogLevel level)
{
	minLevel = level;
}

void Logger::Init(const char * filePath)
{
	SHCreateDirectoryExA(nullptr, GetAddonFolder().c_str(), nullptr);
	file = std::fstream(GetAddonFolder().append(filePath).c_str(),std::ios::out | std::ios::trunc);
	if (!file) {
		Logger::LogString(LogLevel::Error, MAIN_INFO, "Could not open/create logfile : " +GetAddonFolder().append(filePath));
	}
}

void Logger::Close()
{
	if (file)
		file.close();
}

void Logger::LogString(LogLevel level, std::string info,std::string log)
{
	if (level < minLevel) return;
	LogTime();
	LogInfo(level);
	file << "[" << info.c_str() << "] ";
	file << log.c_str() << std::endl;
	file.flush();
}
