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
#include <oCore/process.h>
#include <oCore/filesystem.h>
#include <oBase/date.h>
#include "../oStd/win.h"
#include <TlHelp32.h>
#include <Psapi.h>
#include <set>

#define oCLOSE(hHandle) do { if (hHandle && hHandle != INVALID_HANDLE_VALUE) ::CloseHandle(hHandle); } while(false)

using namespace oStd;
using namespace std::placeholders;

namespace ouro {

bool from_string(process::id* _pProcessID, const char* _String)
{
	return from_string((unsigned int*)_pProcessID, _String);
}

static HANDLE oCreateEvent(bool _AutoReset, const char* _Name)
{
	ouro::lstring n;
	snprintf(n, "Global\\%s", oSAFESTR(_Name));
	HANDLE e = CreateEventA(0, _AutoReset ? FALSE : TRUE, FALSE, n);
	if (!e) throw oStd::windows::error();
	return e;
}

process::event::event(const char* _Name)
	: e(oCreateEvent(false, _Name))
{}

process::event::event(process::autoreset_t _AutoReset, const char* _Name)
	: e(oCreateEvent(true, _Name))
{}

process::event::~event()
{
	CloseHandle(HANDLE(e));
}

void process::event::set()
{
	SetEvent(HANDLE(e));
}

void process::event::reset()
{
	ResetEvent(HANDLE(e));
}

void process::event::wait()
{
	wait_for_ms(INFINITE);
}

bool process::event::wait_for_ms(unsigned int _TimeoutMS)
{
	return WAIT_OBJECT_0 == ::WaitForSingleObject(HANDLE(e), _TimeoutMS);
}

class windows_process : public process
{
public:
	windows_process(const info& _Info)
		: Info(_Info)
		, CommandLine(oSAFESTR(_Info.command_line))
		, EnvironmentString(oSAFESTR(_Info.environment))
		, InitialWorkingDirectory(oSAFESTR(_Info.initial_directory))
		, hOutputRead(nullptr)
		, hOutputWrite(nullptr)
		, hInputRead(nullptr)
		, hInputWrite(nullptr)
		, hErrorWrite(nullptr)
		, Suspended(_Info.suspended)
	{
		memset(&ProcessInfo, 0, sizeof(PROCESS_INFORMATION));
		memset(&StartInfo, 0, sizeof(STARTUPINFO));
		StartInfo.cb = sizeof(STARTUPINFO);

		Info.command_line = CommandLine.c_str();
		Info.environment = EnvironmentString.c_str();
		Info.initial_directory = InitialWorkingDirectory.c_str();

		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = 0;
		sa.bInheritHandle = TRUE;

		DWORD dwCreationFlags = 0;
		if (Suspended)
			dwCreationFlags |= CREATE_SUSPENDED;

		switch (_Info.show)
		{
			case hide: dwCreationFlags |= CREATE_NO_WINDOW; StartInfo.wShowWindow = SW_HIDE; break;
			case show: StartInfo.wShowWindow = SW_SHOWNOACTIVATE; break;
			case focused: StartInfo.wShowWindow = SW_SHOWNORMAL; break;
			case minimized: StartInfo.wShowWindow = SW_SHOWMINNOACTIVE; break;
			case minimized_focused: StartInfo.wShowWindow = SW_SHOWMINIMIZED; break;
			oNODEFAULT;
		}

		if (_Info.stdout_buffer_size)
		{
			// Based on setup described here: http://support.microsoft.com/kb/190351

			HANDLE hOutputReadTmp = 0;
			HANDLE hInputWriteTmp = 0;
			oVB(CreatePipe(&hOutputReadTmp, &hOutputWrite, &sa, 0));
			oVB(DuplicateHandle(GetCurrentProcess(), hOutputWrite, GetCurrentProcess(), &hErrorWrite, 0, TRUE, DUPLICATE_SAME_ACCESS));

			if (!CreatePipe(&hInputRead, &hInputWriteTmp, &sa, 0))
			{
				HRESULT hr = GetLastError();
				oVB(CloseHandle(hOutputReadTmp));
				oVB(CloseHandle(hOutputWrite));
				throw windows::error(hr);
			}

			oVB(DuplicateHandle(GetCurrentProcess(), hOutputReadTmp, GetCurrentProcess(), &hOutputRead, 0, FALSE, DUPLICATE_SAME_ACCESS));
			oVB(DuplicateHandle(GetCurrentProcess(), hInputWriteTmp, GetCurrentProcess(), &hInputWrite, 0, FALSE, DUPLICATE_SAME_ACCESS));
			oVB(SetHandleInformation(hOutputRead, HANDLE_FLAG_INHERIT, 0));
			oVB(SetHandleInformation(hInputWrite, HANDLE_FLAG_INHERIT, 0));

			oVB(CloseHandle(hOutputReadTmp));
			oVB(CloseHandle(hInputWriteTmp));

			StartInfo.dwFlags |= STARTF_USESTDHANDLES;
			StartInfo.hStdOutput = hOutputWrite;
			StartInfo.hStdInput = hInputRead;
			StartInfo.hStdError = hErrorWrite;
		}

		else
			dwCreationFlags |= CREATE_NEW_CONSOLE;

		// Prepare to use specified environment
		char* env = nullptr;
		finally FreeEnv([&] { if (env) delete [] env; });
		if (!EnvironmentString.empty())
		{
			env = new char[EnvironmentString.length()+1];
			replace(env, EnvironmentString.length()+1, EnvironmentString.c_str(), '\n', '\0');
		}

		// Make a copy because CreateProcess does not take a const char*
		char* cmdline = nullptr;
		finally FreeCmd([&] { if (cmdline) delete [] cmdline; });
		if (!CommandLine.empty())
		{
			cmdline = new char[CommandLine.length()+1];
			strlcpy(cmdline, CommandLine.c_str(), CommandLine.length()+1);
		}

		oASSERT(cmdline, "a null cmdline can BSOD a machine");
		oVB(CreateProcessA(nullptr, cmdline, nullptr, &sa, TRUE, dwCreationFlags, env
			, InitialWorkingDirectory.empty() ? nullptr : InitialWorkingDirectory.c_str()
			, &StartInfo, &ProcessInfo));
	}

