#pragma once
#include <Windows.h>
class Process {
private:
	HMODULE GetModule(LPCWSTR moduleName);
protected:
	HANDLE _handle;
	DWORD _pid;
	DWORD64 _baseAddress;
	HMODULE _module;
public:
	Process(LPCWSTR name,LPCWSTR moduleName);
	~Process();
	HANDLE GetHandle();
	DWORD GetPID();
	DWORD64 GetBaseAddress();
};