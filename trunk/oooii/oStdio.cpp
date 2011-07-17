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
#include <oooii/oStdio.h>
#include <oooii/oAssert.h>
#include <oooii/oPath.h>
#include <oooii/oP4.h>
#include <oooii/oProcess.h>
#include <oooii/oRef.h>
#include <oooii/oStddef.h>
#include <oooii/oWindows.h>
#include <io.h>
#include <regex>
using namespace std::tr1;

double oTimer()
{
	LARGE_INTEGER ticks, freq;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&ticks);
	return ticks.QuadPart / static_cast<double>(freq.QuadPart);
}

bool oGetSysPath(char* _StrSysPath, size_t _SizeofStrSysPath, oSYSPATH _SysPath)
{
	bool success = true;
	DWORD nElements = static_cast<DWORD>(_SizeofStrSysPath);

	switch (_SysPath)
	{
		case oSYSPATH_APP: oGetExePath(_StrSysPath, _SizeofStrSysPath); *oGetFilebase(_StrSysPath) = 0; break;
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
			// fixme: find a better way to do this...
			success = oGetExePath(_StrSysPath, _SizeofStrSysPath);
			*oGetFilebase(_StrSysPath) = 0;
			success = 0 == strcat_s(_StrSysPath, _SizeofStrSysPath, "../../../"); // assumes $DEV/bin/$PLATFORM/$BUILDTYPE
			break;
		}

		case oSYSPATH_COMPILER_INCLUDES:
		{
			// @oooii-tony: Yes, sorta hard-coded but better than trying to get at this
			// directory elsewhere in user code.
			success = oGetEnvironmentVariable(_StrSysPath, _SizeofStrSysPath, "VS90COMNTOOLS");
			if (success)
			{
				oEnsureFileSeparator(_StrSysPath, _SizeofStrSysPath);
				strcat_s(_StrSysPath, _SizeofStrSysPath, "../../VC/include/");
				oCleanPath(_StrSysPath, _SizeofStrSysPath, _StrSysPath);
			}

			else
				oSetLastError(ENOENT, "Failed to find compiler include path becayse env var VS90COMNTOOLS does not exist");

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

		default: oASSUME(0);
	}

	if (success)
	{
		oEnsureFileSeparator(_StrSysPath, _SizeofStrSysPath);
		success = 0 == oCleanPath(_StrSysPath, _SizeofStrSysPath, _StrSysPath);
	}
	return success;
}

bool oGetHostname(char* _Hostname, size_t _SizeofHostname)
{
	DWORD dwSize = static_cast<DWORD>(_SizeofHostname);
	if (dwSize != _SizeofHostname)
	{
		oSetLastError(EINVAL, "Specified size is too large for underlying platform");
		return false;
	}

	if (!GetComputerNameEx(ComputerNameDnsHostname, _Hostname, &dwSize))
	{
		oWinSetLastError();
		return false;
	}

	return true;
}