	~windows_process()
	{
		if (Suspended)
			kill(std::errc::timed_out);
		oCLOSE(ProcessInfo.hProcess);
		oCLOSE(ProcessInfo.hThread);
		oCLOSE(hOutputRead);
		oCLOSE(hOutputWrite);
		oCLOSE(hInputRead);
		oCLOSE(hInputWrite);
		oCLOSE(hErrorWrite);
	}

	info get_info() const override
	{
		return Info;
	}

	void start() override
	{
		if (Suspended)
		{
			oVB(ResumeThread(ProcessInfo.hThread));
			Suspended = false;
		}
	}

	void kill(int _ExitCode) override
	{
		id ID;
		*(unsigned int*)&ID = ProcessInfo.dwProcessId;
		terminate(ID, _ExitCode);
	}

	void wait() override
	{
		wait_for_ms(*(id*)&ProcessInfo.dwProcessId, INFINITE);
	}

	id get_id() const override
	{
		id ID;
		*(DWORD*)&ID = ProcessInfo.dwProcessId;
		return ID;
	}

	thread::id get_thread_id() const override
	{
		thread::id ID;
		*(DWORD*)&ID = ProcessInfo.dwThreadId;
		return ID;
	}

	bool exit_code(int* _pExitCode) const override
	{
		DWORD dwExitCode = 0;
		if (!GetExitCodeProcess(ProcessInfo.hProcess, &dwExitCode))
		{
			if (GetLastError() == STILL_ACTIVE)
				return false;
			throw windows::error();
		}

		*_pExitCode = *(int*)&dwExitCode;
		return true;
	}

	size_t to_stdin(const void* _pSource, size_t _Size) override
	{
		if (!hInputWrite)
			oTHROW0(permission_denied);

		if (_Size > UINT_MAX)
			throw std::invalid_argument("Windows supports only 32-bit sized writes");

		oCHECK_SIZE(DWORD, _Size);
		DWORD dwSizeofWritten = 0;
		oVB(WriteFile(hInputWrite, _pSource, static_cast<DWORD>(_Size), &dwSizeofWritten, 0));
		return dwSizeofWritten;
	}

