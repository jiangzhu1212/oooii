/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
#include <oStd/oStdChrono.h>
#include <oStd/scc.h>
#include <oPlatform/oProcess.h>
#include <oPlatform/oProgressBar.h>
#include <oPlatform/Windows/oWindows.h>
#include "SoftLink/oWinPowrProf.h"

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

static void oSystemGetDate(FILETIME* _pFileTime, bool _IsUTC)
{
	if (_IsUTC)
		GetSystemTimeAsFileTime(_pFileTime);
	else
	{
		SYSTEMTIME st;
		GetLocalTime(&st);
		SystemTimeToFileTime(&st, _pFileTime);
	}
}

void oSystemGetDate(oStd::ntp_timestamp* _pNTPTimestamp)
{
	FILETIME ft;
	oSystemGetDate(&ft, true);
	*_pNTPTimestamp = oStd::date_cast<oStd::ntp_timestamp>(ft);
}

void oSystemGetDate(oStd::ntp_date* _pNTPDate)
{
	FILETIME ft;
	oSystemGetDate(&ft, true);
	*_pNTPDate = oStd::date_cast<oStd::ntp_date>(ft);
}

void oSystemGetDate(oStd::date* _pDate)
{
	FILETIME ft;
	oSystemGetDate(&ft, true);
	*_pDate = oStd::date_cast<oStd::date>(ft);
}

static void oToDate(const SYSTEMTIME& _SystemTime, oStd::date* _pDate)
{
	_pDate->year = _SystemTime.wYear;
	_pDate->month = (oStd::month::value)_SystemTime.wMonth;
	_pDate->day = _SystemTime.wDay;
	_pDate->day_of_week = (oStd::weekday::value)_SystemTime.wDayOfWeek;
	_pDate->hour = _SystemTime.wHour;
	_pDate->minute = _SystemTime.wMinute;
	_pDate->second = _SystemTime.wSecond;
	_pDate->millisecond = _SystemTime.wMilliseconds;
}

static void oFromDate(const oStd::date& _Date, SYSTEMTIME* _pSystemTime)
{
	_pSystemTime->wYear = (WORD)_Date.year;
	_pSystemTime->wMonth = (WORD)_Date.month;
	_pSystemTime->wDay = (WORD)_Date.day;
	_pSystemTime->wHour = (WORD)_Date.hour;
	_pSystemTime->wMinute = (WORD)_Date.minute;
	_pSystemTime->wSecond = (WORD)_Date.second;
	_pSystemTime->wMilliseconds = (WORD)_Date.millisecond;
}

bool oSystemDateFromLocal(const oStd::date& _LocalDate, oStd::date* _pUTCDate)
{
	SYSTEMTIME In, Out;
	oFromDate(_LocalDate, &In);
	if (!SystemTimeToTzSpecificLocalTime(nullptr, &In, &Out))
		return oWinSetLastError();
	oToDate(Out, _pUTCDate);
	return true;
}

bool oSystemDateToLocal(const oStd::date& _UTCDate, oStd::date* _pLocalTime)
{
	SYSTEMTIME In, Out;
	oFromDate(_UTCDate, &In);
	if (!SystemTimeToTzSpecificLocalTime(nullptr, &In, &Out))
		return oWinSetLastError();
	oToDate(Out, _pLocalTime);
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
			oASSERT(false, "oSystemAllowSleep won't work on %s.", oStd::as_string(oGetWindowsVersion()));
		default:
			break;
	}

	EXECUTION_STATE next = _Allow ? ES_CONTINUOUS : (ES_CONTINUOUS|ES_SYSTEM_REQUIRED|ES_AWAYMODE_REQUIRED);
	if (!SetThreadExecutionState(next))
		return oWinSetLastError();
	return true;
}

bool oSystemScheduleWakeup(time_t _UnixAbsoluteTime, oTASK _OnWake)
{
	return oScheduleTask("OOOii.Wakeup", _UnixAbsoluteTime, true, _OnWake);
}

