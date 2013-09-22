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
#include <oCore/system.h>
#include <oCore/process.h>
#include <oStd/date.h>
#include "../oStd/win.h"

using namespace oStd;

namespace oCore {
	namespace system {

heap_info get_heap_info()
{
	MEMORYSTATUSEX ms;
	ms.dwLength = sizeof(MEMORYSTATUSEX);
	oVB(GlobalMemoryStatusEx(&ms));
	heap_info hi;
	hi.total_used = ms.dwMemoryLoad;
	hi.avail_physical = ms.ullAvailPhys;
	hi.total_physical = ms.ullTotalPhys;
	hi.avail_virtual_process = ms.ullAvailVirtual;
	hi.total_virtual_process = ms.ullTotalVirtual;
	hi.avail_paged = ms.ullAvailPageFile;
	hi.total_paged = ms.ullTotalPageFile;
	return std::move(hi);
}

static void now(FILETIME* _pFileTime, bool _IsUTC)
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

void now(ntp_timestamp* _pNTPTimestamp)
{
	FILETIME ft;
	now(&ft, true);
	*_pNTPTimestamp = date_cast<ntp_timestamp>(ft);
}

void now(ntp_date* _pNTPDate)
{
	FILETIME ft;
	now(&ft, true);
	*_pNTPDate = date_cast<ntp_date>(ft);
}

void now(date* _pDate)
{
	FILETIME ft;
	now(&ft, true);
	*_pDate = date_cast<date>(ft);
}

static date to_date(const SYSTEMTIME& _SystemTime)
{
	date d;
	d.year = _SystemTime.wYear;
	d.month = (month::value)_SystemTime.wMonth;
	d.day = _SystemTime.wDay;
	d.day_of_week = (weekday::value)_SystemTime.wDayOfWeek;
	d.hour = _SystemTime.wHour;
	d.minute = _SystemTime.wMinute;
	d.second = _SystemTime.wSecond;
	d.millisecond = _SystemTime.wMilliseconds;
	return std::move(d);
}

static void from_date(const date& _Date, SYSTEMTIME* _pSystemTime)
{
	_pSystemTime->wYear = (WORD)_Date.year;
	_pSystemTime->wMonth = (WORD)_Date.month;
	_pSystemTime->wDay = (WORD)_Date.day;
	_pSystemTime->wHour = (WORD)_Date.hour;
	_pSystemTime->wMinute = (WORD)_Date.minute;
	_pSystemTime->wSecond = (WORD)_Date.second;
	_pSystemTime->wMilliseconds = (WORD)_Date.millisecond;
}

date from_local(const date& _LocalDate)
{
	SYSTEMTIME In, Out;
	from_date(_LocalDate, &In);
	oVB(!SystemTimeToTzSpecificLocalTime(nullptr, &In, &Out));
	return std::move(to_date(Out));
}

date to_local(const date& _UTCDate)
{
	SYSTEMTIME In, Out;
	from_date(_UTCDate, &In);
	oVB(!SystemTimeToTzSpecificLocalTime(nullptr, &In, &Out));
	return std::move(to_date(Out));
}

static void windows_set_privileges(const char* _privilege)
{
	HANDLE hT;
	TOKEN_PRIVILEGES tkp;
	oVB(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hT));
	windows::scoped_handle hToken(hT);
	oVB(!LookupPrivilegeValue(nullptr, _privilege, &tkp.Privileges[0].Luid));
	tkp.PrivilegeCount = 1;      
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
	oVB(!AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, nullptr, 0));
	// AdjustTokenPrivileges only fails for invalid parameters. If it fails 
	// because the app did not have permissions, then it succeeds (result will be 
	// true), but sets last error to ERROR_NOT_ALL_ASSIGNED.
	if (GetLastError() != ERROR_SUCCESS)
		throw windows::error();
}

void reboot()
{
	windows_set_privileges(SE_SHUTDOWN_NAME);
	oVB(ExitWindowsEx(EWX_REBOOT, SHTDN_REASON_FLAG_PLANNED));
}

