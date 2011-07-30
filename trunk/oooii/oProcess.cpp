// $(header)
#include <oooii/oProcess.h>
#include <oooii/oErrno.h>
#include <oooii/oRefCount.h>
#include <oooii/oThreading.h>
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
	oDEFINE_CONST_GETDESC_INTERFACE(Desc, threadsafe);

	Process_Impl(const DESC* _pDesc, bool* _pSuccess);
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

	if (*_ppProcess)
		oTRACE("Process %u created (attach to this in a debugger to get breakpoints)", (*_ppProcess)->GetProcessID());

	return !!*_ppProcess;
}

Process_Impl::Process_Impl(const DESC* _pDesc, bool* _pSuccess)
	: Desc(*_pDesc)
	, CommandLine(oSAFESTR(_pDesc->CommandLine))
	, EnvironmentString(oSAFESTR(_pDesc->EnvironmentString))
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
