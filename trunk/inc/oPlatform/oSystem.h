// $(header)
// API for dealing with system-wide functionality
#pragma once
#ifndef oSystem_h
#define oSystem_h

#include <oBasis/oPlatformFeatures.h>

struct oSYSTEM_HEAP_STATS
{
	unsigned long long TotalMemoryUsed;
	unsigned long long AvailablePhysical;
	unsigned long long TotalPhysical;
	unsigned long long AvailableVirtualProcess;
	unsigned long long TotalVirtualProcess;
	unsigned long long AvailablePaged;
	unsigned long long TotalPaged;
};

// Returns memory statistics for the current system
oAPI bool oSystemGetHeapStats(oSYSTEM_HEAP_STATS* _pStats);

// Reboots the current system. All operating system rules apply, so this call is
// merely a request that an active user or other applications can deny.
oAPI bool oSystemReboot();

// Shut down (power down) the current system. All operating system rules apply,
// so this call is merely a request that an active user or other applications
// can deny.
oAPI bool oSystemShutdown();

// Puts system in a low-power/suspended/sleeping state
oAPI bool oSystemSleep(); // @oooii-tony: wrapper for SetSuspendState

// Enable or disable the system from entering a low-power/sleep state.
oAPI bool oSystemAllowSleep(bool _Allow = true);

// Schedules the specified function to be called at the specified absolute time.
// Use oSystem
oAPI bool oSystemScheduleWakeup(time_t _PosixAbsoluteTime, oTASK _OnWake);

// oSystemExecute spawns a child process to execute the specified command 
// line. If a string buffer is specified, then after the process is 
// finished its stdout is read into the buffer. If the address of an 
// exitcode value is specified, the child process's exit code is filled 
// in. If the specified timeout is reached, then the exitcode will be 
// oERROR_REDUNDANT.
oAPI bool oSystemExecute(const char* _CommandLine, char* _StrStdout, size_t _SizeofStrStdOut, int* _pExitCode = 0, uint _ExecutionTimeout = oINFINITE_WAIT);

// Pool system for all processes to be relatively idle (i.e. <3% CPU usage). 
// This is primarily intended to determine heuristically when a computer is 
// "fully booted" meaning all system services and startup applications are done.
// (This arose from Windows apps stealing focus during startup and the 
// application at the time needed to be a startup app and go into fullscreen.
// Randomly, another startup app/service would steal focus, knocking our app out
// of fullscreen mode.)
oAPI bool oSystemWaitIdle(unsigned int _TimeoutMS = oINFINITE_WAIT);

oAPI bool oSystemGUIUsesGPUCompositing();
oAPI bool oSystemGUIEnableGPUCompositing(bool _Enable, bool _Force = false);

// Accessors for the environment variables passed into this process
bool oSystemSetEnvironmentVariable(const char* _Name, const char* _Value);
char* oSystemGetEnvironmentVariable(char* _Value, size_t _SizeofValue, const char* _Name);

// Fills _StrEnvironment with all environment variables delimited by '\n'
char* oGetEnvironmentString(char* _StrEnvironment, size_t _SizeofStrEnvironment);

enum oSYSPATH
{
	oSYSPATH_CWD, // current working directory
	oSYSPATH_APP, // application directory (path where exe is)
	oSYSPATH_APP_FULL, // full path (with filename) to application executable
	oSYSPATH_HOST, // returns the hostname of this computer
	oSYSPATH_TMP, // platform temporary directory
	oSYSPATH_SYS, // platform system directory
	oSYSPATH_OS, // platform installation directory
	oSYSPATH_DEV, // current project development root directory
	oSYSPATH_COMPILER_INCLUDES, // location of compiler includes
	oSYSPATH_DESKTOP, // platform current user desktop
	oSYSPATH_DESKTOP_ALLUSERS, // platform shared desktop
	oSYSPATH_P4ROOT, // current user's Perforce workspace root (requires P4PORT and P4USER env vars to be set)
	oSYSPATH_DATA, // the data path of the application
	oSYSPATH_EXECUTION, // hostname.processID.threadID
};

// Return the full path to one of the types of system paths enumerated by oSYSPATH
char* oSystemGetPath(char* _StrSysPath, size_t _SizeofStrSysPath, oSYSPATH _SysPath);

// Find a file in the specified system path. Returns _ResultingFullPath if successful,
// nullptr otherwise
char* oSystemFindInPath(char* _ResultingFullPath, size_t _SizeofResultingFullPath, oSYSPATH _SysPath, const char* _RelativePath, const char* _DotPath, oFUNCTION_PATH_EXISTS _PathExists);

// Searches all system and environment paths, as well as extraSearchPath which 
// is a string of paths delimited by semi-colons. _RelativePath is the filename/
// partial path to be matched against the various prefixes to get a full path.
// Returns _ResultingFullPath if successful, nullptr if no _RelativePath was 
// found.
char* oSystemFindPath(char* _ResultingFullPath, size_t _SizeofResultingFullPath, const char* _RelativePath, const char* _DotPath, const char* _ExtraSearchPath, oFUNCTION_PATH_EXISTS _PathExists);

// Sets the current working directory. Use oSystemGetPath to get CWD
bool oSetCWD(const char* _CWD);

// _____________________________________________________________________________
// Templated-on-size versions of the above API

template<size_t size> bool oSystemExecute(const char* _CommandLine, char (&_StrStdout)[size] , int* _pExitCode = 0, unsigned int _ExecutionTimeout = oINFINITE_WAIT) { return oSystemExecute(_CommandLine, _StrStdout, size, _pExitCode, _ExecutionTimeout); }
template<size_t size> char* oSystemGetEnvironmentVariable(char (&_Value)[size], const char* _Name) { return oSystemGetEnvironmentVariable(_Value, size, _Name); }
template<size_t size> char* oGetEnvironmentString(char (&_StrEnvironment)[size]) { return oSystemGetEnvironmentString(_pEnvironment, size); }
template<size_t size> char* oSystemGetPath(char (&_StrSysPath)[size], oSYSPATH _SysPath) { return oSystemGetPath(_StrSysPath, size, _SysPath); }
template<size_t size> char* oSystemFindInPath(char(&_ResultingFullPath)[size], oSYSPATH _SysPath, const char* _RelativePath, const char* _DotPath, oFUNCTION_PATH_EXISTS _PathExists) { return oSystemFindInPath(_ResultingFullPath, size, _SysPath, _RelativePath, _DotPath, _PathExists); }
template<size_t size> char* oSystemFindPath(char (&_ResultingFullPath)[size], const char* _RelativePath, const char* _DotPath, const char* _ExtraSearchPath, oFUNCTION_PATH_EXISTS _PathExists) { return oSystemFindPath(_ResultingFullPath, size, _RelativePath, _DotPath, _ExtraSearchPath, _PathExists); }

#endif
