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
#include <oPlatform/oProcess.h>
#include <oBasis/oError.h>
#include <oBasis/oRefCount.h>
#include <oPlatform/Windows/oWindows.h>
#include <string>
#include <set>
#include "SoftLink/oWinPSAPI.h"

struct oWinProcess : public oProcess
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oProcess);
	oDEFINE_CONST_GETDESC_INTERFACE(Desc, threadsafe);

	oWinProcess(const DESC& _Desc, bool* _pSuccess);
	~oWinProcess();

	void Start() threadsafe override;
	bool Kill(int _ExitCode) threadsafe override;
	bool Wait(unsigned int _TimeoutMS = oInfiniteWait) threadsafe override;
	bool GetExitCode(int* _pExitCode) const threadsafe override;
	unsigned int GetProcessID() const threadsafe override;
	unsigned int GetThreadID() const threadsafe override;

	size_t WriteToStdin(const void* _pSource, size_t _SizeofWrite) threadsafe override;
	size_t ReadFromStdout(void* _pDestination, size_t _SizeofRead) threadsafe override;

	PROCESS_INFORMATION ProcessInfo;
	STARTUPINFO StartInfo;

	DESC Desc;
	HANDLE hOutputRead;
	HANDLE hOutputWrite;
	HANDLE hInputRead;
	HANDLE hInputWrite;
	HANDLE hErrorWrite;
	std::string CommandLine;
	std::string EnvironmentString;
	std::string InitialWorkingDirectory;
	oRefCount RefCount;
	bool Suspended;
};

bool oProcessCreate(const oProcess::DESC& _Desc, threadsafe oProcess** _ppProcess)
{
	if (!_ppProcess)
		return oErrorSetLast(std::errc::invalid_argument);
	bool success = false;
	oCONSTRUCT(_ppProcess, oWinProcess(_Desc, &success));
	if (success)
		oTRACE("Process %u created (attach to this in a debugger to get breakpoints)", (*_ppProcess)->GetProcessID());
	return success;
}