	size_t from_stdout(void* _pDestination, size_t _Size) override
	{
		if (!hOutputRead)
			oTHROW0(permission_denied);
	
		DWORD Available = 0;
		oVB(PeekNamedPipe(hOutputRead, nullptr, 0, nullptr, &Available, nullptr));
		if (0 == Available)
			return 0;

		if (_Size > UINT_MAX)
			throw std::invalid_argument("Windows supports only 32-bit sized reads");

		oCHECK_SIZE(DWORD, _Size);
		DWORD dwSizeofRead = 0;
		oVB(ReadFile(hOutputRead, _pDestination, static_cast<DWORD>(_Size), &dwSizeofRead, 0));
		return dwSizeofRead;
	}

private:
	info Info;
	PROCESS_INFORMATION ProcessInfo;
	STARTUPINFO StartInfo;
	HANDLE hOutputRead;
	HANDLE hOutputWrite;
	HANDLE hInputRead;
	HANDLE hInputWrite;
	HANDLE hErrorWrite;
	std::string CommandLine;
	std::string EnvironmentString;
	std::string InitialWorkingDirectory;
	bool Suspended;
};

std::shared_ptr<process> process::make(const info& _Info)
{
	return std::make_shared<windows_process>(_Info);
}

void process::enumerate(const std::function<bool(id _ID, id _ParentID, const path& _ProcessExePath)>& _Enumerator)
{
	windows::scoped_handle hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(entry);
	BOOL keepLooking = Process32First(hSnapshot, &entry);

	while (keepLooking)
	{
		id c, p;
		*(unsigned int*)&c = entry.th32ProcessID;
		*(unsigned int*)&p = entry.th32ParentProcessID;
		if (!_Enumerator(c, p, entry.szExeFile))
			break;
		entry.dwSize = sizeof(entry);
		keepLooking = Process32Next(hSnapshot, &entry);
	}
}

void enumerate_threads(process::id _ID, const std::function<bool(oStd::thread::id _ThreadID)>& _Enumerator)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());
	if (hSnapshot == INVALID_HANDLE_VALUE)
		throw windows::error();

	THREADENTRY32 entry;
	entry.dwSize = sizeof(THREADENTRY32);

	BOOL keepLooking = Thread32First(hSnapshot, &entry);

	if (!keepLooking)
		oTHROW(no_such_process, "no process threads found");

	while (keepLooking)
	{
		if (asdword(_ID) == entry.th32OwnerProcessID)
			if (!_Enumerator(*(oStd::thread::id*)&entry.th32ThreadID))
				break;
		keepLooking = Thread32Next(hSnapshot, &entry);
	}

	oVB(CloseHandle(hSnapshot));
}

process::id process::get_id(const char* _Name)
{
	path Find(_Name);
	id pid;
	enumerate([&](id _ID, id _ParentID, const path& _ProcessExePath)->bool
	{
		if (Find == _ProcessExePath)
		{
			pid = _ID;
			return false;
		}
		return true;
	});
	return pid;
}

bool process::has_debugger_attached(id _ID)
{
	if (_ID != id() && _ID != ouro::this_process::get_id())
	{
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ|PROCESS_DUP_HANDLE, FALSE, asdword(_ID));
		if (hProcess)
		{
			BOOL present = FALSE;
			BOOL result = CheckRemoteDebuggerPresent(hProcess, &present);
			CloseHandle(hProcess);

			if (result)
				return !!present;
			else
				throw windows::error();
		}

		oTHROW0(no_such_process);
	}

	return !!IsDebuggerPresent();
}

void process::wait(id _ID)
{
	wait_for_ms(_ID, INFINITE);
}

bool process::wait_for_ms(id _ID, unsigned int _TimeoutMS)
{
	windows::scoped_handle hProcess = OpenProcess(SYNCHRONIZE, FALSE, asdword(_ID));
	bool result = true; // No process means it's exited
	if (hProcess)
		result = WAIT_OBJECT_0 == ::WaitForSingleObject(hProcess, _TimeoutMS);
	return result;
}

static void terminate_internal(process::id _ID
	, int _ExitCode
	, bool _AllChildProcessesToo
	, std::set<process::id>& _HandledProcessIDs);

static bool terminate_child(process::id _ID
	, process::id _ParentID
	, const path& _ProcessExePath
	, process::id _TargetParentID
	, int _ExitCode
	, bool _AllChildProcessesToo
	, std::set<process::id>& _HandledProcessIDs)
{
	if (_ParentID == _TargetParentID)
		terminate_internal(_ID, _ExitCode, _AllChildProcessesToo, _HandledProcessIDs);
	return true;
}

