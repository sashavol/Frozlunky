#pragma once

#include "winheaders.h"
#include "util.h"

#include <string>

#define MAX_PROCESSES 1024

namespace util {
	void InjectDLL(HANDLE process, const std::string& dll_name) {
		HANDLE loadlib_addr = GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
		LPVOID remote_str = (LPVOID)VirtualAllocEx(process, NULL, dll_name.size(), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
		if(remote_str != NULL) {
			WriteProcessMemory(process, remote_str, dll_name.c_str(), dll_name.size(), NULL);
			CreateRemoteThread(process, NULL, NULL, (LPTHREAD_START_ROUTINE)loadlib_addr, remote_str, NULL, NULL);
		}
		CloseHandle(loadlib_addr);
	}

	//Gets process id by name
	DWORD FindProcess(__in_z LPCTSTR lpcszFileName) 
	{ 
	  LPDWORD lpdwProcessIds; 
	  LPTSTR  lpszBaseName; 
	  HANDLE  hProcess; 
	  DWORD   i, cdwProcesses, dwProcessId = 0; 

	  lpdwProcessIds = (LPDWORD)HeapAlloc(GetProcessHeap(), 0, MAX_PROCESSES*sizeof(DWORD)); 
	  if (lpdwProcessIds != NULL) 
	  { 
		if (EnumProcesses(lpdwProcessIds, MAX_PROCESSES*sizeof(DWORD), &cdwProcesses)) 
		{ 
		  lpszBaseName = (LPTSTR)HeapAlloc(GetProcessHeap(), 0, MAX_PATH*sizeof(TCHAR)); 
		  if (lpszBaseName != NULL) 
		  { 
			cdwProcesses /= sizeof(DWORD); 
			for (i = 0; i < cdwProcesses; i++) 
			{ 
			  hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, lpdwProcessIds[i]); 
			  if (hProcess != NULL) 
			  { 
				if (GetModuleBaseName(hProcess, NULL, lpszBaseName, MAX_PATH) > 0) 
				{ 
				  if (!lstrcmpi(lpszBaseName, lpcszFileName)) 
				  { 
					dwProcessId = lpdwProcessIds[i]; 
					CloseHandle(hProcess); 
					break; 
				  } 
				} 
				CloseHandle(hProcess); 
			  } 
			} 
			HeapFree(GetProcessHeap(), 0, (LPVOID)lpszBaseName); 
		  } 
		} 
		HeapFree(GetProcessHeap(), 0, (LPVOID)lpdwProcessIds); 
	  } 
	  return dwProcessId; 
	}

	//taken from http://pastebin.com/6tw0CRrS
	FARPROC GetRemoteProcAddress( HANDLE hProcess, const char *szModuleName, const char *szProcName )
	{
		HMODULE hLocalModule = GetModuleHandleA( szModuleName );
 
		if( hLocalModule == false )
			return (FARPROC)0;
 
		FARPROC fpLocal = GetProcAddress( hLocalModule, szProcName );
 
		if( fpLocal == (FARPROC)0 )
			return (FARPROC)0;
 
		DWORD dwOffset = (DWORD)fpLocal - (DWORD)hLocalModule;
 
		HMODULE hRemoteModuleHandle = GetRemoteModuleHandle( szModuleName, hProcess, false );
 
		if( hRemoteModuleHandle == (HMODULE)0 )
			return (FARPROC)0;
 
		return (FARPROC)((DWORD)hRemoteModuleHandle + dwOffset);
	}
 
	HMODULE GetRemoteModuleHandle(const char *szModuleName, HANDLE hProcess, bool bUsePath )
	{
		HANDLE tlh = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, GetProcessId( hProcess ) );
 
		MODULEENTRY32 modEntry;
   
		modEntry.dwSize = sizeof( MODULEENTRY32 );
 
		Module32First( tlh, &modEntry );
		do
		{
			std::string comp;
			comp.clear();
 
			if(bUsePath){ comp = modEntry.szExePath; } else { comp = modEntry.szModule; }
 
			if( !strcmp( szModuleName, comp.c_str() ) )
			{
				CloseHandle( tlh );
 
				return modEntry.hModule;
			}
		}
		while(Module32Next( tlh, &modEntry ) );
 
		CloseHandle( tlh );
 
		return NULL;
	}
}