oWinProcess::oWinProcess(const DESC& _Desc, bool* _pSuccess)
	: Desc(_Desc)
	, CommandLine(oSAFESTR(_Desc.CommandLine))
	, EnvironmentString(oSAFESTR(_Desc.EnvironmentString))
	, InitialWorkingDirectory(oSAFESTR(_Desc.InitialWorkingDirectory))
	, hOutputRead(nullptr)
	, hOutputWrite(nullptr)
	, hInputRead(nullptr)
	, hInputWrite(nullptr)
	, hErrorWrite(nullptr)
	, Suspended(_Desc.StartSuspended)
{
	*_pSuccess = false;

	memset(&ProcessInfo, 0, sizeof(PROCESS_INFORMATION));
	memset(&StartInfo, 0, sizeof(STARTUPINFO));
	StartInfo.cb = sizeof(STARTUPINFO);

	if (!oSTRVALID(_Desc.CommandLine))
	{
		oErrorSetLast(std::errc::invalid_argument, "invalid command line");
		return;
	}

	Desc.CommandLine = CommandLine.c_str();
	Desc.EnvironmentString = EnvironmentString.c_str();
	Desc.InitialWorkingDirectory = InitialWorkingDirectory.c_str();

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = 0;
	sa.bInheritHandle = TRUE;

	DWORD dwCreationFlags = 0;
	if (_Desc.StartSuspended)
		dwCreationFlags |= CREATE_SUSPENDED;

	switch (Desc.Show)
	{
		case oPROCESS_HIDE:
			dwCreationFlags |= CREATE_NO_WINDOW;
			StartInfo.wShowWindow = SW_HIDE;
			break;
		case oPROCESS_SHOW:
			StartInfo.wShowWindow = SW_SHOWNOACTIVATE;
			break;
		case oPROCESS_SHOW_FOCUSED:
			StartInfo.wShowWindow = SW_SHOWNORMAL;
			break;
		case oPROCESS_SHOW_MINIMIZED:
			StartInfo.wShowWindow = SW_SHOWMINNOACTIVE;
			break;
		case oPROCESS_SHOW_MINIMIZED_FOCUSED:
			StartInfo.wShowWindow = SW_SHOWMINIMIZED;
			break;
		oNODEFAULT;
	}

	if (Desc.StdHandleBufferSize)
	{
		// Based on setup described here: http://support.microsoft.com/kb/190351

		HANDLE hOutputReadTmp = 0;
		HANDLE hInputWriteTmp = 0;
		if (!CreatePipe(&hOutputReadTmp, &hOutputWrite, &sa, 0))
		{
			oErrorSetLast(std::errc::permission_denied);
			return;
		}

		oVB(DuplicateHandle(GetCurrentProcess(), hOutputWrite, GetCurrentProcess(), &hErrorWrite, 0, TRUE, DUPLICATE_SAME_ACCESS));

		if (!CreatePipe(&hInputRead, &hInputWriteTmp, &sa, 0))
		{
			oVB(CloseHandle(hOutputReadTmp));
			oVB(CloseHandle(hOutputWrite));
			oErrorSetLast(std::errc::permission_denied);
			return;
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
	oStd::finally FreeEnv([&] { if (env) delete [] env; });
	if (!EnvironmentString.empty())
	{
		env = new char[EnvironmentString.length()+1];
		if (!oConvertEnvStringToEnvBlock(env, EnvironmentString.length()+1, EnvironmentString.c_str(), '\n'))
			return;
	}

	// Make a copy because CreateProcess does not take a const char*
	char* cmdline = nullptr;
	oStd::finally FreeCmd([&] { if (cmdline) delete [] cmdline; });
	if (!CommandLine.empty())
	{
		cmdline = new char[CommandLine.length()+1];
		oStrcpy(cmdline, CommandLine.length()+1, CommandLine.c_str());
	}

	oASSERT(cmdline, "a null cmdline can BSOD a machine");
	if (!CreateProcessA(nullptr, cmdline, nullptr, &sa, TRUE, dwCreationFlags, env
		, InitialWorkingDirectory.empty() ? nullptr : InitialWorkingDirectory.c_str()
		, &StartInfo, &ProcessInfo))
	{
		oWinSetLastError();
		return;
	}

	*_pSuccess = true;
}

oWinProcess::~oWinProcess()
{
	if (Suspended)
		Kill(std::errc::timed_out);
	if (ProcessInfo.hProcess) CloseHandle(ProcessInfo.hProcess);
	if (ProcessInfo.hThread) CloseHandle(ProcessInfo.hThread);
	if (hOutputRead) CloseHandle(hOutputRead);
	if (hOutputWrite) CloseHandle(hOutputWrite);
	if (hInputRead) CloseHandle(hInputRead);
	if (hInputWrite) CloseHandle(hInputWrite);
	if (hErrorWrite) CloseHandle(hErrorWrite);
}

void oWinProcess::Start() threadsafe
{
	if (Suspended)
	{
		oVB(ResumeThread(ProcessInfo.hThread));
		Suspended = false;
	}
}

bool oWinProcess::Kill(int _ExitCode) threadsafe
{
	return oProcessTerminate(ProcessInfo.dwProcessId, (unsigned int)_ExitCode);
}

bool oWinProcess::Wait(unsigned int _TimeoutMS) threadsafe
{
	return oWaitSingle(ProcessInfo.hProcess, _TimeoutMS);
}

unsigned int oWinProcess::GetThreadID() const threadsafe
{
	return ProcessInfo.dwThreadId;
}

unsigned int oWinProcess::GetProcessID() const threadsafe
{
	return ProcessInfo.dwProcessId;
}

bool oWinProcess::GetExitCode(int* _pExitCode) const threadsafe
{
	DWORD exitCode = 0;
	if (!GetExitCodeProcess(ProcessInfo.hProcess, &exitCode))
	{
		if (GetLastError() == STILL_ACTIVE)
			return false;
		oVB(false);
	}

	*_pExitCode = (int)exitCode;
	return true;
}

size_t oWinProcess::WriteToStdin(const void* _pSource, size_t _SizeofWrite) threadsafe
{
	if (!hInputWrite)
	{
		oErrorSetLast(std::errc::permission_denied);
		return 0;
	}

	oASSERT(_SizeofWrite <= UINT_MAX, "Windows supports only 32-bit sized writes.");
	DWORD sizeofWritten = 0;
	oVB(WriteFile(hInputWrite, _pSource, static_cast<DWORD>(_SizeofWrite), &sizeofWritten, 0));
	return sizeofWritten;
}

size_t oWinProcess::ReadFromStdout(void* _pDestination, size_t _SizeofRead) threadsafe
{
	if (!hOutputRead)
	{
		oErrorSetLast(std::errc::permission_denied);
		return 0;
	}
	
	DWORD Available = 0;
	oVB(PeekNamedPipe(hOutputRead, nullptr, 0, nullptr, &Available, nullptr));
	if (0 == Available)
		return 0;

	oASSERT(_SizeofRead <= UINT_MAX, "Windows supports only 32-bit sized reads.");
	DWORD sizeofRead = 0;
	oVB(ReadFile(hOutputRead, _pDestination, static_cast<DWORD>(_SizeofRead), &sizeofRead, 0));
	return sizeofRead;
}

unsigned int oProcessGetCurrentID()
{
	return ::GetCurrentProcessId();
}

bool oProcessEnum(oFUNCTION<bool(unsigned int _ProcessID, unsigned int _ParentProcessID, const char* _ProcessExePath)> _Function)
{
	return oWinEnumProcesses(_Function);
}

bool oProcessWaitExit(unsigned int _ProcessID, unsigned int _TimeoutMS)
{
	HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, _ProcessID);
	if (!hProcess)
	{
		// No process means it's exited
		return true;
	}

	bool result = oWaitSingle(hProcess, _TimeoutMS);
	CloseHandle(hProcess);
	if (!result)
		oErrorSetLast(std::errc::timed_out);
	return result;
}

static bool FindProcessByName(unsigned int _ProcessID, unsigned int _ParentProcessID, const char* _ProcessExePath, const char* _FindName, unsigned int* _pOutPID)
{
	if (!oStricmp(_FindName, _ProcessExePath))
	{
		*_pOutPID = _ProcessID;
		return false;
	}

	return true;
}

unsigned int oProcessGetID(const char* _Name)
{
	unsigned int pid = 0;
	oProcessEnum(oBIND(FindProcessByName, oBIND1, oBIND2, oBIND3, _Name, &pid));
	return pid;
}

char* oProcessGetName(char* _StrDestination, size_t _SizeofStrDestination, unsigned int _ProcessID)
{
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, _ProcessID);
	if (!hProcess)
		return nullptr;

	oStd::path_string Temp;
	if (0 == oWinPSAPI::Singleton()->GetModuleFileNameExA(hProcess, nullptr, Temp.c_str(), oUInt(Temp.capacity())))
		return (char*)oErrorSetLast(std::errc::no_such_file_or_directory, "failed to get name for process %u", _ProcessID);
	
	oStrcpy(_StrDestination, _SizeofStrDestination, oGetFilebase(Temp));
	return _StrDestination;
}

