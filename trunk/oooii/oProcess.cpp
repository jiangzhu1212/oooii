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
#include <oooii/oProcess.h>
#include <oooii/oErrno.h>
#include <oooii/oRefCount.h>
#include <oooii/oWindows.h>
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
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<oProcess>());

	Process_Impl(const DESC* _pDesc, bool* _pSuccess);
	~Process_Impl();

	void Start() threadsafe override;
	bool Kill(int _ExitCode) threadsafe override;
	bool Wait(unsigned int _TimeoutMS = oINFINITE_WAIT) threadsafe override;
	bool GetCPUTime(unsigned __int64 *_pCPUTime) const threadsafe override;
	bool GetMemoryWorkingSetSize(size_t *_pWorkingSetSize) const threadsafe;
	bool GetExitCode(int* _pExitCode) const threadsafe override;
	size_t GetProcessID() const threadsafe override;
	size_t GetThreadId() const threadsafe override;

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
};

bool oProcess::Create(const DESC* _pDesc, threadsafe oProcess** _ppProcess)
{
	if (!_pDesc || !_ppProcess) return false;

	bool success = false;
	*_ppProcess = new Process_Impl(_pDesc, &success);
	if (!success)
	{
		delete *_ppProcess;
		*_ppProcess = 0;
	}

	return !!*_ppProcess;
}

Process_Impl::Process_Impl(const DESC* _pDesc, bool* _pSuccess)
	: Desc(*_pDesc)
	, CommandLine(_pDesc->CommandLine ? _pDesc->CommandLine : "")
	, EnvironmentString(_pDesc->EnvironmentString ? _pDesc->EnvironmentString : "")
	, hOutputRead(0)
	, hOutputWrite(0)
	, hInputRead(0)
	, hInputWrite(0)
	, hErrorWrite(0)
{
	Desc.CommandLine = CommandLine.c_str();
	Desc.EnvironmentString = EnvironmentString.c_str();

	// Always create suspended so a user can put a breakpoint after process
	// creation, but before it starts execution.
	DWORD dwCreationFlags = CREATE_SUSPENDED;

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
			oSetLastError(EPIPE);
			return;
		}

		oVB(DuplicateHandle(GetCurrentProcess(), hOutputWrite, GetCurrentProcess(), &hErrorWrite, 0, TRUE, DUPLICATE_SAME_ACCESS));

		if (!CreatePipe(&hInputRead, &hInputWriteTmp, &sa, 0))
		{
			oVB(CloseHandle(hOutputReadTmp));
			oVB(CloseHandle(hOutputWrite));
			*_pSuccess = false;
			oSetLastError(EPIPE);
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
}

bool Process_Impl::Kill(int _ExitCode) threadsafe
{
	HRESULT hr = TerminateProcess(ProcessInfo.hProcess, (UINT)_ExitCode);
	if (FAILED(hr))
	{
		oWinSetLastError(hr);
		return false;
	}

	return true;
}


bool Process_Impl::Wait(unsigned int _TimeoutMS) threadsafe
{
	return oWaitSingle(ProcessInfo.hProcess, _TimeoutMS);
}

bool Process_Impl::GetCPUTime(unsigned __int64 *_pCPUTime) const threadsafe
{
	FILETIME creationTime;
	FILETIME exitTime;
	FILETIME kernelTime;
	FILETIME userTime;

	if (!GetProcessTimes(ProcessInfo.hProcess, &creationTime, &exitTime, &kernelTime, &userTime))
	{
		return false;
	}

	*_pCPUTime = (Int64ShllMod32(kernelTime.dwHighDateTime, 32) | kernelTime.dwLowDateTime) +
								  (Int64ShllMod32(userTime.dwHighDateTime, 32) | userTime.dwLowDateTime);

	return true;
}

bool Process_Impl::GetMemoryWorkingSetSize(size_t *_pWorkingSetSize) const threadsafe
{
	PROCESS_MEMORY_COUNTERS_EX m;
	memset(&m, 0, sizeof(m));
	m.cb = sizeof(m);

	if (!oWinPSAPI::Singleton()->GetProcessMemoryInfo(ProcessInfo.hProcess, (PROCESS_MEMORY_COUNTERS*)&m, m.cb))
	{
		return false;
	}

	*_pWorkingSetSize = m.WorkingSetSize;
	return true;
}


size_t Process_Impl::GetThreadId() const threadsafe
{
	return ProcessInfo.dwThreadId;
}

size_t Process_Impl::GetProcessID() const threadsafe
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
		oSetLastError(EPIPE);
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
		oSetLastError(EPIPE);
		return 0;
	}

	oASSERT(_SizeofRead <= UINT_MAX, "Windows supports only 32-bit sized reads.");
	DWORD sizeofRead = 0;
	oVB(ReadFile(hOutputRead, _pDestination, static_cast<DWORD>(_SizeofRead), &sizeofRead, 0));
	return sizeofRead;
}
