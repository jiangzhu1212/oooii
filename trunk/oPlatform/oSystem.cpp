/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                  *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/
#include <oPlatform/oSystem.h>
#include <oBasis/oStdChrono.h>
#include <oPlatform/oP4.h>
#include <oPlatform/oProcess.h>
#include <oPlatform/oWindows.h>
#include "oWinPowrProf.h"

bool oSystemGetHeapStats(oSYSTEM_HEAP_STATS* _pStats)
{
	MEMORYSTATUSEX ms;
	ms.dwLength = sizeof(MEMORYSTATUSEX);
	if (!GlobalMemoryStatusEx(&ms))
		return oWinSetLastError();
	_pStats->TotalMemoryUsed = ms.dwMemoryLoad;
	_pStats->AvailablePhysical = ms.ullAvailPhys;
	_pStats->TotalPhysical = ms.ullTotalPhys;
	_pStats->AvailableVirtualProcess = ms.ullAvailVirtual;
	_pStats->TotalVirtualProcess = ms.ullTotalVirtual;
	_pStats->AvailablePaged = ms.ullAvailPageFile;
	_pStats->TotalPaged = ms.ullTotalPageFile;
	return true;
}

bool oSystemReboot()
{
	return oWinExitWindows(EWX_REBOOT, SHTDN_REASON_FLAG_PLANNED);
}

bool oSystemShutdown()
{
	return oWinExitWindows(EWX_POWEROFF, SHTDN_REASON_FLAG_PLANNED);
}

bool oSystemSleep()
{
	if (!oWinPowrProf::Singleton()->SetSuspendState(FALSE, TRUE, FALSE))
		return oWinSetLastError();
	return true;
}

bool oSystemAllowSleep(bool _Allow)
{
	switch (oGetWindowsVersion())
	{
		case oWINDOWS_2000:
		case oWINDOWS_XP:
		case oWINDOWS_XP_PRO_64BIT:
		case oWINDOWS_SERVER_2003:
		case oWINDOWS_SERVER_2003R2:
			oASSERT(false, "oSystemAllowSleep won't work on %s.", oAsString(oGetWindowsVersion()));
		default:
			break;
	}

	EXECUTION_STATE next = _Allow ? ES_CONTINUOUS : (ES_CONTINUOUS|ES_SYSTEM_REQUIRED|ES_AWAYMODE_REQUIRED);
	if (!SetThreadExecutionState(next))
		return oWinSetLastError();
	return true;
}

bool oSystemScheduleWakeup(time_t _PosixAbsoluteTime, oTASK _OnWake)
{
	return oScheduleTask("OOOii.Wakeup", _PosixAbsoluteTime, true, _OnWake);
}

bool oSystemExecute(const char* _CommandLine, char* _StrStdout, size_t _SizeofStrStdOut, int* _pExitCode, uint _ExecutionTimeout)
{
	oProcess::DESC desc;
	desc.CommandLine = _CommandLine;
	desc.EnvironmentString = 0;
	desc.StdHandleBufferSize = 4096;
	oRef<threadsafe oProcess> process;
	if (!oProcessCreate(desc, &process))
		return false;

	oTRACE("oExecute: \"%s\"...", oSAFESTRN(_CommandLine));
	process->Start();
	if (!process->Wait(_ExecutionTimeout))
	{
		oErrorSetLast(oERROR_TIMEOUT, "Executing \"%s\" timed out after %.01f seconds.", _CommandLine, static_cast<float>(_ExecutionTimeout) / 1000.0f);
		if (_pExitCode)
			*_pExitCode = oERROR_REDUNDANT;
		return false;
	}

	if (_pExitCode && !process->GetExitCode(_pExitCode))
		*_pExitCode = oERROR_REDUNDANT;

	if (_StrStdout && _SizeofStrStdOut)
	{
		oTRACE("oExecute: Reading from stdout... \"%s\"", oSAFESTRN(_CommandLine));
		size_t sizeRead = process->ReadFromStdout(_StrStdout, _SizeofStrStdOut);
		oASSERT(sizeRead < _SizeofStrStdOut, "");
		_StrStdout[sizeRead] = 0;
	}

	return true;
}

