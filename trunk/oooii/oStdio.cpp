// $(header)
#include <oooii/oStdio.h>
#include <oooii/oAssert.h>
#include <oooii/oPath.h>
#include <oooii/oP4.h>
#include <oooii/oProcess.h>
#include <oooii/oRef.h>
#include <oooii/oStddef.h>
#include <oooii/oWindows.h>
#include <io.h>
#include <regex>
using namespace std::tr1;

double oTimer()
{
	LARGE_INTEGER ticks, freq;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&ticks);
	return ticks.QuadPart / static_cast<double>(freq.QuadPart);
}

bool oGetSysPath(char* _StrSysPath, size_t _SizeofStrSysPath, oSYSPATH _SysPath)
{
	bool success = true;
	DWORD nElements = static_cast<DWORD>(_SizeofStrSysPath);

	switch (_SysPath)
	{
		case oSYSPATH_APP: oGetExePath(_StrSysPath, _SizeofStrSysPath); *oGetFilebase(_StrSysPath) = 0; break;
		case oSYSPATH_CWD: GetCurrentDirectoryA(nElements, _StrSysPath); break;
		case oSYSPATH_SYS: GetSystemDirectoryA(_StrSysPath, nElements); break;
		case oSYSPATH_OS: GetWindowsDirectoryA(_StrSysPath, nElements); break;
		case oSYSPATH_P4ROOT:
		{
			oP4::CLIENT_SPEC cspec;
			success = oP4::GetClientSpec(&cspec);
			if (success)
				strcpy_s(_StrSysPath, _SizeofStrSysPath, cspec.Root);
			break;
		}

		case oSYSPATH_DEV:
		{
			// fixme: find a better way to do this...
			success = oGetExePath(_StrSysPath, _SizeofStrSysPath);
			*oGetFilebase(_StrSysPath) = 0;
			success = 0 == strcat_s(_StrSysPath, _SizeofStrSysPath, "../../../"); // assumes $DEV/bin/$PLATFORM/$BUILDTYPE
			break;
		}

		case oSYSPATH_COMPILER_INCLUDES:
		{
			// @oooii-tony: Yes, sorta hard-coded but better than trying to get at this
			// directory elsewhere in user code.
			success = oGetEnvironmentVariable(_StrSysPath, _SizeofStrSysPath, "VS90COMNTOOLS");
			if (success)
			{
				oEnsureFileSeparator(_StrSysPath, _SizeofStrSysPath);
				strcat_s(_StrSysPath, _SizeofStrSysPath, "../../VC/include/");
				oCleanPath(_StrSysPath, _SizeofStrSysPath, _StrSysPath);
			}

			else
				oSetLastError(ENOENT, "Failed to find compiler include path becayse env var VS90COMNTOOLS does not exist");

			break;
		}

		case oSYSPATH_TMP:
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

		default: oASSUME(0);
	}

	if (success)
	{
		oEnsureFileSeparator(_StrSysPath, _SizeofStrSysPath);
		success = 0 == oCleanPath(_StrSysPath, _SizeofStrSysPath, _StrSysPath);
	}
	return success;
}

bool oGetHostname(char* _Hostname, size_t _SizeofHostname)
{
	DWORD dwSize = static_cast<DWORD>(_SizeofHostname);
	if (dwSize != _SizeofHostname)
	{
		oSetLastError(EINVAL, "Specified size is too large for underlying platform");
		return false;
	}

	if (!GetComputerNameEx(ComputerNameDnsHostname, _Hostname, &dwSize))
	{
		oSetLastErrorNative(::GetLastError());
		return false;
	}

	return true;
}

bool oGetExePath(char* _ExePath, size_t _SizeofExePath)
{
	DWORD length = GetModuleFileNameA(GetModuleHandle(0), _ExePath, static_cast<DWORD>(_SizeofExePath));
	if (length == (DWORD)_SizeofExePath && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		oSetLastError(STRUNCATE);
		return false;
	}

	return true;
}

bool oSetCWD(const char* _CWD)
{
	if (!SetCurrentDirectoryA(_CWD))
	{
		oSetLastErrorNative(::GetLastError());
		return false;
	}

	return true;
}

bool oExecute(const char* _CommandLine, char* _StrStdout, size_t _SizeofStrStdOut, int* _pExitCode, unsigned int _ExecutionTimeout)
{
	oProcess::DESC desc;
	desc.CommandLine = _CommandLine;
	desc.EnvironmentString = 0;
	desc.StdHandleBufferSize = 4096;
	oRef<threadsafe oProcess> process;
	if (!oProcess::Create(&desc, &process))
		return false;

	oTRACE("oExecute: \"%s\"...", oSAFESTRN(_CommandLine));
	process->Start();
	if (!process->Wait(_ExecutionTimeout))
	{
		oSetLastError(ETIMEDOUT, "Executing \"%s\" timed out after %.01f seconds.", _CommandLine, static_cast<float>(_ExecutionTimeout) / 1000.0f);
		if (_pExitCode)
			*_pExitCode = EINPROGRESS;
		return false;
	}

	if (_pExitCode && !process->GetExitCode(_pExitCode))
		*_pExitCode = EINPROGRESS;

	if (_StrStdout && _SizeofStrStdOut)
	{
		oTRACE("oExecute: Reading from stdout... \"%s\"", oSAFESTRN(_CommandLine));
		size_t sizeRead = process->ReadFromStdout(_StrStdout, _SizeofStrStdOut);
		oASSERT(sizeRead < _SizeofStrStdOut, "");
		_StrStdout[sizeRead] = 0;
	}

	return true;
}

bool oSetEnvironmentVariable(const char* _Name, const char* _Value)
{
	return !!SetEnvironmentVariableA(_Name, _Value);
}

bool oGetEnvironmentVariable(char* _Value, size_t _SizeofValue, const char* _Name)
{
	oASSERT((size_t)(int)_SizeofValue == _SizeofValue, "Invalid size");
	size_t len = GetEnvironmentVariableA(_Name, _Value, (int)_SizeofValue);
	return len && len < _SizeofValue;
}

bool oGetEnvironmentString(char* _StrEnvironment, size_t _SizeofStrEnvironment)
{
	char* pEnv = GetEnvironmentStringsA();

	// @oooii-tony: replace nuls with newlines to make parsing this mega-string
	// a bit less obtuse

	char* c = pEnv;
	size_t len = strlen(pEnv);
	while (len)
	{
		c[len] = '\n';
		c += len+1;
		len = strlen(c);
	}

	errno_t err = strcpy_s(_StrEnvironment, _SizeofStrEnvironment, pEnv);
	oVB(FreeEnvironmentStringsA(pEnv));

	if (err)
	{
		oSetLastError(err, "strcpy to user-specified buffer failed");
		return false;
	}

	return true;
}