void shutdown()
{
	windows_set_privileges(SE_SHUTDOWN_NAME);
	oVB(ExitWindowsEx(EWX_POWEROFF, SHTDN_REASON_FLAG_PLANNED));
}

void sleep()
{
	oVB(SetSuspendState(FALSE, TRUE, FALSE));
}

void allow_sleep(bool _Allow)
{
	switch (windows::get_version())
	{
		case windows::version::win2000:
		case windows::version::xp:
		case windows::version::xp_pro_64bit:
		case windows::version::server_2003:
		case windows::version::server_2003r2:
			oTHROW(operation_not_supported, "allow_sleep not supported on %s", as_string(windows::get_version()));
		default:
			break;
	}

	EXECUTION_STATE next = _Allow ? ES_CONTINUOUS : (ES_CONTINUOUS|ES_SYSTEM_REQUIRED|ES_AWAYMODE_REQUIRED);
	oVB(!SetThreadExecutionState(next));
}

struct SCHEDULED_FUNCTION_CONTEXT
{
	HANDLE hTimer;
	std::function<void()> OnTimer;
	time_t ScheduledTime;
	sstring DebugName;
};

static void CALLBACK ExecuteScheduledFunctionAndCleanup(LPVOID lpArgToCompletionRoutine, DWORD dwTimerLowValue, DWORD dwTimerHighValue)
{
	SCHEDULED_FUNCTION_CONTEXT& Context = *(SCHEDULED_FUNCTION_CONTEXT*)lpArgToCompletionRoutine;
	if (Context.OnTimer)
	{
		#ifdef _DEBUG
			sstring diff;
			format_duration(diff, (double)(time(nullptr) - Context.ScheduledTime));
			oTRACE("Running scheduled function '%s' %s after it was scheduled", oSAFESTRN(Context.DebugName), diff.c_str());
		#endif

		Context.OnTimer();
		oTRACE("Finished scheduled function '%s'", oSAFESTRN(Context.DebugName));
	}
	oVB(CloseHandle(Context.hTimer));
	delete &Context;
}

static void schedule_task(const char* _DebugName
	, time_t _AbsoluteTime
	, bool _Alertable
	, const std::function<void()>& _Task)
{
	SCHEDULED_FUNCTION_CONTEXT& Context = *new SCHEDULED_FUNCTION_CONTEXT();
	Context.hTimer = CreateWaitableTimer(nullptr, TRUE, nullptr);
	if (!Context.hTimer)
		throw windows::error();
	Context.OnTimer = _Task;
	Context.ScheduledTime = _AbsoluteTime;

	if (oSTRVALID(_DebugName))
		strlcpy(Context.DebugName, _DebugName);
	
	#ifdef _DEBUG
		date then;
		sstring StrTime, StrDiff;
		try { then = date_cast<date>(_AbsoluteTime); }
		catch (std::exception&) { StrTime = "(out-of-time_t-range)"; }
		strftime(StrTime, sortable_date_format, then);
		format_duration(StrDiff, (double)(time(nullptr) - Context.ScheduledTime));
		oTRACE("Setting timer to run function '%s' at %s (%s from now)", oSAFESTRN(Context.DebugName), StrTime.c_str(), StrDiff.c_str());
	#endif

	FILETIME ft = date_cast<FILETIME>(_AbsoluteTime);
	LARGE_INTEGER liDueTime;
	liDueTime.LowPart = ft.dwLowDateTime;
	liDueTime.HighPart = ft.dwHighDateTime;
	oVB(!SetWaitableTimer(Context.hTimer, &liDueTime, 0, ExecuteScheduledFunctionAndCleanup, (LPVOID)&Context, _Alertable ? TRUE : FALSE));
}

void schedule_wakeup(time_t _Time, const std::function<void()>& _OnWake)
{
	schedule_task("Ouroboros.Wakeup", _Time, true, _OnWake);
}