bool oSystemWaitIdle(unsigned int _TimeoutMS)
{
	bool IsSteady = false;
	oStd::chrono::high_resolution_clock::time_point TimeStart = oStd::chrono::high_resolution_clock::now();
	oStd::chrono::high_resolution_clock::time_point TimeCurrent = TimeStart;
	unsigned long long PreviousIdleTime = 0, PreviousSystemTime = 0;
	static const double kLowCPUUsage = 5.0;
	static const unsigned int kNumSamplesAtLowCPUUsageToBeSteady = 10;
	unsigned int nSamplesAtLowCPUUsage = 0;
	
	while (1)
	{
		if (_TimeoutMS != oINFINITE_WAIT && (oSeconds(TimeCurrent - TimeStart) >= oStd::chrono::milliseconds(_TimeoutMS)))
		{
			oErrorSetLast(oERROR_TIMEOUT);
			return false;
		}
		
		if (oWinSystemAllServicesInSteadyState())
		{
			double CPUUsage = oWinSystemCalculateCPUUsage(&PreviousIdleTime, &PreviousSystemTime);
			if (CPUUsage < kLowCPUUsage)
				nSamplesAtLowCPUUsage++;
			else
				nSamplesAtLowCPUUsage = 0;

			if (nSamplesAtLowCPUUsage > kNumSamplesAtLowCPUUsageToBeSteady)
				return true;

			Sleep(200);
		}

		TimeCurrent = oStd::chrono::high_resolution_clock::now();
	}

	oASSERT_NOEXECUTION;
}

bool oSystemGUIUsesGPUCompositing()
{
	return oIsAeroEnabled();
}

bool oSystemGUIEnableGPUCompositing(bool _Enable, bool _Force)
{
	return oEnableAero(_Enable, _Force);
}

bool oSetEnvironmentVariable(const char* _Name, const char* _Value)
{
	return !!SetEnvironmentVariableA(_Name, _Value);
}

char* oSystemGetEnvironmentVariable(char* _Value, size_t _SizeofValue, const char* _Name)
{
	oASSERT((size_t)(int)_SizeofValue == _SizeofValue, "Invalid size");
	size_t len = GetEnvironmentVariableA(_Name, _Value, (int)_SizeofValue);
	return (len && len < _SizeofValue) ? _Value : nullptr;
}

char* oSystemGetEnvironmentString(char* _StrEnvironment, size_t _SizeofStrEnvironment)
{
	char* pEnv = GetEnvironmentStringsA();

	// @oooii-tony: replace nuls with newlines to make parsing this mega-string
	// a bit less obtuse

	char* c = pEnv;
	size_t len = strlen(pEnv);
	while (len)
	{
		c[len] = '\n';
		c += len+1;
		len = strlen(c);
	}

	errno_t err = strcpy_s(_StrEnvironment, _SizeofStrEnvironment, pEnv);
	oVB(FreeEnvironmentStringsA(pEnv));

	if (err)
	{
		oErrorSetLast(err == STRUNCATE ? oERROR_AT_CAPACITY : oERROR_INVALID_PARAMETER, "strcpy to user-specified buffer failed");
		return nullptr;
	}

	return _StrEnvironment;
}