static void terminate_internal(process::id _ID
	, int _ExitCode
	, bool _AllChildProcessesToo
	, std::set<process::id>& _HandledProcessIDs)
{
	bool result = true;

	// do not recurse
	if (_HandledProcessIDs.find(_ID) != _HandledProcessIDs.end())
		return;

	_HandledProcessIDs.insert(_ID);

	if (_AllChildProcessesToo)
		process::enumerate(std::bind(terminate_child, _1, _2, _3, _ID, _ExitCode
			, _AllChildProcessesToo, std::ref(_HandledProcessIDs)));

	windows::scoped_handle hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, asdword(_ID));
	if (!hProcess)
		oTHROW0(no_such_process);

	path ProcessName = process::get_name(_ID);
	oTRACE("Terminating process %u (%s) with ExitCode %u", *(unsigned int*)&_ID, ProcessName.c_str(), _ExitCode);
	if (!TerminateProcess(hProcess, _ExitCode))
		throw windows::error();
}

void process::terminate(id _ID, int _ExitCode, bool _AllChildProcessesToo)
{
	std::set<process::id> HandledProcessIDs;
	terminate_internal(_ID, _ExitCode, _AllChildProcessesToo, HandledProcessIDs);
}

void process::terminate_children(id _ID, int _ExitCode, bool _AllChildProcessesToo)
{
	std::set<process::id> HandledProcessIDs;
	process::enumerate(std::bind(terminate_child, _1, _2, _3, _ID, _ExitCode
		, _AllChildProcessesToo, std::ref(HandledProcessIDs)));
}

path process::get_name(id _ID)
{
	windows::scoped_handle hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, asdword(_ID));
	path_string Temp;
	oCHECK_SIZE(unsigned int, Temp.capacity());
	oVB(GetModuleFileNameExA(hProcess, nullptr, Temp.c_str(), static_cast<unsigned int>(Temp.capacity())));
	return path(Temp);
}

process::memory_info process::get_memory_info(id _ID)
{
	PROCESS_MEMORY_COUNTERS_EX m;
	memset(&m, 0, sizeof(m));
	m.cb = sizeof(m);

	windows::scoped_handle hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, asdword(_ID));
	if (hProcess)
		oVB(GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&m, m.cb));
	else
		oTHROW0(no_such_process);

	process::memory_info mi;
	mi.working_set = m.WorkingSetSize;
	mi.working_set_peak = m.PeakWorkingSetSize;
	mi.nonshared_usage = m.PrivateUsage;
	mi.pagefile_usage = m.PagefileUsage;
	mi.pagefile_usage_peak = m.PeakPagefileUsage;
	mi.page_fault_count = m.PageFaultCount;

	return mi;
}

process::time_info process::get_time_info(id _ID)
{
	FILETIME c, e, k, u;
	{
		windows::scoped_handle hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, asdword(_ID));
		if (!hProcess)
			oTHROW0(no_such_process);
		oVB(GetProcessTimes(hProcess, &c, &e, &k, &u));
	}

	process::time_info ti;

	ti.start = date_cast<time_t>(c);

	// running processes don't have an exit time yet, so use 0
	if (e.dwLowDateTime || e.dwHighDateTime)
		ti.exit = date_cast<time_t>(e);
	else
		ti.exit = 0;

	LARGE_INTEGER li;
	li.LowPart = k.dwLowDateTime;
	li.HighPart = k.dwHighDateTime;
	ti.kernel = chrono::duration_cast<chrono::seconds>(file_time(li.QuadPart)).count();
	li.LowPart = u.dwLowDateTime;
	li.HighPart = u.dwHighDateTime;
	ti.user = chrono::duration_cast<chrono::seconds>(file_time(li.QuadPart)).count();

	return ti;
}