void wait_idle(const std::function<bool()>& _ContinueWaiting)
{
	wait_for_idle(INFINITE, _ContinueWaiting);
}

void windows_enumerate_services(const std::function<bool(SC_HANDLE _hSCManager, const ENUM_SERVICE_STATUS_PROCESS& _Status)>& _Enumerator)
{
	SC_HANDLE hSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE);
	if (!hSCManager)
		throw windows::error();
	finally closeSCManager([&] { oVB(CloseServiceHandle(hSCManager)); });

	DWORD requiredBytes = 0;
	DWORD nServices = 0;
	EnumServicesStatusEx(hSCManager, SC_ENUM_PROCESS_INFO, SERVICE_TYPE_ALL, SERVICE_STATE_ALL, nullptr, 0, &requiredBytes, &nServices, nullptr, nullptr);
	if (GetLastError() != ERROR_MORE_DATA)
		throw windows::error();

	ENUM_SERVICE_STATUS_PROCESS* lpServices = (ENUM_SERVICE_STATUS_PROCESS*)malloc(requiredBytes);
	finally freeServices([&] { free(lpServices); });
	
	oVB(EnumServicesStatusEx(hSCManager, SC_ENUM_PROCESS_INFO, SERVICE_TYPE_ALL, SERVICE_STATE_ALL, (LPBYTE)lpServices, requiredBytes, &requiredBytes, &nServices, nullptr, nullptr));
	for (DWORD i = 0; i < nServices; i++)
		if (!_Enumerator(hSCManager, lpServices[i]))
			break;
}

static bool windows_all_services_steady()
{
	bool NonSteadyStateFound = false;
	windows_enumerate_services([&](SC_HANDLE _hSCManager, const ENUM_SERVICE_STATUS_PROCESS& _Status)->bool
	{
		if (SERVICE_PAUSED != _Status.ServiceStatusProcess.dwCurrentState && SERVICE_RUNNING != _Status.ServiceStatusProcess.dwCurrentState && SERVICE_STOPPED != _Status.ServiceStatusProcess.dwCurrentState)
		{
			NonSteadyStateFound = true;
			return false;
		}
		return true;
	});
	return !NonSteadyStateFound;
}

static double windows_cpu_usage(unsigned long long* _pPreviousIdleTime, unsigned long long* _pPreviousSystemTime)
{
	double CPUUsage = 0.0;

	unsigned long long idleTime, kernelTime, userTime;
	oVB(GetSystemTimes((LPFILETIME)&idleTime, (LPFILETIME)&kernelTime, (LPFILETIME)&userTime));
	unsigned long long systemTime = kernelTime + userTime;
	
	if (*_pPreviousIdleTime && *_pPreviousSystemTime)
	{
		unsigned long long idleDelta = idleTime - *_pPreviousIdleTime;
		unsigned long long systemDelta = systemTime - *_pPreviousSystemTime;
		CPUUsage = (systemDelta - idleDelta) * 100.0 / (double)systemDelta;
	}		

	*_pPreviousIdleTime = idleTime;
	*_pPreviousSystemTime = systemTime;
	return CPUUsage; 
}

bool wait_for_idle(unsigned int _TimeoutMS, const std::function<bool()>& _ContinueWaiting)
{
	bool IsSteady = false;
	chrono::high_resolution_clock::time_point TimeStart = chrono::high_resolution_clock::now();
	chrono::high_resolution_clock::time_point TimeCurrent = TimeStart;
	unsigned long long PreviousIdleTime = 0, PreviousSystemTime = 0;
	static const double kLowCPUUsage = 5.0;
	static const unsigned int kNumSamplesAtLowCPUUsageToBeSteady = 10;
	unsigned int nSamplesAtLowCPUUsage = 0;
	
	while (true)
	{
		if (_TimeoutMS != INFINITE && (oSeconds(TimeCurrent - TimeStart) >= chrono::milliseconds(_TimeoutMS)))
			break;
		
		if (windows_all_services_steady())
		{
			double CPUUsage = windows_cpu_usage(&PreviousIdleTime, &PreviousSystemTime);
			if (CPUUsage < kLowCPUUsage)
				nSamplesAtLowCPUUsage++;
			else
				nSamplesAtLowCPUUsage = 0;

			if (nSamplesAtLowCPUUsage > kNumSamplesAtLowCPUUsageToBeSteady)
				return true;

			if (_ContinueWaiting && !_ContinueWaiting())
				break;

			this_thread::sleep_for(chrono::milliseconds(200));
		}

		TimeCurrent = chrono::high_resolution_clock::now();
	}

	return false;
}

