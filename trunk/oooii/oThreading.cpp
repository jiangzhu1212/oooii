// $(header)
#include <oooii/oThreading.h>
#include <oooii/oAssert.h>
#include <oooii/oErrno.h>
#include <oooii/oEvent.h>
#include <oooii/oFile.h>
#include <oooii/oRef.h>
#include <oooii/oRefCount.h>
#include <oooii/oSingleton.h>
#include <oooii/oThread.h>
#include <oooii/oWindows.h>
#include "oWinPSAPI.h"

unsigned int oThreadGetCurrentID()
{
	return ::GetCurrentThreadId();
}

static bool FindOldestAndThusMainThread(unsigned int _ThreadID, unsigned int _ProcessID, ULONGLONG* _pMinCreateTime, unsigned int* _pOutMainThreadID)
{
	HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, TRUE, _ThreadID);
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

unsigned int oThreadGetMainID()
{
	unsigned int mainThreadID = 0;
	ULONGLONG minCreateTime = MAXULONGLONG;
	oEnumProcessThreads(::GetCurrentProcessId(), oBIND(FindOldestAndThusMainThread, oBIND1, oBIND2, &minCreateTime, &mainThreadID));
	return mainThreadID;
}

void* oThreadGetCurrentNativeHandle()
{
	return ::GetCurrentThread();
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
		oSetLastError(ETIMEDOUT);
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
	if (_ProcessID)
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
			oSetLastError(ESRCH);

		return false;
	}

	return !!IsDebuggerPresent();
}

static bool TerminateChildProcess(unsigned int _ProcessID, unsigned int _ParentProcessID, const char* _ProcessExePath, unsigned int _TargetParentProcessID, unsigned int _ExitCode, bool _Recursive)
{
	if (_ParentProcessID == _TargetParentProcessID)
		oProcessTerminate(_ProcessID, _ExitCode, _Recursive);
	return true;
}

bool oProcessTerminate(unsigned int _ProcessID, unsigned int _ExitCode, bool _Recursive)
{
	if (_Recursive)
		oProcessTerminateChildren(_ProcessID, _ExitCode, _Recursive);

	HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, _ProcessID);
	if (!hProcess)
	{
		oSetLastError(ESRCH);
		return false;
	}

	oTRACE("Terminating process %u with ExitCode %u", _ProcessID, _ExitCode);
	bool result = true;
	if (!TerminateProcess(hProcess, _ExitCode))
	{
		oWinSetLastError();
		result = false;
	}

	CloseHandle(hProcess);
	return result;
}

void oProcessTerminateChildren(unsigned int _ProcessID, unsigned int _ExitCode, bool _Recursive)
{
	oProcessEnum(oBIND(TerminateChildProcess, oBIND1, oBIND2, oBIND3, _ProcessID, _ExitCode, _Recursive));
}

bool oProcessGetMemoryStats(unsigned int _ProcessID, oPROCESS_MEMORY_STATS* _pStats)
{
	if (!_pStats)
	{
		oSetLastError(EINVAL);
		return false;
	}

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, _ProcessID);
	if (!hProcess)
	{
		oSetLastError(ESRCH);
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
		oSetLastError(EINVAL);
		return false;
	}

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, _ProcessID);
	if (!hProcess)
	{
		oSetLastError(ESRCH);
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
		oSetLastError(ESRCH);

	return !!result;
}

void oYield()
{
	#if defined(_WIN32) || defined(_WIN64)
		SwitchToThread();
	#else
		#error Unsupported platform
	#endif
}

void oSleep(unsigned int _Milliseconds)
{
	Sleep((unsigned int)_Milliseconds);
}

void oSerialFor(oFUNCTION<void(size_t _Index)> _Function, size_t _Begin, size_t _End)
{
	for (size_t i = _Begin; i < _End; i++)
		_Function(i);
}

void oRunSerialTasks(oFUNCTION<void()>* _pFunctions, size_t _NumFunctions)
{
	for (size_t i = 0; i < _NumFunctions; i++)
		_pFunctions[i]();
}
