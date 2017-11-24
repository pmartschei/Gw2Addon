#ifndef _MAIN_H
#define _MAIN_H
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <imgui.h>
#include <vector>
#include <string>
#include <memory>
#include <list>
#include <set>
#include <iterator>
#include <algorithm>
#include <tchar.h>
#include <sstream>
#include <iostream>
#include <mutex>
#include <d3dx9.h>
#include <functional>
#include <codecvt>
#include <cctype>
#include <fstream>
#include <Shlobj.h>
#include <ctime>

#define COM_RELEASE(x) { if((x)) { (x)->Release(); (x) = nullptr; } }

template <typename T> T CLAMP(const T& value, const T& low, const T& high)
{
	return value < low ? low : (value > high ? high : value);
}

#define POINTER_SIZE (8)
#define MAIN_INFO ("Main")

typedef unsigned char uchar;
typedef unsigned int uint;
typedef std::basic_string<TCHAR> tstring;
typedef unsigned __int64 mstime;

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif
#endif
