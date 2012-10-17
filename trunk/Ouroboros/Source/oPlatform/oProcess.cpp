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
#include <oPlatform/oProcess.h>
#include <oBasis/oError.h>
#include <oBasis/oRefCount.h>
#include <oPlatform/Windows/oWindows.h>
#include <string>
#include "SoftLink/oWinPSAPI.h"

const oGUID& oGetGUID(threadsafe const oProcess* threadsafe const *)
{
	// {EAA75587-9771-4d9e-A2EA-E406AA2E8B8F}
	static const oGUID oIIDProcess = { 0xeaa75587, 0x9771, 0x4d9e, { 0xa2, 0xea, 0xe4, 0x6, 0xaa, 0x2e, 0x8b, 0x8f } };
	return oIIDProcess;
}

struct Process_Impl : public oProcess
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oProcess);
	oDEFINE_CONST_GETDESC_INTERFACE(Desc, threadsafe);

	Process_Impl(const DESC& _Desc, bool* _pSuccess);
	~Process_Impl();

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
		return oErrorSetLast(oERROR_INVALID_PARAMETER);
	bool success = false;
	oCONSTRUCT(_ppProcess, Process_Impl(_Desc, &success));
	if (success)
		oTRACE("Process %u created (attach to this in a debugger to get breakpoints)", (*_ppProcess)->GetProcessID());
	return success;
}

Process_Impl::Process_Impl(const DESC& _Desc, bool* _pSuccess)
	: Desc(_Desc)
	, CommandLine(oSAFESTR(_Desc.CommandLine))
	, EnvironmentString(oSAFESTR(_Desc.EnvironmentString))
	, InitialWorkingDirectory(oSAFESTR(_Desc.InitialWorkingDirectory))
	, hOutputRead(0)
	, hOutputWrite(0)
	, hInputRead(0)
	, hInputWrite(0)
	, hErrorWrite(0)
	#if _DEBUG
		, Suspended(_Desc.StartSuspended)
	#else
		, Suspended(false)
	#endif
{
	DWORD dwCreationFlags = 0;

	// Start suspended so that a debugger can be attached to the process before
	// really executing it in case debugging must occur near the start of the new 
	// process. Only do this in debug because in release, this can BSOD if the 
	// process doesn't exist (i.e. CreateProcess would fail in debug, BSODs in 
	// release).
	#ifdef _DEBUG
		if (_Desc.StartSuspended)
			dwCreationFlags |= CREATE_SUSPENDED;
	#endif

	Desc.CommandLine = CommandLine.c_str();
	Desc.EnvironmentString = EnvironmentString.c_str();
	Desc.InitialWorkingDirectory = InitialWorkingDirectory.c_str();

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = 0;
	sa.bInheritHandle = TRUE;

	memset(&ProcessInfo, 0, sizeof(PROCESS_INFORMATION));
	memset(&StartInfo, 0, sizeof(STARTUPINFO));
	StartInfo.cb = sizeof(STARTUPINFO);

	if (!Desc.SetFocus || Desc.StartMinimized)
	{
		StartInfo.dwFlags |= STARTF_USESHOWWINDOW;
		
		if (!Desc.SetFocus)
			StartInfo.wShowWindow |= (Desc.StartMinimized ? SW_SHOWMINNOACTIVE : SW_SHOWNOACTIVATE);
		else
			StartInfo.wShowWindow |= (Desc.StartMinimized ? SW_SHOWMINIMIZED : SW_SHOWDEFAULT);
	}

	if (Desc.StdHandleBufferSize)
	{
		// Based on setup described here: http://support.microsoft.com/kb/190351

		HANDLE hOutputReadTmp = 0;
		HANDLE hInputWriteTmp = 0;
		if (!CreatePipe(&hOutputReadTmp, &hOutputWrite, &sa, 0))
		{
			*_pSuccess = false;
			oErrorSetLast(oERROR_REFUSED);
			return;
		}

		oVB(DuplicateHandle(GetCurrentProcess(), hOutputWrite, GetCurrentProcess(), &hErrorWrite, 0, TRUE, DUPLICATE_SAME_ACCESS));

		if (!CreatePipe(&hInputRead, &hInputWriteTmp, &sa, 0))
		{
			oVB(CloseHandle(hOutputReadTmp));
			oVB(CloseHandle(hOutputWrite));
			*_pSuccess = false;
			oErrorSetLast(oERROR_REFUSED);
			return;
		}

		oVB(DuplicateHandle(GetCurrentProcess(), hOutputReadTmp, GetCurrentProcess(), &hOutputRead, 0, FALSE, DUPLICATE_SAME_ACCESS));
		oVB(DuplicateHandle(GetCurrentProcess(), hInputWriteTmp, GetCurrentProcess(), &hInputWrite, 0, FALSE, DUPLICATE_SAME_ACCESS));

		oVB(CloseHandle(hOutputReadTmp));
		oVB(CloseHandle(hInputWriteTmp));

		StartInfo.dwFlags |= STARTF_USESTDHANDLES;
		StartInfo.hStdOutput = hOutputWrite;
		StartInfo.hStdInput = hInputRead;
		StartInfo.hStdError = hErrorWrite;
	}

	else
	{
		dwCreationFlags |= CREATE_NEW_CONSOLE;
	}

	char* env = 0;
	if (!EnvironmentString.empty())
	{
		env = new char[EnvironmentString.length()+1];
		if (!oConvertEnvStringToEnvBlock(env, EnvironmentString.length()+1, EnvironmentString.c_str(), '\n'))
		{
			if (_pSuccess)
				*_pSuccess = false;
			return;
		}
	}

	// @oooii-tony: Make a copy because CreateProcess does not take a const char*
	char* cmdline = 0;
	if (!CommandLine.empty())
	{
		cmdline = new char[CommandLine.length()+1];
		oStrcpy(cmdline, CommandLine.length()+1, CommandLine.c_str());
	}

	bool success = !!CreateProcessA(0, cmdline, 0, &sa, TRUE, dwCreationFlags, env, InitialWorkingDirectory.empty() ? nullptr : InitialWorkingDirectory.c_str(), &StartInfo, &ProcessInfo);
	oVB(success);
	if (_pSuccess)
		*_pSuccess = success;

	if (env) delete env;
	if (cmdline) delete cmdline;
}