bool uses_gpu_compositing()
{
	BOOL enabled = FALSE;
	oV(DwmIsCompositionEnabled(&enabled));
	return !!enabled;
}

void enable_gpu_compositing(bool _Enable, bool _Force)
{
	if (_Enable && _Force && !uses_gpu_compositing())
	{
		::system("%SystemRoot%\\system32\\rundll32.exe %SystemRoot%\\system32\\shell32.dll,Control_RunDLL %SystemRoot%\\system32\\desk.cpl desk,@Themes /Action:OpenTheme /file:\"C:\\Windows\\Resources\\Themes\\aero.theme\"");
		this_thread::sleep_for(oSeconds(31)); // Windows takes about 30 sec to settle after doing this.
	}
	else	
		oVB(DwmEnableComposition(_Enable ? DWM_EC_ENABLECOMPOSITION : DWM_EC_DISABLECOMPOSITION));
}

bool is_remote_session()
{
	return !!GetSystemMetrics(SM_REMOTESESSION);
}

bool gui_is_drawable()
{
	return !!GetForegroundWindow();
}

char* host_name(char* _StrDestination, size_t _SizeofStrDestination)
{
	oCHECK_SIZE(DWORD, _SizeofStrDestination);
	DWORD nElements = static_cast<DWORD>(_SizeofStrDestination);
	oVB(GetComputerNameExA(ComputerNameDnsHostname, _StrDestination, &nElements));
	return _StrDestination;
}

char* workgroup_name(char* _StrDestination, size_t _SizeofStrDestination)
{
	LPWKSTA_INFO_102 pInfo = nullptr;
	NET_API_STATUS nStatus;
	LPTSTR pszServerName = nullptr;

	nStatus = NetWkstaGetInfo(nullptr, 102, (LPBYTE *)&pInfo);
	oStd::finally OSCFreeBuffer([&] { if (pInfo) NetApiBufferFree(pInfo); });
	if (nStatus != NERR_Success)
		throw oStd::windows::error();
	
	WideCharToMultiByte(CP_ACP, 0, pInfo->wki102_langroup, -1, _StrDestination, static_cast<int>(_SizeofStrDestination), 0, 0);
	return _StrDestination;
}

char* exec_path(char* _StrDestination, size_t _SizeofStrDestination)
{
	sstring hostname;
	if (-1 == snprintf(_StrDestination, _SizeofStrDestination, "[%s.%u.%u]", hostname.c_str(), oCore::this_process::get_id(), oStd::this_thread::get_id()))
		oTHROW0(no_buffer_space);
	return _StrDestination;
}

void setenv(const char* _EnvVarName, const char* _Value)
{
	oVB(SetEnvironmentVariableA(_EnvVarName, _Value));
}

char* getenv(char* _Value, size_t _SizeofValue, const char* _EnvVarName)
{
	oCHECK_SIZE(int, _SizeofValue);
	size_t len = GetEnvironmentVariableA(_EnvVarName, _Value, (int)_SizeofValue);
	return (len && len < _SizeofValue) ? _Value : nullptr;
}