static const char* oGetCommandLineParameters(bool _ParametersOnly)
{
	const char* p = GetCommandLineA();
	if (!_ParametersOnly)
		return p;

	int argc = 0;
	const char** argv = oWinCommandLineToArgvA(true, p, &argc);
	oStd::finally OSCFreeArgv([&] { if (argv) oWinCommandLineToArgvAFree(argv); });

	const char* exe = strstr(p, argv[0]);
	const char* after = exe + oStrlen(argv[0]);

	p += strcspn(p, oWHITESPACE); // move to whitespace
	p += strspn(p, oWHITESPACE); // move past whitespace
	return p;
}

char* oProcessGetCommandLine(char* _StrDestination, size_t _SizeofStrDestination, bool _ParametersOnly)
{
	return oStrcpy(_StrDestination, _SizeofStrDestination, oGetCommandLineParameters(_ParametersOnly));
}

bool oProcessHasDebuggerAttached(unsigned int _ProcessID)
{
	if (_ProcessID && _ProcessID != oProcessGetCurrentID())
	{
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ|PROCESS_DUP_HANDLE, FALSE, _ProcessID);
		if (hProcess)
		{
			BOOL present = FALSE;
			BOOL result = CheckRemoteDebuggerPresent(hProcess, &present);
			CloseHandle(hProcess);

			if (result)
				return !!present;
			else
				oWinSetLastError();
		}

		else
			oErrorSetLast(std::errc::no_such_process);

		return false;
	}

	return !!IsDebuggerPresent();
}

