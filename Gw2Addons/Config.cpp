#include "Config.h"

void Config::Init()
{
	Config* c = GetInstance();
	c->iniFile = new CSimpleIniA(true);
	std::string addonFolder = GetAddonFolder();
	if (SHCreateDirectoryExA(nullptr, addonFolder.c_str(), nullptr) == 0) {
		Logger::LogString(LogLevel::Debug, MAIN_INFO, "Created config directory : ");
	}
	addonFolder = addonFolder.append(CONFIG_FILENAME);
	size_t size = addonFolder.size() + 1;
	char* cptr = new char[size];
	strcpy_s(cptr, size, addonFolder.c_str());
	c->configLocation = cptr;
	Logger::LogString(LogLevel::Debug, MAIN_INFO, std::string("Config file location : ").append(c->configLocation));
}


bool Config::Load()
{
	Config* c = GetInstance(); 
	if (!FileExists(c->configLocation)) {
		FILE* fp;
		fopen_s(&fp,c->configLocation, "w");
		if (fp) {
			fclose(fp);
		}
		else {
			return false;
		}
	}
	c->iniFile->LoadFile(c->configLocation);
	return true;
}

bool Config::Save()
{
	Config* c = GetInstance();
	return (c->iniFile->SaveFile(c->configLocation) == SI_Error::SI_OK);
}

bool Config::LoadBool(const char* section, const char* key, bool defaultBool)
{
	Config* c = GetInstance();
	if (!c->iniFile) return defaultBool;
	return c->iniFile->GetBoolValue(section, key, defaultBool);
}

double Config::LoadDouble(const char* section, const char* key, double defaultDouble)
{
	Config* c = GetInstance();
	if (!c->iniFile) return defaultDouble;
	return c->iniFile->GetDoubleValue(section, key, defaultDouble);
}

long Config::LoadLong(const char* section, const char* key, long defaultLong)
{
	Config* c = GetInstance();
	if (!c->iniFile) return defaultLong;
	return c->iniFile->GetLongValue(section, key, defaultLong);
}

const char* Config::LoadText(const char* section, const char* key, const char* defaultText)
{
	Config* c = GetInstance();
	if (!c->iniFile) return defaultText;
	return c->iniFile->GetValue(section, key, defaultText);
}

std::set<uint> Config::LoadKeyBinds(const char * section, const char * key, std::set<uint> defaultKeys)
{
	Config* c = GetInstance();
	if (!c->iniFile) return defaultKeys;
	const char* keys = c->iniFile->GetValue(section, key, '\0');
	if (keys == '\0') {
		return defaultKeys;
	}
	std::set<uint> result;
	std::stringstream ss(keys);
	while (ss.good())
	{
		std::string substr;
		std::getline(ss, substr, ',');
		if (substr.size() > 0) {
			int val = std::stoi(substr);
			result.insert((uint)val);
		}
	}
	return result;
}

void Config::SaveBool(const char* section, const char* key, bool b)
{
	Config* c = GetInstance();
	if (!c->iniFile) return;
	c->iniFile->SetBoolValue(section, key, b);
}

void Config::SaveDouble(const char* section, const char* key, double d)
{
	Config* c = GetInstance();
	if (!c->iniFile) return;
	c->iniFile->SetBoolValue(section, key, d);
}

void Config::SaveLong(const char* section, const char* key, long l)
{
	Config* c = GetInstance();
	if (!c->iniFile) return;
	c->iniFile->SetLongValue(section, key, l);
}

void Config::SaveText(const char* section, const char* key, const char* text)
{
	Config* c = GetInstance();
	if (!c->iniFile) return;
	c->iniFile->SetValue(section, key, text);
}

void Config::SaveKeyBinds(const char * section, const char * key, std::set<uint> keys)
{
	Config* c = GetInstance();
	if (!c->iniFile) return;
	std::string keyS = "";
	for (auto it = keys.begin(), end = keys.end(); it != end;) {
		keyS.append(std::to_string((*it)));
		++it;
		if (it != end) {
			keyS.append(", ");
		}
	}
	c->iniFile->SetValue(section, key, keyS.c_str());
}