// This moves any leftovers to the front of the buffer and returns the offset
// where a new write should start.
static size_t VisitLines(char* _Buffer, size_t _ReadSize, const oFUNCTION<void(char* _Line)>& _GetLine)
{
	_Buffer[_ReadSize] = '\0';
	char* line = _Buffer;
	size_t eol = strcspn(line, oNEWLINE);

	while (line[eol] != '\0')
	{
		line[eol] = '\0';
		_GetLine(line);
		line += eol + 1;
		line += strspn(line, oNEWLINE);
		eol = strcspn(line, oNEWLINE);
	}

	return strlcpy(_Buffer, line, _ReadSize);
}

//#define DEBUG_EXECUTED_PROCESS
bool oSystemExecute(const char* _CommandLine, const oFUNCTION<void(char* _Line)>& _GetLine, int* _pExitCode, bool _ShowWindow, unsigned int _ExecutionTimeout)
{
	oStd::xlstring tempStdOut;
	oProcess::DESC desc;
	desc.CommandLine = _CommandLine;
	desc.EnvironmentString = 0;
	desc.StdHandleBufferSize = tempStdOut.capacity() - 1;
	desc.Show = _ShowWindow ? oPROCESS_SHOW : oPROCESS_HIDE;
	#ifdef DEBUG_EXECUTED_PROCESS
		desc.StartSuspended = true;
	#endif
	oRef<threadsafe oProcess> process;
	if (!oProcessCreate(desc, &process))
		return false;

	oTRACE("oExecute: \"%s\"...", oSAFESTRN(_CommandLine));
	#ifdef DEBUG_EXECUTED_PROCESS
		process->Start();
	#endif
	bool Finished = true;
	float startTime = oTimerMSF();
	uint timeSoFarMS = 0;
	static const uint timePerFlushMS = 50;
	
	// need to flush stdout once in a while or it can hang the process if we are 
	// redirecting output
	bool once = false;

	do
	{
		if (!once)
		{
			oTRACEA("oExecute: Reading from stdout... \"%s\"", oSAFESTRN(_CommandLine));
			once = true;
		}
		
		size_t ReadSize = process->ReadFromStdout((void*)tempStdOut.c_str(), tempStdOut.capacity());
		while (ReadSize && _GetLine)
		{
			size_t offset = VisitLines(tempStdOut, ReadSize, _GetLine);
			ReadSize = process->ReadFromStdout((void*)oStd::byte_add(tempStdOut.c_str(), offset), tempStdOut.capacity() - offset);
		}

		timeSoFarMS = static_cast<int>(oTimerMSF() - startTime);
	} while(timeSoFarMS < _ExecutionTimeout && !process->Wait(timePerFlushMS));

	// get any remaining text from stdout
	size_t offset = 0;
	size_t ReadSize = process->ReadFromStdout((void*)tempStdOut.c_str(), tempStdOut.capacity());
	while (ReadSize && _GetLine)
	{
		offset = VisitLines(tempStdOut, ReadSize, _GetLine);
		ReadSize = process->ReadFromStdout((void*)oStd::byte_add(tempStdOut.c_str(), offset), tempStdOut.capacity() - offset);
	}

	if (offset && _GetLine)
		_GetLine(tempStdOut);

	if (timeSoFarMS >= _ExecutionTimeout) // timed out
	{
		Finished = false;
		oErrorSetLast(std::errc::timed_out, "Executing \"%s\" timed out after %.01f seconds.", _CommandLine, static_cast<float>(_ExecutionTimeout) / 1000.0f);
		if (_pExitCode)
			*_pExitCode = std::errc::operation_in_progress;
	}

	if (_pExitCode && !process->GetExitCode(_pExitCode))
		*_pExitCode = std::errc::operation_in_progress;
	
	return Finished;
}

