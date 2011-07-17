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
// Calendar Time API

struct oDateTime
{
	unsigned short Year; // The full year.
	unsigned short Month; // [1,12] (January - December)
	unsigned short DayOfWeek; // [0,6] (Sunday - Saturday)
	unsigned short Day; // [1,31]
	unsigned short Hour; // [0,23]
	unsigned short Minute; // [0,59]
	unsigned short Second; // [0,59]
	unsigned short Milliseconds; // [0,999]
};

// 0 means equal
// >0 _DateTime1 > _DateTime2 (1 is later/more in the future than 2)
// <0 _DateTime1 < _DateTime2 (1 is eariler/more in the past than 2)
int oCompareDateTime(const oDateTime& _DateTime1, const oDateTime& _DateTime2);

// Get the current time in oDataTime format
bool oGetDateTime(oDateTime* _pDateTime);

time_t oConvertDateTime(const oDateTime& _DateTime);
void oConvertDateTime(oDateTime* _DateTime, time_t _Time);

// oToString and oFromString support a standard format:
// YYYY/MM/DD HH:MM:SS:MMS (3 digits of milliseconds)

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

bool oSysGUIUsesGPUCompositing();

// Set a time (preferably in the future) when a sleeping system should wake up
// Obviously a wakeup API cannot be called while the computer is system, so
// before using platform API to put the system to sleep, set up when it is to 
// reawake.
bool oScheduleWakeupAbsolute(time_t _AbsoluteTime, oFUNCTION<void()> _OnWake);
bool oScheduleWakeupRelative(unsigned int _TimeFromNowInMilliseconds, oFUNCTION<void()> _OnWake);

// Allow system to go to sleep (probably to default behavior), but more usefully
// setting this to false will prevent a computer going to sleep and leave it in
// a server-like mode (ES_AWAYMODE_REQUIRED on Windows) for long processes such
// as video processing.
void oSysAllowSleep(bool _Allow);

class oScopedDisableSystemSleep
{
	oScopedDisableSystemSleep() { oSysAllowSleep(false); }
	~oScopedDisableSystemSleep() { oSysAllowSleep(true); }
};

// _____________________________________________________________________________
// Templated-on-size versions of the above API

template<size_t size> inline bool oGetSysPath(char (&_StrSysPath)[size], oSYSPATH _SysPath) { return oGetSysPath(_StrSysPath, size, _SysPath); }
template<size_t size> inline bool oGetHostname(char (&_Hostname)[size]) { return oGetHostname(_Hostname, size); }
template<size_t size> inline bool oGetExePath(char (&_StrExePath)[size]) { return oGetExePath(_StrExePath, size); }
template<size_t size> inline bool oExecute(const char* _CommandLine, char (&_StrStdout)[size] , int* _pExitCode = 0, unsigned int _ExecutionTimeout = oINFINITE_WAIT) { return oExecute(_CommandLine, _StrStdout, size, _pExitCode, _ExecutionTimeout); }
template<size_t size> inline bool oGetEnvironmentString(char (&_StrEnvironment)[size]) { return oGetEnvironmentString(_pEnvironment, size); }

#endif