double process::cpu_usage(id _ID, unsigned long long* _pPreviousSystemTime, unsigned long long* _pPreviousProcessTime)
{
	double CPUUsage = 0.0f;

	FILETIME ftIdle, ftKernel, ftUser;
	oVB(GetSystemTimes(&ftIdle, &ftKernel, &ftUser));

	time_info ti = get_time_info(_ID);

	unsigned long long kernel = 0, user = 0;

	LARGE_INTEGER li;
	li.LowPart = ftKernel.dwLowDateTime;
	li.HighPart = ftKernel.dwHighDateTime;
	kernel = chrono::duration_cast<chrono::seconds>(file_time(li.QuadPart)).count();

	li.LowPart = ftUser.dwLowDateTime;
	li.HighPart = ftUser.dwHighDateTime;
	user = chrono::duration_cast<chrono::seconds>(file_time(li.QuadPart)).count();

	unsigned long long totalSystemTime = kernel + user;
	unsigned long long totalProcessTime = ti.kernel + ti.user;

	if (*_pPreviousSystemTime && *_pPreviousProcessTime)
	{
		unsigned long long totalSystemDiff = totalSystemTime - *_pPreviousSystemTime;
		unsigned long long totalProcessDiff = totalProcessTime - *_pPreviousProcessTime;

		CPUUsage = totalProcessDiff * 100.0 / totalSystemDiff;
	}
	
	*_pPreviousSystemTime = totalSystemTime;
	*_pPreviousProcessTime = totalProcessTime;

	if (_isnan(CPUUsage) || !_finite(CPUUsage))
		return 0.0;

	// If the diffs are measured at not exactly the same time we can get a value 
	// larger than 100%, so don't let that outside this API. This is probably 
	// because GetSystemTimes and oProcessGetTimeStats can't be atomically called 
	// together.
	return __min(CPUUsage, 100.0);
}

	namespace this_process {

process::id get_id()
{
	process::id ID; *(DWORD*)&ID = ::GetCurrentProcessId();
	return ID;
}

process::id get_parent_id()
{
	process::id ParentID, id = get_id();
	process::enumerate([&](process::id _ID, process::id _ParentID, const path& _ProcessExePath)->bool
	{
		if (_ID == id)
		{
			ParentID = _ParentID;
			return false;
		}
		return true;
	});
	return ParentID;
}

static bool find_oldest_and_thus_main_thread(oStd::thread::id _ThreadID, ULONGLONG* _pMinCreateTime, oStd::thread::id* _pOutMainThreadID)
{
	HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, TRUE, asdword(_ThreadID));
	if (hThread)
	{
		FILETIME tCreate, tExit, tKernel, tUser;

		if (GetThreadTimes(hThread, &tCreate, &tExit, &tKernel, &tUser))
		{
			ULONGLONG createTime = ((ULONGLONG)tCreate.dwHighDateTime << 32) | tCreate.dwLowDateTime;
			if (createTime && createTime < *_pMinCreateTime)
			{
				*_pMinCreateTime = createTime;
				*_pOutMainThreadID = _ThreadID;
			}
		}

		oVB(CloseHandle(hThread));
	}

	return true;
}

oStd::thread::id get_main_thread_id()
{
	oStd::thread::id id;
	ULONGLONG minCreateTime = MAXULONGLONG;
	enumerate_threads(std::bind(find_oldest_and_thus_main_thread, std::placeholders::_1, &minCreateTime, &id));
	return id;
}

bool has_debugger_attached()
{
	return !!IsDebuggerPresent();
}

char* command_line(char* _StrDestination, size_t _SizeofStrDestination, bool _ParametersOnly)
{
	const char* p = GetCommandLineA();
	if (_ParametersOnly)
	{
		// find the start
		int argc = 0;
		ouro::path AppPath = ouro::filesystem::app_path(true);
		const char** argv = ouro::argtok(malloc, AppPath, p, &argc);
		finally freeArgv([&] { if (argv) free(argv); });

		const char* exe = strstr(p, argv[0]);
		p = exe + strlen(argv[0]);
		p += strcspn(p, oWHITESPACE); // move to whitespace
		p += strspn(p, oWHITESPACE); // move past whitespace
	}

	if (strlcpy(_StrDestination, p, _SizeofStrDestination) >= _SizeofStrDestination)
		oTHROW0(no_buffer_space);
	return _StrDestination;
}

void enumerate_children(const std::function<bool(process::id _ChildID, const path& _ProcessExePath)>& _Enumerator)
{
	process::id Parent = get_id();

	process::enumerate([&](process::id _ID, process::id _ParentID, const path& _ProcessExePath)->bool
	{
		if (_ParentID == Parent && !_Enumerator(_ID, _ProcessExePath))
			return false;
		return true;
	});
}

void enumerate_threads(const std::function<bool(oStd::thread::id _ThreadID)>& _Enumerator)
{
	enumerate_threads(get_id(), _Enumerator);
}

	} // namespace this_process
} // namespace ouro