bool oSystemWaitIdle(unsigned int _TimeoutMS, oFUNCTION<bool()> _ContinueIdling)
{
	oRef<oProgressBar> PB;
	if(!_ContinueIdling)
	{
		if (!oProgressBarCreate([=] {}, "Waiting for System Idle...", &PB))
			return false; // pass through error
		PB->SetText("Waiting for system steady state...");
		PB->SetSubtext("Press stop to stop waiting and continue.");
		_ContinueIdling = [&]()->bool { return PB->GetStopped(); };
	}
	
	bool IsSteady = false;
	oStd::chrono::high_resolution_clock::time_point TimeStart = oStd::chrono::high_resolution_clock::now();
	oStd::chrono::high_resolution_clock::time_point TimeCurrent = TimeStart;
	unsigned long long PreviousIdleTime = 0, PreviousSystemTime = 0;
	static const double kLowCPUUsage = 5.0;
	static const unsigned int kNumSamplesAtLowCPUUsageToBeSteady = 10;
	unsigned int nSamplesAtLowCPUUsage = 0;
	
	while (1)
	{
		if (_TimeoutMS != oInfiniteWait && (oSeconds(TimeCurrent - TimeStart) >= oStd::chrono::milliseconds(_TimeoutMS)))
			return oErrorSetLast(std::errc::timed_out);
		
		if (oWinSystemAllServicesInSteadyState())
		{
			double CPUUsage = oWinSystemCalculateCPUUsage(&PreviousIdleTime, &PreviousSystemTime);
			if (CPUUsage < kLowCPUUsage)
				nSamplesAtLowCPUUsage++;
			else
				nSamplesAtLowCPUUsage = 0;

			if (nSamplesAtLowCPUUsage > kNumSamplesAtLowCPUUsageToBeSteady)
				return true;

			if(!_ContinueIdling() )
				return oErrorSetLast(std::errc::operation_canceled, "User canceled the wait"); 

			Sleep(200);
		}

		TimeCurrent = oStd::chrono::high_resolution_clock::now();
	}

	return false;
}

bool oSystemGUIUsesGPUCompositing()
{
	return oIsAeroEnabled();
}

bool oSystemGUIEnableGPUCompositing(bool _Enable, bool _Force)
{
	return oEnableAero(_Enable, _Force);
}

bool oSystemIsRemote()
{
	return !!GetSystemMetrics(SM_REMOTESESSION);
}

