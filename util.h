#pragma once

#include "winheaders.h"

#include <string>

#define MAX_PROCESSES 1024

namespace util {
	void InjectDLL(HANDLE process, const std::string& dll_name);
	//Gets process id by name
	DWORD FindProcess(__in_z LPCTSTR lpcszFileName);

	FARPROC GetRemoteProcAddress(HANDLE hProcess, const char* szModuleName, const char* szProcName);
	HMODULE GetRemoteModuleHandle(const char* szModuleName, HANDLE hProcess, bool bUsePath);
}