bool oGetExePath(char* _ExePath, size_t _SizeofExePath)
{
	DWORD length = GetModuleFileNameA(GetModuleHandle(0), _ExePath, static_cast<DWORD>(_SizeofExePath));
	if (length == (DWORD)_SizeofExePath && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		oSetLastError(STRUNCATE);
		return false;
	}

	return true;
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

bool oExecute(const char* _CommandLine, char* _StrStdout, size_t _SizeofStrStdOut, int* _pExitCode, unsigned int _ExecutionTimeout)
{
	oProcess::DESC desc;
	desc.CommandLine = _CommandLine;
	desc.EnvironmentString = 0;
	desc.StdHandleBufferSize = 4096;
	oRef<threadsafe oProcess> process;
	if (!oProcess::Create(&desc, &process))
		return false;

	oTRACE("oExecute: \"%s\"...", oSAFESTRN(_CommandLine));
	process->Start();
	if (!process->Wait(_ExecutionTimeout))
	{
		oSetLastError(ETIMEDOUT, "Executing \"%s\" timed out after %.01f seconds.", _CommandLine, static_cast<float>(_ExecutionTimeout) / 1000.0f);
		if (_pExitCode)
			*_pExitCode = EINPROGRESS;
		return false;
	}

	if (_pExitCode && !process->GetExitCode(_pExitCode))
		*_pExitCode = EINPROGRESS;

	if (_StrStdout && _SizeofStrStdOut)
	{
		oTRACE("oExecute: Reading from stdout... \"%s\"", oSAFESTRN(_CommandLine));
		size_t sizeRead = process->ReadFromStdout(_StrStdout, _SizeofStrStdOut);
		oASSERT(sizeRead < _SizeofStrStdOut, "");
		_StrStdout[sizeRead] = 0;
	}

	return true;
}

int oCompareDateTime(const oDateTime& _DateTime1, const oDateTime& _DateTime2)
{
	time_t time1 = oConvertDateTime(_DateTime1);
	time_t time2 = oConvertDateTime(_DateTime2);
	if (time1 == time2)
		return _DateTime1.Milliseconds > _DateTime2.Milliseconds ? 1 : -1;
	else return time1 > time2 ? 1 : -1;
}

bool oGetDateTime(oDateTime* _pDateTime)
{
	if (!_pDateTime)
	{
		oSetLastError(EINVAL, "A valid address to receive an oDateTime must be specified.");
		return false;
	}
	
	SYSTEMTIME st;
	GetLocalTime(&st);

	oSTATICASSERT(sizeof(SYSTEMTIME) == sizeof(oDateTime));
	memcpy(_pDateTime, &st, sizeof(st));
	return true;
}

time_t oConvertDateTime(const oDateTime& _DateTime)
{
	return oSystemTimeToUnixTime((const SYSTEMTIME*)&_DateTime);
}

void oConvertDateTime(oDateTime* _DateTime, time_t _Time)
{
	SYSTEMTIME st;
	oUnixTimeToSystemTime(_Time, &st);
	oV(SystemTimeToTzSpecificLocalTime(0, &st, (SYSTEMTIME*)_DateTime));
}

static const char* TIME_STRING_FORMAT = "%04hu/%02hu/%02hu %02hu:%02hu:%02hu:%03hu";

errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const oDateTime& _Value)
{
	return -1 == sprintf_s(_StrDestination, _SizeofStrDestination, TIME_STRING_FORMAT, _Value.Year, _Value.Month, _Value.Day, _Value.Hour, _Value.Minute, _Value.Second, _Value.Milliseconds) ? EINVAL : 0;
}

errno_t oFromString(oDateTime* _pValue, const char* _StrSource)
{
	return 7 == sscanf_s(_StrSource, TIME_STRING_FORMAT, &_pValue->Year, &_pValue->Month, &_pValue->Day, &_pValue->Hour, &_pValue->Minute, &_pValue->Second, &_pValue->Milliseconds) ? 0 : EINVAL;
}

bool oSetEnvironmentVariable(const char* _Name, const char* _Value)
{
	return !!SetEnvironmentVariableA(_Name, _Value);
}

bool oGetEnvironmentVariable(char* _Value, size_t _SizeofValue, const char* _Name)
{
	oASSERT((size_t)(int)_SizeofValue == _SizeofValue, "Invalid size");
	size_t len = GetEnvironmentVariableA(_Name, _Value, (int)_SizeofValue);
	return len && len < _SizeofValue;
}

bool oGetEnvironmentString(char* _StrEnvironment, size_t _SizeofStrEnvironment)
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
		oSetLastError(err, "strcpy to user-specified buffer failed");
		return false;
	}

	return true;
}

bool oSysGUIUsesGPUCompositing()
{
	return oIsAeroEnabled();
}

// Schedule a wakeup time before the computer sleeps using waitable timers
bool oScheduleWakeupAbsolute(time_t _AbsoluteTime, oFUNCTION<void()> _OnWake)
{
	return oScheduleFunction("OOOii.Wakeup", _AbsoluteTime, true, _OnWake);
}

bool oScheduleWakeupRelative(unsigned int _TimeFromNowInMilliseconds, oFUNCTION<void()> _OnWake)
{
	time_t now = time(nullptr);
	return oScheduleWakeupAbsolute(now + (_TimeFromNowInMilliseconds / 1000), _OnWake);
}

void oSysAllowSleep(bool _Allow)
{
	switch (oGetWindowsVersion())
	{
		case oWINDOWS_2000:
		case oWINDOWS_XP:
		case oWINDOWS_XP_PRO_64BIT:
		case oWINDOWS_SERVER_2003:
		case oWINDOWS_SERVER_2003R2:
			oASSERT(false, "oAllowSystemSleep won't work on %s.", oAsString(oGetWindowsVersion()));
		default:
			break;
	}

	EXECUTION_STATE next = _Allow ? ES_CONTINUOUS : (ES_CONTINUOUS|ES_SYSTEM_REQUIRED|ES_AWAYMODE_REQUIRED);
	EXECUTION_STATE prior = SetThreadExecutionState(next);
	oASSERT(prior != 0, "SetThreadExecutionState failed.");
}