bool oSystemGUIIsWritable()
{
	return !!GetForegroundWindow();
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

static std::regex EncodedSearch("(%(.+?)%)");

char* oSystemTranslateEnvironmentVariables(char* _StrDestination, size_t _SizeofStrDestination, const char* _RawString)
{
	oStd::path_string Current = _RawString;

	const std::cregex_token_iterator end;
	int arr[] = {1,2}; 
	bool NoTranslations = true;
	for ( std::cregex_token_iterator VecTok(_RawString, _RawString + oStrlen(_RawString), EncodedSearch, arr); VecTok != end; ++VecTok )
	{
		auto Replace = VecTok->str();
		++VecTok;
		auto EnvVariable = VecTok->str();
		oStd::path_string TranslatedVariable;
		oSystemGetEnvironmentVariable(TranslatedVariable, EnvVariable.c_str());
		oStd::replace(_StrDestination, _SizeofStrDestination, Current, Replace.c_str(), TranslatedVariable.c_str());
		Current = _StrDestination;
		NoTranslations = false;
	}
	if( NoTranslations )
		oStrcpy(_StrDestination, _SizeofStrDestination, Current);

	return _StrDestination;
}

char* oSystemGetEnvironmentString(char* _StrEnvironment, size_t _SizeofStrEnvironment)
{
	char* pEnv = GetEnvironmentStringsA();
	oStd::finally OSCFreeEnv([&] { if (pEnv) { FreeEnvironmentStringsA(pEnv); } });

	// @oooii-tony: replace nuls with newlines to make parsing this mega-string
	// a bit less obtuse

	char* c = pEnv;
	size_t len = oStrlen(pEnv);
	while (len)
	{
		c[len] = '\n';
		c += len+1;
		len = oStrlen(c);
	}

	if (!oStrcpy(_StrEnvironment, _SizeofStrEnvironment, pEnv))
	{
		oErrorSetLast(std::errc::no_buffer_space, "strcpy to user-specified buffer failed");
		return nullptr;
	}

	return _StrEnvironment;
}

namespace oStd {

	const char* as_string(oSYSPATH _SysPath)
	{
		switch (_SysPath)
		{
			case oSYSPATH_CWD: return "oSYSPATH_CWD";
			case oSYSPATH_APP: return "oSYSPATH_APP";
			case oSYSPATH_APP_FULL: return "oSYSPATH_APP_FULL";
			case oSYSPATH_SYSTMP: return "oSYSPATH_SYSTMP";
			case oSYSPATH_SYS: return "oSYSPATH_SYS";
			case oSYSPATH_OS: return "oSYSPATH_OS";
			case oSYSPATH_DEV: return "oSYSPATH_DEV";
			case oSYSPATH_TESTTMP: return "oSYSPATH_TESTTMP";
			case oSYSPATH_COMPILER_INCLUDES: return "oSYSPATH_COMPILER_INCLUDES";
			case oSYSPATH_DESKTOP: return "oSYSPATH_DESKTOP";
			case oSYSPATH_DESKTOP_ALLUSERS: return "oSYSPATH_DESKTOP_ALLUSERS";
			case oSYSPATH_DATA: return "oSYSPATH_DATA";
			oNODEFAULT;
		}
	}

bool from_string(oSYSPATH* _pValue, const char* _StrSource)
{
	static const char* sStrings[] =
	{
		"oSYSPATH_CWD",
		"oSYSPATH_APP",
		"oSYSPATH_APP_FULL",
		"oSYSPATH_SYSTMP",
		"oSYSPATH_SYS",
		"oSYSPATH_OS",
		"oSYSPATH_DEV",
		"oSYSPATH_TESTTMP",
		"oSYSPATH_COMPILER_INCLUDES",
		"oSYSPATH_DESKTOP",
		"oSYSPATH_DESKTOP_ALLUSERS",
		"oSYSPATH_P4ROOT",
		"oSYSPATH_DATA",
	};

	oStd::sstring SourceUppercase = _StrSource;
	oToUpper(SourceUppercase);
	oFORI(i, sStrings)
	{
		if (!oStrcmp(_StrSource, sStrings[i]) || !oStrcmp(SourceUppercase, sStrings[i]+9)) // +9 match against just "OS" or "HOST" after oSYSPATH_
		{
			*_pValue = (oSYSPATH)i;
			return true;
		}
	}
	return false;
}

} // namespace oStd

char* oSystemGetURI(char* _StrSysURI, size_t _SizeofStrSysURI, oSYSPATH _SysPath)
{
	oStd::path_string Path;
	if (!oSystemGetPath(Path, _SysPath))
		return false; // pass through error
	return oURIFromAbsolutePath(_StrSysURI, _SizeofStrSysURI, Path); // pass through error
}

char* oSystemGetPath(char* _StrSysPath, size_t _SizeofStrSysPath, oSYSPATH _SysPath)
{
	bool ensureSeparator = true;
	bool success = true;
	DWORD nElements = oUInt(_SizeofStrSysPath);

	switch (_SysPath)
	{
		case oSYSPATH_APP_FULL:
		{
			ensureSeparator = false;
			DWORD len = GetModuleFileNameA(GetModuleHandle(nullptr), _StrSysPath, nElements);
			if (len == nElements && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				oErrorSetLast(std::errc::no_buffer_space);
				success = false;
			}
			break;
		}

		case oSYSPATH_APP:
			if (!oSystemGetPath(_StrSysPath, _SizeofStrSysPath, oSYSPATH_APP_FULL))
				return false;
			*oGetFilebase(_StrSysPath) = 0; 
			break;

		case oSYSPATH_CWD: GetCurrentDirectoryA(nElements, _StrSysPath); break;
		case oSYSPATH_SYS: GetSystemDirectoryA(_StrSysPath, nElements); break;
		case oSYSPATH_OS: GetWindowsDirectoryA(_StrSysPath, nElements); break;
		case oSYSPATH_SCCROOT:
		{
			oStd::path_string app;
			oSystemGetPath(app, oSYSPATH_APP);

			auto scc = oStd::make_scc(oStd::scc_protocol::svn, oBIND(oSystemExecute, oBIND1, oBIND2, oBIND3, false, oBIND4));
			try { scc->root(app, _StrSysPath, _SizeofStrSysPath); }
			catch (std::exception& e) { oErrorSetLast(e); return nullptr; }

			break;
		}

		case oSYSPATH_DEV:
		{
			if (!oSystemGetPath(_StrSysPath, _SizeofStrSysPath, oSYSPATH_APP_FULL))
				return false;
			char* cur = oGetFilebase(_StrSysPath);
			*cur = 0;

			while (_StrSysPath <= (cur-4)) // 4 is oStrlen("bin/")
			{
				if (!_memicmp("bin", cur-4, 3))
				{
					*(cur-1) = 0;
					cur = oGetFilebase(_StrSysPath);
					*cur = 0;
					success = true;
					break;
				}

				*(cur-1) = 0;
				cur = oGetFilebase(_StrSysPath);
				*cur = 0;
			}

			break;
		}

		case oSYSPATH_COMPILER_INCLUDES:
		{
			static const char* kCmnToolsEnvVar = "VS100COMNTOOLS";

			// @oooii-tony: Yes, sorta hard-coded but better than trying to get at this
			// directory elsewhere in user code.
			success = !!oSystemGetEnvironmentVariable(_StrSysPath, _SizeofStrSysPath, kCmnToolsEnvVar);
			if (success)
			{
				oEnsureSeparator(_StrSysPath, _SizeofStrSysPath);
				oStrcat(_StrSysPath, _SizeofStrSysPath, "../../VC/include/");
				oCleanPath(_StrSysPath, _SizeofStrSysPath, _StrSysPath);
			}

			else
				oErrorSetLast(std::errc::no_such_file_or_directory, "Failed to find compiler include path because env var %s does not exist", kCmnToolsEnvVar);

			break;
		}

		case oSYSPATH_SYSTMP:
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
			// Either data is "./Data" or we're in the SDK dir and the path is 
			// relative to that

			char path[_MAX_PATH];
			if (oSystemGetPath(path, oSYSPATH_APP))
			{
				if (-1 != oPrintf(_StrSysPath, _SizeofStrSysPath, "%sData/", path))
				{
					if (INVALID_FILE_ATTRIBUTES == GetFileAttributesA(_StrSysPath))
					{
						if (oSystemGetPath(_StrSysPath, _SizeofStrSysPath, oSYSPATH_DEV))
						{
							if (!oStrcat(_StrSysPath, _SizeofStrSysPath, "Data/"))
								success = false;
						}
						else
							success = false;
					}
				}

				else
				{
					oErrorSetLast(std::errc::no_buffer_space);
					success = false;
				}
			}

			else
				success = false;

			break;
		}

		case oSYSPATH_TESTTMP:
		{
			oStd::path_string dataPath;
			success = true;
			if (!oSystemGetPath(dataPath, oSYSPATH_DATA))
				success = false;

			if (-1 == oPrintf(_StrSysPath, _SizeofStrSysPath, "%sTemp/", dataPath.c_str()))
			{
				oErrorSetLast(std::errc::no_buffer_space);
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

char* oSystemGetHostname(char* _StrDestination, size_t _SizeofStrDestination)
{
	DWORD nElements = oUInt(_SizeofStrDestination);
	if (!GetComputerNameExA(ComputerNameDnsHostname, _StrDestination, &nElements))
	{
		oWinSetLastError();
		return nullptr;
	}
	return _StrDestination;
}

char* oSystemGetExecutionPath(char* _StrDestination, size_t _SizeofStrDestination)
{
	oStd::sstring hostname;
	if (!oSystemGetHostname(hostname))
		return false; // pass through error
	else if (oInvalid == oPrintf(_StrDestination, _SizeofStrDestination, "[%s.%u.%u]", hostname.c_str(), oProcessGetCurrentID(), oStd::this_thread::get_id()))
	{
		oErrorSetLast(std::errc::no_buffer_space);
		return nullptr;
	}
	return _StrDestination;
}

char* oSystemURIToPath(char* _Path, size_t _SizeofPath, const char* _URI)
{
	oURIParts parts;
	if (!oURIDecompose(_URI, &parts))
		return nullptr;

	return oSystemURIPartsToPath(_Path, _SizeofPath, parts);
}

char* oSystemURIPartsToPath(char* _Path, size_t _SizeofPath, const oURIParts& _URIParts)
{
	if (_URIParts.Authority.empty())
	{
		if (_URIParts.Path[0]=='/' && _URIParts.Path[2]==':')
			oStrcpy(_Path, _SizeofPath, _URIParts.Path+1);
		else
			oStrcpy(_Path, _SizeofPath, _URIParts.Path);
	}
	else
	{
		oSYSPATH SysPath;
		if (oStd::from_string(&SysPath, _URIParts.Authority))
		{
			if (!oSystemGetPath(_Path, _SizeofPath, SysPath))
			{
				oErrorSetLast(std::errc::protocol_error, "Failed to find %s", oStd::as_string(SysPath));
				return nullptr;
			}

			if (!oEnsureSeparator(_Path, _SizeofPath) && oErrorGetLast() != std::errc::operation_in_progress)
			{
				oErrorSetLast(std::errc::no_buffer_space);
				return nullptr;
			}

			if (-1 == oStrAppendf(_Path, _SizeofPath, _URIParts.Path))
			{
				oErrorSetLast(std::errc::no_buffer_space);
				return nullptr;
			}
		}

		else if (!oURIPartsToPath(_Path, _SizeofPath, _URIParts))
		{
			oErrorSetLast(std::errc::invalid_argument);
			return nullptr;
		}
	}

	return _Path;
}

char* oSystemFindInPath(char* _ResultingFullPath, size_t _SizeofResultingFullPath, oSYSPATH _SysPath, const char* _RelativePath, const char* _DotPath, const oFUNCTION<bool(const char* _Path)>& _PathExists)
{
	if (oSystemGetPath(_ResultingFullPath, _SizeofResultingFullPath, _SysPath))
	{
		size_t len = oStrlen(_ResultingFullPath);
		if (!oStrcpy(_ResultingFullPath + len, _SizeofResultingFullPath - len, _RelativePath))
		{
			oErrorSetLast(std::errc::no_buffer_space);
			return nullptr;
		}

		else if (_PathExists(_ResultingFullPath))
			return _ResultingFullPath;
	}

	return nullptr;
}

char* oSystemFindPath(char* _ResultingFullPath, size_t _SizeofResultingFullPath, const char* _RelativePath, const char* _DotPath, const char* _ExtraSearchPath, const oFUNCTION<bool(const char* _Path)>& _PathExists)
{
	if (oIsFullPath(_RelativePath) && _PathExists(_RelativePath) && !!oStrcpy(_ResultingFullPath, _SizeofResultingFullPath, _RelativePath))
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

const char *oSystemAppendDebugSuffixToDLL(const char *_pModuleName, char *_pAppendedName, int _SizeOfAppendedName)
{
#ifdef _DEBUG
	const char* BUILDSUFFIX = "D";
#else
	const char* BUILDSUFFIX = "";
#endif

	oSystemGetPath(_pAppendedName, _SizeOfAppendedName, oSYSPATH_APP);
	oPrintf(_pAppendedName, _SizeOfAppendedName, "%s%s%s.dll", _pAppendedName, _pModuleName, BUILDSUFFIX);

	return _pAppendedName;
}