bool oProcessGetMemoryStats(unsigned int _ProcessID, oPROCESS_MEMORY_STATS* _pStats)
{
	if (!_pStats)
	{
		oErrorSetLast(std::errc::invalid_argument);
		return false;
	}

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, _ProcessID);
	if (!hProcess)
	{
		oErrorSetLast(std::errc::no_such_process);
		return false;
	}

	BOOL result = false;
	PROCESS_MEMORY_COUNTERS_EX m;
	memset(&m, 0, sizeof(m));
	m.cb = sizeof(m);
	result = oWinPSAPI::Singleton()->GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&m, m.cb);
	oVB(CloseHandle(hProcess));
	if (result)
	{
		_pStats->NumPageFaults = m.PageFaultCount;
		_pStats->WorkingSet = m.WorkingSetSize;
		_pStats->WorkingSetPeak = m.PeakWorkingSetSize;
		_pStats->NonSharedUsage = m.PrivateUsage;
		_pStats->PageFileUsage = m.PagefileUsage;
		_pStats->PageFileUsagePeak = m.PeakPagefileUsage;
	}
	
	else
		oWinSetLastError();

	return !!result;
}

bool oProcessGetTimeStats(unsigned int _ProcessID, oPROCESS_TIME_STATS* _pStats)
{
	if (!_pStats)
		return oErrorSetLast(std::errc::invalid_argument);

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, _ProcessID);
	if (!hProcess)
		return oErrorSetLast(std::errc::no_such_process);

	BOOL result = false;
	FILETIME c, e, k, u;
	result = GetProcessTimes(hProcess, &c, &e, &k, &u);
	oVB(CloseHandle(hProcess));

	if (result)
	{
		_pStats->StartTime = oStd::date_cast<time_t>(c);

		// running processes don't have an exit time yet, so use 0
		if (e.dwLowDateTime || e.dwHighDateTime)
			_pStats->ExitTime = oStd::date_cast<time_t>(e);
		else
			_pStats->ExitTime = 0;

		LARGE_INTEGER li;
		li.LowPart = k.dwLowDateTime;
		li.HighPart = k.dwHighDateTime;
		_pStats->KernelTime = oStd::chrono::duration_cast<oStd::chrono::seconds>(oStd::file_time(li.QuadPart)).count();
		li.LowPart = u.dwLowDateTime;
		li.HighPart = u.dwHighDateTime;
		_pStats->UserTime = oStd::chrono::duration_cast<oStd::chrono::seconds>(oStd::file_time(li.QuadPart)).count();
	}

	else
		oErrorSetLast(std::errc::no_such_process);

	return !!result;
}

double oProcessCalculateCPUUsage(unsigned int _ProcessID, unsigned long long* _pPreviousSystemTime, unsigned long long* _pPreviousProcessTime)
{
	double CPUUsage = 0.0f;

	FILETIME ftIdle, ftKernel, ftUser;
	oVB(GetSystemTimes(&ftIdle, &ftKernel, &ftUser));

	oPROCESS_TIME_STATS s;
	oVERIFY(oProcessGetTimeStats(_ProcessID, &s));

	unsigned long long idle = 0, kernel = 0, user = 0;

	LARGE_INTEGER li;
	li.LowPart = ftIdle.dwLowDateTime;
	li.HighPart = ftIdle.dwHighDateTime;
	idle = oStd::chrono::duration_cast<oStd::chrono::seconds>(oStd::file_time(li.QuadPart)).count();

	li.LowPart = ftKernel.dwLowDateTime;
	li.HighPart = ftKernel.dwHighDateTime;
	kernel = oStd::chrono::duration_cast<oStd::chrono::seconds>(oStd::file_time(li.QuadPart)).count();

	li.LowPart = ftUser.dwLowDateTime;
	li.HighPart = ftUser.dwHighDateTime;
	user = oStd::chrono::duration_cast<oStd::chrono::seconds>(oStd::file_time(li.QuadPart)).count();

	unsigned long long totalSystemTime = kernel + user;
	unsigned long long totalProcessTime = s.KernelTime + s.UserTime;

	if (*_pPreviousSystemTime && *_pPreviousProcessTime)
	{
		unsigned long long totalSystemDiff = totalSystemTime - *_pPreviousSystemTime;
		unsigned long long totalProcessDiff = totalProcessTime - *_pPreviousProcessTime;

		CPUUsage = totalProcessDiff * 100.0 / totalSystemDiff;
	}
	
	*_pPreviousSystemTime = totalSystemTime;
	*_pPreviousProcessTime = totalProcessTime;

	if (isnan(CPUUsage) || isinf(CPUUsage))
		return 0.0;

	// @oooii-tony: sometimes if the diffs are measured at not exactly the same 
	// time we can get a value larger than 100%, so don't let that outside this 
	// API. I believe this is because GetSystemTimes and oProcessGetTimeStats 
	// can't be atomically called together. Performance counters promise this, but
	// I can't get them to work... take a look at oWinPDH.h|cpp for a starting
	// point.
	return __min(CPUUsage, 100.0);
}

