// $(header)
#pragma once
#ifndef oWINPSAPI_h
#define oWINPSAPI_h

#include <oooii/oModule.h>
#include <oooii/oSingleton.h>
#include <oooii/oWindows.h>
#include <psapi.h>

struct oWinPSAPI : oModuleSingleton<oWinPSAPI>
{
	oWinPSAPI();
	~oWinPSAPI();

public:

	BOOL (__stdcall *EnumProcesses)(DWORD* lpidProcess, DWORD cb, LPDWORD lpcbNeeded);
	BOOL (__stdcall *EnumProcessModules)(HANDLE hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded);
	DWORD (__stdcall *GetModuleBaseNameA)(HANDLE hProcess, HMODULE hModule, LPSTR lpBaseName, DWORD nSize);
	BOOL (__stdcall *GetProcessMemoryInfo)(HANDLE Process, PPROCESS_MEMORY_COUNTERS ppsmemCounters, DWORD cb);
	BOOL (__stdcall *GetModuleInformation)(HANDLE hProcess, HMODULE hModule, LPMODULEINFO lpmodinfo, DWORD cb);

protected:
	oHMODULE hPSAPI;
};

#endif
