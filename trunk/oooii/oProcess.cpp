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
#include "pch.h"
#include <oooii/oProcess.h>
#include <oooii/oErrno.h>
#include <oooii/oRefCount.h>
#include <oooii/oWindows.h>
#include <string>
#include <oooii/oWinPSAPI.h>

struct Process_Impl : public oProcess
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);

	Process_Impl(const DESC* _pDesc, bool* _pSuccess);
	~Process_Impl();

	void Start() threadsafe override;
	void Kill(int _ExitCode) threadsafe override;
	bool Wait(unsigned int _TimeoutMS = oINFINITE_WAIT) threadsafe override;
	bool GetExitCode(int* _pExitCode) const threadsafe override;
	size_t WriteToStdin(const void* _pSource, size_t _SizeofWrite) threadsafe override;
	size_t ReadFromStdout(void* _pDestination, size_t _SizeofRead) threadsafe override;

	PROCESS_INFORMATION ProcessInfo;
	STARTUPINFO StartInfo;

	DESC Desc;
	HANDLE hPipeRead;
	HANDLE hPipeWrite;
	HANDLE hPipeError;
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

size_t oProcess::GetCurrentProcessID()
{
	return GetCurrentProcessId();
}

size_t oProcess::GetProcessHandle(const char* _pName)
{
	// From http://msdn.microsoft.com/en-us/library/ms682623(v=VS.85).aspx
	// Get the list of process identifiers.

	DWORD aProcesses[1024], cbNeeded, cProcesses;
	unsigned int i;

	oWinPSAPI* pPSAPI = oWinPSAPI::Singleton();

	if ( !pPSAPI->EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
		return NULL;

	// Calculate how many process identifiers were returned.

	cProcesses = cbNeeded / sizeof(DWORD);

	// Print the name and process identifier for each process.

	for ( i = 0; i < cProcesses; i++ )
		if( aProcesses[i] != 0 )
		{
			// Get a handle to the process.

			HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
				PROCESS_VM_READ | PROCESS_DUP_HANDLE ,
				FALSE, aProcesses[i] );

			// Get the process name.

			if (NULL != hProcess )
			{
				HMODULE hMod;
				DWORD cbNeeded;

				char szProcessName[MAX_PATH];
				if ( pPSAPI->EnumProcessModules( hProcess, &hMod, sizeof(hMod), 
					&cbNeeded) )
				{
					pPSAPI->GetModuleBaseNameA( hProcess, hMod, szProcessName, 
						sizeof(szProcessName)/sizeof(char) );
					if( strcmp( szProcessName, _pName ) == 0 )
					{
						return (size_t)hProcess;
					}
				}
			}
		}
		return NULL;
}

Process_Impl::Process_Impl(const DESC* _pDesc, bool* _pSuccess)
	: Desc(*_pDesc)
	, CommandLine(_pDesc->CommandLine ? _pDesc->CommandLine : "")
	, EnvironmentString(_pDesc->EnvironmentString ? _pDesc->EnvironmentString : "")
	, hPipeRead(0)
	, hPipeWrite(0)
	, hPipeError(0)
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

	if (Desc.StdHandleBufferSize)
	{
		// Based on setup described here: http://support.microsoft.com/kb/190351

		HANDLE hOutputReadTmp = 0, hOutputRead = 0;
		HANDLE hInputWriteTmp = 0, hInputWrite = 0;
		if (!CreatePipe(&hOutputReadTmp, &hPipeWrite, &sa, 0))
		{
			*_pSuccess = false;
			oSetLastError(EPIPE);
			return;
		}

		oVB(DuplicateHandle(GetCurrentProcess(), hPipeWrite, GetCurrentProcess(), &hPipeError, 0, TRUE, DUPLICATE_SAME_ACCESS));

		if (!CreatePipe(&hPipeRead, &hInputWriteTmp, &sa, 0))
		{
			oVB(CloseHandle(hOutputReadTmp));
			oVB(CloseHandle(hPipeWrite));
			*_pSuccess = false;
			oSetLastError(EPIPE);
			return;
		}

		oVB(DuplicateHandle(GetCurrentProcess(), hOutputReadTmp, GetCurrentProcess(), &hOutputRead, 0, FALSE, DUPLICATE_SAME_ACCESS));
		oVB(DuplicateHandle(GetCurrentProcess(), hInputWriteTmp, GetCurrentProcess(), &hInputWrite, 0, FALSE, DUPLICATE_SAME_ACCESS));

		oVB(CloseHandle(hOutputReadTmp));
		oVB(CloseHandle(hInputWriteTmp));

		// Create an anonymous pipe to hook std handles
		oASSERT(_pDesc->StdHandleBufferSize < UINT_MAX, "");
		*_pSuccess = !!CreatePipe(&hPipeRead, &hPipeWrite, &sa, static_cast<DWORD>(_pDesc->StdHandleBufferSize));
		StartInfo.dwFlags |= STARTF_USESTDHANDLES;
		StartInfo.hStdInput = hPipeRead;
		StartInfo.hStdOutput = hPipeWrite;
		StartInfo.hStdError = hPipeError;
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
	if (hPipeRead) CloseHandle(hPipeRead);
	if (hPipeWrite) CloseHandle(hPipeWrite);
	if (hPipeError) CloseHandle(hPipeError);
}

void Process_Impl::Start() threadsafe
{
	oVB(ResumeThread(ProcessInfo.hThread));
}

void Process_Impl::Kill(int _ExitCode) threadsafe
{
	oVB(TerminateProcess(ProcessInfo.hProcess, (UINT)_ExitCode));
}

bool Process_Impl::Wait(unsigned int _TimeoutMS) threadsafe
{
	return oWaitSingle(ProcessInfo.hProcess, _TimeoutMS);
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
	if (!hPipeWrite)
	{
		oSetLastError(EPIPE);
		return 0;
	}

	oASSERT(_SizeofWrite <= UINT_MAX, "Windows supports only 32-bit sized writes.");
	DWORD sizeofWritten = 0;
	oVB(WriteFile(hPipeWrite, _pSource, static_cast<DWORD>(_SizeofWrite), &sizeofWritten, 0));
	return sizeofWritten;
}

size_t Process_Impl::ReadFromStdout(void* _pDestination, size_t _SizeofRead) threadsafe
{
	if (!hPipeRead)
	{
		oSetLastError(EPIPE);
		return 0;
	}

	oASSERT(_SizeofRead <= UINT_MAX, "Windows supports only 32-bit sized reads.");
	DWORD sizeofRead = 0;
	oVB(ReadFile(hPipeRead, _pDestination, static_cast<DWORD>(_SizeofRead), &sizeofRead, 0));
	return sizeofRead;
}