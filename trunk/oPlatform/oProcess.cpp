// $(header)
#include <oPlatform/oProcess.h>
#include <oBasis/oError.h>
#include <oBasis/oRefCount.h>
#include <oPlatform/oWindows.h>
#include <string>
#include "oWinPSAPI.h"

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
	bool Wait(unsigned int _TimeoutMS = oINFINITE_WAIT) threadsafe override;
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
	, hOutputRead(0)
	, hOutputWrite(0)
	, hInputRead(0)
	, hInputWrite(0)
	, hErrorWrite(0)
	, Suspended(true) // Always create suspended so a user can put a breakpoint after process creation, but before it starts execution.
{
	DWORD dwCreationFlags = CREATE_SUSPENDED;

	Desc.CommandLine = CommandLine.c_str();
	Desc.EnvironmentString = EnvironmentString.c_str();

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
		strcpy_s(cmdline, CommandLine.length()+1, CommandLine.c_str());
	}

	// @oooii-tony: Always start suspended so the user can place a breakpoint
	// in the spawned code.
	bool success = !!CreateProcessA(0, cmdline, 0, &sa, TRUE, dwCreationFlags, env, 0, &StartInfo, &ProcessInfo);
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
	oVB(ResumeThread(ProcessInfo.hThread));
	Suspended = false;
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
	if (!_stricmp(_FindName, _ProcessExePath))
	{
		*_pOutPID = _ProcessID;
		return false;
	}

	return true;
}

bool oProcessExists(const char* _Name)
{
	unsigned int pid = 0;
	oProcessEnum(oBIND(FindProcessByName, oBIND1, oBIND2, oBIND3, _Name, &pid));
	return pid != 0;
}

bool oProcessHasDebuggerAttached(unsigned int _ProcessID)
{
	if (_ProcessID != oProcessGetCurrentID())
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
	FILETIME c, e, k, u;
	result = GetProcessTimes(hProcess, &c, &e, &k, &u);
	oVB(CloseHandle(hProcess));

	if (result)
	{
		_pStats->StartTime = oFileTimeToUnixTime(&c);
		_pStats->ExitTime = oFileTimeToUnixTime(&e);
		_pStats->KernelTime = oFileTimeToUnixTime(&k);
		_pStats->UserTime = oFileTimeToUnixTime(&u);
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

	time_t idle = oFileTimeToUnixTime(&ftIdle);
	time_t kernel = oFileTimeToUnixTime(&ftKernel);
	time_t user = oFileTimeToUnixTime(&ftUser);
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

	return CPUUsage;
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

//////////// Task Tree Termination

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

	oTRACE("Terminating process %u with ExitCode %u", _ProcessID, _ExitCode);
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
  {
		oProcessTerminateWorker(_ProcessID, _ExitCode, _Recursive);
  }
	return true;
}


void oProcessTerminateChildren(unsigned int _ProcessID, unsigned int _ExitCode, bool _Recursive)
{
  // process each task and call TerminateChildProcess on it
	oProcessEnum(oBIND(TerminateChildProcess, oBIND1, oBIND2, oBIND3, _ProcessID, _ExitCode, _Recursive));
}


// -i TESTRenderNodeServer -n 50