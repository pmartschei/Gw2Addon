#ifndef _UTILITY_H
#define _UTILITY_H
#include "main.h"

#define CONFIG_FILENAME ("config.ini")

std::wstring s2ws(const std::string & str);

std::string ws2s(const std::wstring & wstr);

std::string ToHex(uintptr_t val);

std::string GetKeyName(unsigned int virtualKey);

void SplitFilename(const tstring & str, tstring * folder, tstring * file);

bool FileExists(const char* path);

std::string GetExeFolder();

std::string GetAddonFolder();

wchar_t *convertCharArrayToLPCWSTR(const char* charArray);

std::vector<std::string> ListFilesInDir(std::string dir, std::string filter = "*.*");

// trim from start (in place)
void ltrim(std::string &s);

// trim from end (in place)
void rtrim(std::string &s);
// trim from both ends (in place)
void trim(std::string &s);
#endif