char* oSystemGetPath(char* _StrSysPath, size_t _SizeofStrSysPath, oSYSPATH _SysPath)
{
	bool ensureSeparator = true;
	bool success = true;
	DWORD nElements = oSize32(_SizeofStrSysPath);

	switch (_SysPath)
	{
		case oSYSPATH_APP_FULL:
		{
			ensureSeparator = false;
			DWORD len = GetModuleFileNameA(GetModuleHandle(nullptr), _StrSysPath, nElements);
			if (len == nElements && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				oErrorSetLast(oERROR_AT_CAPACITY);
				success = false;
			}
			break;
		}

		case oSYSPATH_APP:
			if (!oSystemGetPath(_StrSysPath, _SizeofStrSysPath, oSYSPATH_APP_FULL))
				return false;
			*oGetFilebase(_StrSysPath) = 0; 
			break;

		case oSYSPATH_HOST:
			ensureSeparator = false;
			if (!GetComputerNameEx(ComputerNameDnsHostname, _StrSysPath, &nElements))
			{
				oWinSetLastError();
				success = false;
			}
			break;

		case oSYSPATH_CWD: GetCurrentDirectoryA(nElements, _StrSysPath); break;
		case oSYSPATH_SYS: GetSystemDirectoryA(_StrSysPath, nElements); break;
		case oSYSPATH_OS: GetWindowsDirectoryA(_StrSysPath, nElements); break;
		case oSYSPATH_P4ROOT:
		{
			oP4::CLIENT_SPEC cspec;
			success = oP4::GetClientSpec(&cspec);
			if (success)
				strcpy_s(_StrSysPath, _SizeofStrSysPath, cspec.Root);
			break;
		}

		case oSYSPATH_DEV:
		{
			if (!oSystemGetPath(_StrSysPath, _SizeofStrSysPath, oSYSPATH_APP_FULL))
				return false;
			*oGetFilebase(_StrSysPath) = 0;
			success = 0 == strcat_s(_StrSysPath, _SizeofStrSysPath, "../../../"); // assumes $DEV/bin/$PLATFORM/$BUILDTYPE
			break;
		}

		case oSYSPATH_COMPILER_INCLUDES:
		{
			// @oooii-tony: Yes, sorta hard-coded but better than trying to get at this
			// directory elsewhere in user code.
			success = !!oSystemGetEnvironmentVariable(_StrSysPath, _SizeofStrSysPath, "VS90COMNTOOLS");
			if (success)
			{
				oEnsureSeparator(_StrSysPath, _SizeofStrSysPath);
				strcat_s(_StrSysPath, _SizeofStrSysPath, "../../VC/include/");
				oCleanPath(_StrSysPath, _SizeofStrSysPath, _StrSysPath);
			}

			else
				oErrorSetLast(oERROR_NOT_FOUND, "Failed to find compiler include path becayse env var VS90COMNTOOLS does not exist");

			break;
		}

		case oSYSPATH_TMP:
		{
			DWORD len = GetTempPathA(nElements, _StrSysPath);
			if (len > 0 && len <= MAX_PATH)
				break; // otherwise use the desktop (pass through to next case)
		}

		case oSYSPATH_DESKTOP_ALLUSERS:
		case oSYSPATH_DESKTOP:
		{
			int folder = _SysPath == oSYSPATH_DESKTOP ? CSIDL_DESKTOPDIRECTORY : CSIDL_COMMON_DESKTOPDIRECTORY;
			if (nElements < MAX_PATH)
				oTRACE("WARNING: Getting desktop as a system path might fail because the specified buffer is smaller than the platform assumes.");
			if (!SHGetSpecialFolderPathA(0, _StrSysPath, folder, FALSE))
				success = false;
			break;
		}

		case oSYSPATH_DATA:
		{
			// The logic here is this: Search for ./Data and if that fails assume 
			// we're in the dev branch where the exes are in bin/<platform>

			char path[_MAX_PATH];
			if (oSystemGetPath(path, oSYSPATH_APP))
			{
				if (-1 != sprintf_s(_StrSysPath, _SizeofStrSysPath, "%sData/", path))
				{
					if (INVALID_FILE_ATTRIBUTES == GetFileAttributesA(_StrSysPath))
					{
						if (-1 != sprintf_s(_StrSysPath, _SizeofStrSysPath, "%s../../Data/", path))
						{
							if (INVALID_FILE_ATTRIBUTES == GetFileAttributesA(_StrSysPath))
								success = false;
						}

						else
							success = false;
					}
				}

				else
				{
					oErrorSetLast(oERROR_AT_CAPACITY);
					success = false;
				}
			}

			else
				success = false;

			break;
		}

		case oSYSPATH_EXECUTION:
		{
			ensureSeparator = false;
			char hostname[512];
			*hostname = 0;
			if (!oSystemGetPath(hostname, oSYSPATH_HOST))
				success = false;
			else if (-1 == sprintf_s(_StrSysPath, _SizeofStrSysPath, "[%s.%u.%u]", hostname, oProcessGetCurrentID(), oStd::this_thread::get_id()))
			{
				oErrorSetLast(oERROR_AT_CAPACITY);
				success = false;
			}

			break;
		}

		oNODEFAULT;
	}

	if (success)
	{
		if (ensureSeparator)
			oEnsureSeparator(_StrSysPath, _SizeofStrSysPath);
		success = !!oCleanPath(_StrSysPath, _SizeofStrSysPath, _StrSysPath);
	}

	return success ? _StrSysPath : nullptr;
}