Process_Impl::~Process_Impl()
{
	if (Suspended)
		Kill(oERROR_TIMEOUT);
	if (ProcessInfo.hProcess) CloseHandle(ProcessInfo.hProcess);
	if (ProcessInfo.hThread) CloseHandle(ProcessInfo.hThread);
	if (hOutputRead) CloseHandle(hOutputRead);
	if (hOutputWrite) CloseHandle(hOutputWrite);
	if (hInputRead) CloseHandle(hInputRead);
	if (hInputWrite) CloseHandle(hInputWrite);
	if (hErrorWrite) CloseHandle(hErrorWrite);
}

void Process_Impl::Start() threadsafe
{
	if (Suspended)
	{
		oVB(ResumeThread(ProcessInfo.hThread));
		Suspended = false;
	}
}

bool Process_Impl::Kill(int _ExitCode) threadsafe
{
	return oProcessTerminate(ProcessInfo.dwProcessId, (unsigned int)_ExitCode);
}

bool Process_Impl::Wait(unsigned int _TimeoutMS) threadsafe
{
	return oWaitSingle(ProcessInfo.hProcess, _TimeoutMS);
}

unsigned int Process_Impl::GetThreadID() const threadsafe
{
	return ProcessInfo.dwThreadId;
}

unsigned int Process_Impl::GetProcessID() const threadsafe
{
	return ProcessInfo.dwProcessId;
}

bool Process_Impl::GetExitCode(int* _pExitCode) const threadsafe
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

size_t Process_Impl::WriteToStdin(const void* _pSource, size_t _SizeofWrite) threadsafe
{
	if (!hInputWrite)
	{
		oErrorSetLast(oERROR_REFUSED);
		return 0;
	}

	oASSERT(_SizeofWrite <= UINT_MAX, "Windows supports only 32-bit sized writes.");
	DWORD sizeofWritten = 0;
	oVB(WriteFile(hInputWrite, _pSource, static_cast<DWORD>(_SizeofWrite), &sizeofWritten, 0));
	return sizeofWritten;
}

size_t Process_Impl::ReadFromStdout(void* _pDestination, size_t _SizeofRead) threadsafe
{
	if (!hOutputRead)
	{
		oErrorSetLast(oERROR_REFUSED);
		return 0;
	}

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
		oErrorSetLast(oERROR_TIMEOUT);
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

	oStringPath Temp;
	if (0 == oWinPSAPI::Singleton()->GetModuleFileNameExA(hProcess, nullptr, Temp.c_str(), oUInt(Temp.capacity())))
		return (char*)oErrorSetLast(oERROR_NOT_FOUND, "failed to get name for process %u", _ProcessID);
	
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
	oOnScopeExit OSCFreeArgv([&] { if (argv) oWinCommandLineToArgvAFree(argv); });

	const char* exe = strstr(p, argv[0]);
	const char* after = exe + strlen(argv[0]);

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
			oErrorSetLast(oERROR_NOT_FOUND, "no such process");

		return false;
	}

	return !!IsDebuggerPresent();
}

