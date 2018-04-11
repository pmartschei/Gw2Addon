#ifndef _MAIN_H
#define _MAIN_H
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

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

#define VERSION ("1.0.2")

template <typename T> T CLAMP(const T& value, const T& low, const T& high)
{
	return value < low ? low : (value > high ? high : value);
}
#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif
#endif