char* oSystemFindInPath(char* _ResultingFullPath, size_t _SizeofResultingFullPath, oSYSPATH _SysPath, const char* _RelativePath, const char* _DotPath, oFUNCTION_PATH_EXISTS _PathExists)
{
	if (oSystemGetPath(_ResultingFullPath, _SizeofResultingFullPath, _SysPath))
	{
		size_t len = strlen(_ResultingFullPath);
		if (0 != strcpy_s(_ResultingFullPath + len, _SizeofResultingFullPath - len, _RelativePath))
		{
			oErrorSetLast(oERROR_TRUNCATED);
			return nullptr;
		}

		else if (_PathExists(_ResultingFullPath))
			return _ResultingFullPath;
	}

	return nullptr;
}

char* oSystemFindPath(char* _ResultingFullPath, size_t _SizeofResultingFullPath, const char* _RelativePath, const char* _DotPath, const char* _ExtraSearchPath, oFUNCTION_PATH_EXISTS _PathExists)
{
	if (oIsFullPath(_RelativePath) && _PathExists(_RelativePath) && 0 == strcpy_s(_ResultingFullPath, _SizeofResultingFullPath, _RelativePath))
		return _ResultingFullPath;

	char* success = oSystemFindInPath(_ResultingFullPath, _SizeofResultingFullPath, oSYSPATH_APP, _RelativePath, _DotPath, _PathExists);
	if (!success) success = oSystemFindInPath(_ResultingFullPath, _SizeofResultingFullPath, oSYSPATH_CWD, _RelativePath, _DotPath, _PathExists);
	if (!success) success = oSystemFindInPath(_ResultingFullPath, _SizeofResultingFullPath, oSYSPATH_SYS, _RelativePath, _DotPath, _PathExists);
	if (!success) success = oSystemFindInPath(_ResultingFullPath, _SizeofResultingFullPath, oSYSPATH_OS, _RelativePath, _DotPath, _PathExists);
	if (!success)
	{
		char appPath[_MAX_PATH];
		if (!oSystemFindInPath(appPath, oSYSPATH_CWD, _RelativePath, _DotPath, _PathExists))
		{
			char* envPath = nullptr;
			size_t envPathSize = 0;
			if (0 == _dupenv_s(&envPath, &envPathSize, "PATH"))
				success = oFindInPath(_ResultingFullPath, _SizeofResultingFullPath, envPath, _RelativePath, appPath, _PathExists);
			free(envPath);
		}

		if (!success)
			success = oFindInPath(_ResultingFullPath, _SizeofResultingFullPath, _ExtraSearchPath, _RelativePath, appPath, _PathExists);
	}

	if (!success) success = oSystemFindInPath(_ResultingFullPath, _SizeofResultingFullPath, oSYSPATH_DATA, _RelativePath, _DotPath, _PathExists);

	return success;
}

bool oSetCWD(const char* _CWD)
{
	if (!SetCurrentDirectoryA(_CWD))
	{
		oWinSetLastError();
		return false;
	}

	return true;
}