char* envstr(char* _StrEnvironment, size_t _SizeofStrEnvironment)
{
	char* pEnv = GetEnvironmentStringsA();
	finally OSCFreeEnv([&] { if (pEnv) { FreeEnvironmentStringsA(pEnv); } });

	// nuls -> newlines to make parsing this mega-string a bit less obtuse
	char* c = pEnv;
	char* d = _StrEnvironment;
	size_t len = strlen(pEnv);
	while (len)
	{
		c[len] = '\n';
		c += len+1;
		len = strlen(c);
	}

	if (strlcpy(_StrEnvironment, pEnv, _SizeofStrEnvironment) >= _SizeofStrEnvironment)
		oTHROW0(no_buffer_space);
	return _StrEnvironment;
}

// This moves any leftovers to the front of the buffer and returns the offset
// where a new write should start.
static size_t line_enumerator(char* _Buffer, size_t _ReadSize
	, const std::function<void(char* _Line)>& _GetLine)
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

// starting suspended can BSOD the machine, be careful
//#define DEBUG_EXECUTED_PROCESS
int spawn(const char* _CommandLine
	, const std::function<void(char* _Line)>& _GetLine
	, bool _ShowWindow
	, unsigned int _ExecutionTimeout)
{
	xlstring tempStdOut;
	std::shared_ptr<process> P;
	{
		process::info process_info;
		process_info.command_line = _CommandLine;
		process_info.environment = nullptr;
		process_info.initial_directory = nullptr;
		process_info.stdout_buffer_size = tempStdOut.capacity() - 1;
		process_info.show = _ShowWindow ? process::show : process::hide;
		#ifdef DEBUG_EXECUTED_PROCESS
			process_info.suspended = true;
		#endif
		P = std::move(process::make(process_info));
	}

	oTRACEA("spawn: >>> %s <<<", oSAFESTRN(_CommandLine));
	#ifdef DEBUG_EXECUTED_PROCESS
		process->start();
	#endif

	// need to flush stdout once in a while or it can hang the process if we are 
	// redirecting output
	bool once = false;

	static const unsigned int kTimeoutPerFlushMS = 50;
	double Timeout = _ExecutionTimeout == INFINITE ? DBL_MAX : (1000.0 * static_cast<double>(_ExecutionTimeout));
	double TimeSoFar = 0.0;
	timer t;
	do
	{
		if (!once)
		{
			oTRACEA("spawn stdout: >>> %s <<<", oSAFESTRN(_CommandLine));
			once = true;
		}
		
		size_t ReadSize = P->from_stdout((void*)tempStdOut.c_str(), tempStdOut.capacity() - 1);
		while (ReadSize && _GetLine)
		{
			size_t offset = line_enumerator(tempStdOut, ReadSize, _GetLine);
			ReadSize = P->from_stdout((void*)byte_add(tempStdOut.c_str(), offset), tempStdOut.capacity() - 1 - offset);
		}

		TimeSoFar = t.seconds();

	} while (TimeSoFar < Timeout && !P->wait_for(chrono::milliseconds(kTimeoutPerFlushMS)));
	
	// get any remaining text from stdout
	size_t offset = 0;
	size_t ReadSize = P->from_stdout((void*)tempStdOut.c_str(), tempStdOut.capacity() - 1);
	while (ReadSize && _GetLine)
	{
		offset = line_enumerator(tempStdOut, ReadSize, _GetLine);
		ReadSize = P->from_stdout((void*)byte_add(tempStdOut.c_str(), offset), tempStdOut.capacity() - 1 - offset);
	}

	if (offset && _GetLine)
		_GetLine(tempStdOut);

	if (TimeSoFar >= Timeout)
		return std::errc::timed_out;

	int ExitCode = 0;
	if (!P->exit_code(&ExitCode))
		ExitCode = std::errc::operation_in_progress;
	
	return ExitCode;
}

int spawn(const char* _CommandLine
	, const std::function<void(char* _Line)>& _GetLine
	, bool _ShowWindow)
{
	return spawn(_CommandLine, _GetLine, _ShowWindow, INFINITE);
}

	} // namespace system
} // namespace oCore