bool oProcessGetMemoryStats(unsigned int _ProcessID, oPROCESS_MEMORY_STATS* _pStats)
{
	if (!_pStats)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER);
		return false;
	}

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, _ProcessID);
	if (!hProcess)
	{
		oErrorSetLast(oERROR_NOT_FOUND, "no such process");
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
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, _ProcessID);
	if (!hProcess)
		return oErrorSetLast(oERROR_NOT_FOUND, "no such process");

	BOOL result = false;
	FILETIME c, e, k, u;
	result = GetProcessTimes(hProcess, &c, &e, &k, &u);
	oVB(CloseHandle(hProcess));

	if (result)
	{
		oVERIFY(oDateConvert(c, &_pStats->StartTime));

		// running processes don't have an exit time yet, so use 0
		if (e.dwLowDateTime || e.dwHighDateTime)
			oVERIFY(oDateConvert(e, &_pStats->ExitTime));
		else
			_pStats->ExitTime = 0;

		LARGE_INTEGER li;
		li.LowPart = k.dwLowDateTime;
		li.HighPart = k.dwHighDateTime;
		_pStats->KernelTime = oStd::chrono::duration_cast<oStd::chrono::seconds>(oFileTime100NanosecondUnits(li.QuadPart)).count();
		li.LowPart = u.dwLowDateTime;
		li.HighPart = u.dwHighDateTime;
		_pStats->UserTime = oStd::chrono::duration_cast<oStd::chrono::seconds>(oFileTime100NanosecondUnits(li.QuadPart)).count();
	}

	else
		oErrorSetLast(oERROR_NOT_FOUND, "no such process");

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
	idle = oStd::chrono::duration_cast<oStd::chrono::seconds>(oFileTime100NanosecondUnits(li.QuadPart)).count();

	li.LowPart = ftKernel.dwLowDateTime;
	li.HighPart = ftKernel.dwHighDateTime;
	kernel = oStd::chrono::duration_cast<oStd::chrono::seconds>(oFileTime100NanosecondUnits(li.QuadPart)).count();

	li.LowPart = ftUser.dwLowDateTime;
	li.HighPart = ftUser.dwHighDateTime;
	user = oStd::chrono::duration_cast<oStd::chrono::seconds>(oFileTime100NanosecondUnits(li.QuadPart)).count();

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

#include <set>
static std::set<int> killProcess;

bool oProcessTerminateWorker(unsigned int _ProcessID, unsigned int _ExitCode, bool _Recursive)
{
	bool result = true;

  if (killProcess.find(_ProcessID) != killProcess.end())
  {
    // return success in that it is not an error, but we have an infinite recusion here
    return true;
  }

  killProcess.insert(_ProcessID);

	if (_Recursive)
		oProcessTerminateChildren(_ProcessID, _ExitCode, _Recursive);

	HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, _ProcessID);
	if (!hProcess)
	{
		oErrorSetLast(oERROR_NOT_FOUND, "no such process");
		return false;
	}

	oStringL ProcessName;
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
  killProcess.clear();
  bool returnValue = oProcessTerminateWorker(_ProcessID, _ExitCode, _Recursive);
  killProcess.clear(); // clear it afterward, otherwise a memory leak is reported
  return returnValue;
}

static bool TerminateChildProcess(unsigned int _ProcessID, unsigned int _ParentProcessID, const char* _ProcessExePath, unsigned int _TargetParentProcessID, unsigned int _ExitCode, bool _Recursive)
{
	if (_ParentProcessID == _TargetParentProcessID)
		oProcessTerminateWorker(_ProcessID, _ExitCode, _Recursive);
	return true;
}

void oProcessTerminateChildren(unsigned int _ProcessID, unsigned int _ExitCode, bool _Recursive)
{
  // process each task and call TerminateChildProcess on it
	oProcessEnum(oBIND(TerminateChildProcess, oBIND1, oBIND2, oBIND3, _ProcessID, _ExitCode, _Recursive));
}
