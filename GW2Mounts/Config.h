#ifndef _CONFIG_H
#define _CONFIG_H
#include "main.h"
#include "simpleini\SimpleIni.h"
#include "Utility.h"
#include "Logger.h"

class Config
{
private:
	Config();
	Config(Config const&) {};
	void operator=(Config const&) {};
public:
	static bool Load();
	static bool Save();
	static bool LoadBool(const char* section, const char* key, bool defaultBool);
	static double LoadDouble(const char* section, const char* key, double defaultDouble);
	static long LoadLong(const char* section, const char* key, long defaultLong);
	static const char* LoadText(const char* section, const char* key, const char* defaultText);
	static std::set<uint> LoadKeyBinds(const char* section, const char* key, std::set<uint> defaultKeys);
	static void SaveBool(const char* section, const char* key, bool b);
	static void SaveDouble(const char* section, const char* key, double d);
	static void SaveLong(const char* section, const char* key, long l);
	static void SaveText(const char* section, const char* key, const char* text);
	static void SaveKeyBinds(const char* section, const char* key, std::set<uint> keys);

	static Config* GetInstance();

protected:
	// Config file settings
	const char* configLocation;
	CSimpleIniA* iniFile;
};
#endif
