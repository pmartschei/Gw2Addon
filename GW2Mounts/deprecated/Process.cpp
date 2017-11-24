#include "Process.h"
#include <Psapi.h>
#include <string>

Process::Process(LPCWSTR name, LPCWSTR moduleName)
{
	HWND hwnd = FindWindow(nullptr, name);
	GetWindowThreadProcessId(hwnd, &_pid);
	_handle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, 0, _pid);
	if (_handle) {
		_module = GetModule(moduleName);
	}
	_baseAddress = (DWORD64)_module;
}

Process::~Process()
{
}

HANDLE Process::GetHandle() {
	return _handle;
}

DWORD Process::GetPID() {
	return _pid;
}

DWORD64 Process::GetBaseAddress()
{
	return _baseAddress;
}

HMODULE Process::GetModule(LPCWSTR moduleName)
{
	HMODULE hMods[1024];
	DWORD cbNeeded;
	unsigned int i;

	if (!moduleName) {
		TCHAR szModName[MAX_PATH];
		if (GetModuleFileNameEx(_handle, NULL, szModName, sizeof(szModName) / sizeof(TCHAR))) {
			moduleName = szModName;
		}
		else {
			return nullptr;
		}
	}

	if (EnumProcessModules(_handle, hMods, sizeof(hMods), &cbNeeded))
	{
		for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			TCHAR szModName[MAX_PATH];
			if (GetModuleFileNameEx(_handle, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
			{
				std::wstring szMod = szModName;
				if (szMod.find(moduleName) != std::string::npos)
				{
					return hMods[i];
				}
			}
		}
	}
	return nullptr;
}
