#ifndef _DEF_H
#define _DEF_H

typedef unsigned char uchar;
typedef unsigned int uint;
typedef std::basic_string<TCHAR> tstring;
typedef unsigned __int64 mstime;

#define COM_RELEASE(x) { if((x)) { (x)->Release(); (x) = nullptr; } }



#ifdef _DEBUG
#define LOG_LVL (LogLevel::Debug)
#else
#define LOG_LVL (LogLevel::Info)
#endif
#define ARCH_64BIT
#define CONFIG_FILENAME ("config.ini")
#define POINTER_SIZE (8)
#define MAIN_INFO ("Main")

template <typename T> T CLAMP(const T& value, const T& low, const T& high)
{
	return value < low ? low : (value > high ? high : value);
}

#endif