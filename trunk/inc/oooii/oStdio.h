// $(header)

// Utility functions for interacting with the platform at a very basic level.

#pragma once
#ifndef oStdio_h
#define oStdio_h

#include <oooii/oStddef.h>

// _____________________________________________________________________________
// Platform API

// returns system ticks as seconds since program started
double oTimer();

// oTimer, but in in milliseconds
inline float oTimerMSF() { return (float)oTimer()*1000.0f; }
inline unsigned int oTimerMS() { return static_cast<unsigned int>(oTimerMSF()); }

// oExecute spawns a child process to execute the specified command line.
// If a string buffer is specified, then after the process is finished its 
// stdout is read into the buffer. If the address of an exitcode value is 
// specified, the child process's exit code is filled in. If the specified 
// timeout is reached, then the exitcode will be EINPROGRESS.
bool oExecute(const char* _CommandLine, char* _StrStdout, size_t _SizeofStrStdOut, int* _pExitCode = 0, unsigned int _ExecutionTimeout = oINFINITE_WAIT);

// _____________________________________________________________________________
// Environment API

enum oSYSPATH
{
	oSYSPATH_CWD, // current working directory
	oSYSPATH_APP, // application directory (path where exe is)
	oSYSPATH_TMP, // platform temporary directory
	oSYSPATH_SYS, // platform system directory
	oSYSPATH_OS, // platform installation directory
	oSYSPATH_DEV, // current project development root directory
	oSYSPATH_COMPILER_INCLUDES, // location of compiler includes
	oSYSPATH_DESKTOP, // platform current user desktop
	oSYSPATH_DESKTOP_ALLUSERS, // platform shared desktop
	oSYSPATH_P4ROOT, // current user's Perforce workspace root (requires P4PORT and P4USER env vars to be set)
};

// Return the full path to one of the types of system paths enumerated by oSYSPATH
bool oGetSysPath(char* _StrSysPath, size_t _SizeofStrSysPath, oSYSPATH _SysPath);

// Returns the name of this computer
bool oGetHostname(char* _Hostname, size_t _SizeofHostName);

// Wrapper for use assert/debug macros ONLY because it's returning thread-shared memory
inline const char* oGetHostname() { static char buf[512]; oGetHostname(buf, sizeof(buf)); return buf; }

// Returns the full path to the name of the file that is executing in this 
// process
bool oGetExePath(char* _ExePath, size_t _SizeofExePath);

// Sets the current working directory. Use oGetSysPath to get CWD
bool oSetCWD(const char* _CWD);

// Accessors for the environment variables passed into this process
bool oSetEnvironmentVariable(const char* _Name, const char* _Value);
bool oGetEnvironmentVariable(char* _Value, size_t _SizeofValue, const char* _Name);
template<size_t size> inline bool oGetEnvironmentVariable(char (&_Value)[size], const char* _Name) { return oGetEnvironmentVariable(_Value, size, _Name); }

// Fills _StrEnvironment with all environment variables delimited by '\n'
bool oGetEnvironmentString(char* _StrEnvironment, size_t _SizeofStrEnvironment);

// _____________________________________________________________________________
// Templated-on-size versions of the above API

template<size_t size> inline bool oGetSysPath(char (&_StrSysPath)[size], oSYSPATH _SysPath) { return oGetSysPath(_StrSysPath, size, _SysPath); }
template<size_t size> inline bool oGetHostname(char (&_Hostname)[size]) { return oGetHostname(_Hostname, size); }
template<size_t size> inline bool oGetExePath(char (&_StrExePath)[size]) { return oGetExePath(_StrExePath, size); }
template<size_t size> inline bool oExecute(const char* _CommandLine, char (&_StrStdout)[size] , int* _pExitCode = 0, unsigned int _ExecutionTimeout = oINFINITE_WAIT) { return oExecute(_CommandLine, _StrStdout, size, _pExitCode, _ExecutionTimeout); }
template<size_t size> inline bool oGetEnvironmentString(char (&_StrEnvironment)[size]) { return oGetEnvironmentString(_pEnvironment, size); }

#endif