void oProcessEnumHeapAllocations(oFUNCTION<void(oPROCESS_ALLOC_DESC& _Desc)> _Walker)
{
	oPROCESS_ALLOC_DESC desc;

	HANDLE heaps[128];
	DWORD numHeaps = GetProcessHeaps(oCOUNTOF(heaps), heaps);

	const HANDLE hProcessHeap = GetProcessHeap();
	const HANDLE hCrt = (HANDLE)_get_heap_handle();

	for (DWORD i = 0; i < numHeaps; i++)
	{
		ULONG heapInfo = 3;
		SIZE_T dummy = 0;
		HeapQueryInformation(heaps[i], HeapCompatibilityInformation, &heapInfo, sizeof(heapInfo), &dummy);
		desc.Type = oPROCESS_ALLOC_DESC::EXTERNAL;
		if (heaps[i] == hProcessHeap)
			desc.Type = oPROCESS_ALLOC_DESC::PROCESS;
		else if (heaps[i] == hCrt)
			desc.Type = oPROCESS_ALLOC_DESC::LIBC;

		PROCESS_HEAP_ENTRY e;
		e.lpData = 0;
		while (::HeapWalk(heaps[i], &e))
		{
			desc.BaseAddress = e.lpData;
			desc.Size = e.cbData;
			desc.Overhead = e.cbOverhead;
			desc.Used = (e.wFlags & PROCESS_HEAP_ENTRY_BUSY) != 0;
			_Walker(desc);
		}
	}
}

static bool ProcessTerminateWorker(unsigned int _ProcessID, unsigned int _ExitCode, bool _Recursive, std::set<int>& _HandledProcessIDs);

static bool TerminateChildProcess(unsigned int _ProcessID, unsigned int _ParentProcessID, const char* _ProcessExePath, unsigned int _TargetParentProcessID, unsigned int _ExitCode, bool _Recursive, std::set<int>& _HandledProcessIDs)
{
	if (_ParentProcessID == _TargetParentProcessID)
		ProcessTerminateWorker(_ProcessID, _ExitCode, _Recursive, _HandledProcessIDs);
	return true;
}

static bool ProcessTerminateWorker(unsigned int _ProcessID, unsigned int _ExitCode, bool _Recursive, std::set<int>& _HandledProcessIDs)
{
	bool result = true;

	if (_HandledProcessIDs.find(_ProcessID) != _HandledProcessIDs.end())
	{
		// return success in that it is not an error, but we have an infinite 
		// recursion here
		return true;
	}

	_HandledProcessIDs.insert(_ProcessID);

	if (_Recursive)
		oProcessEnum(oBIND(TerminateChildProcess, oBIND1, oBIND2, oBIND3, _ProcessID, _ExitCode, _Recursive, _HandledProcessIDs));

	HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, _ProcessID);
	if (!hProcess)
	{
		oErrorSetLast(std::errc::no_such_process);
		return false;
	}

	oStd::lstring ProcessName;
	oProcessGetName(ProcessName, _ProcessID);

	oTRACE("Terminating process %u (%s) with ExitCode %u", _ProcessID, ProcessName.c_str(), _ExitCode);
	if (!TerminateProcess(hProcess, _ExitCode))
	{
		oWinSetLastError();
		result = false;
	}

	CloseHandle(hProcess);
	return result;
}

bool oProcessTerminate(unsigned int _ProcessID, unsigned int _ExitCode, bool _Recursive)
{
	std::set<int> handledProcessIDs;
	bool returnValue = ProcessTerminateWorker(_ProcessID, _ExitCode, _Recursive, handledProcessIDs);
	return returnValue;
}

void oProcessTerminateChildren(unsigned int _ProcessID, unsigned int _ExitCode, bool _Recursive)
{
	std::set<int> handledProcessIDs;
	// process each task and call TerminateChildProcess on it
	oProcessEnum(oBIND(TerminateChildProcess, oBIND1, oBIND2, oBIND3, _ProcessID, _ExitCode, _Recursive, handledProcessIDs));
}