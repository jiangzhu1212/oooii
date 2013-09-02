/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
// API for dealing with system-wide functionality
#pragma once
#ifndef oSystem_h
#define oSystem_h

#include <oStd/date.h>
#include <oStd/function.h>
#include <oBasis/oPlatformFeatures.h>
#include <oBasis/oTimer.h> // for oInfiniteWait
#include <oBasis/oURI.h>
#include <oConcurrency/oConcurrency.h>

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

// Get the system's impression of current date/time to NTP-specified precision 
// in Universal Coordinated Time.
template<typename DATE_T> void oSystemGetDate(DATE_T* _pNTPDate);

// Convert from a local timezone-specific time to UTC (GMT) time.
oAPI bool oSystemDateFromLocal(const oStd::date& _LocalDate, oStd::date* _pUTCDate);

// Convert from UTC (GMT) time to a local timezone-specific time.
oAPI bool oSystemDateToLocal(const oStd::date& _UTCDate, oStd::date* _pLocalDate);

template<typename DATE1_T, typename DATE2_T> bool oSystemDateFromLocal(const DATE1_T& _LocalDate, DATE2_T* _pUTCDate)
{
	oStd::date local, utc;
	local = oStd::date_cast<oStd::date>(_LocalDate);
	if (!oSystemDateFromLocal(local, &utc)) return false;
	*_pUTCDate = oStd::date_cast<DATE2_T>(utc);
	return true;
}

template<typename DATE1_T, typename DATE2_T> bool oSystemDateToLocal(const DATE1_T& _UTCDate, DATE2_T* _pLocalDate)
{
	oStd::date local, utc;
	utc = oStd::date_cast<oStd::date>(_UTCDate);
	if (!oSystemDateToLocal(utc, &local)) return false;
	*_pLocalDate = oStd::date_cast<DATE2_T>(local);
	return true;
}

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
oAPI bool oSystemScheduleWakeup(time_t _UnixAbsoluteTime, const oTASK& _OnWake);

// oSystemExecute spawns a child process to execute the specified command line. 
// If a string buffer is specified, then after the process is finished its 
// stdout is read into the buffer. If the address of an exitcode value is 
// specified, the child process's exit code is filled in. If the specified 
// timeout is reached, then the exitcode will be operation_in_progress. Prefer 
// the default _ShowWindow=false for console executions, but for Window 
// applications requiring user interaction, use _ShowWindow=true.
bool oSystemExecute(const char* _CommandLine, const oFUNCTION<void(char* _Line)>& _GetLine = nullptr, int* _pExitCode = nullptr, bool _ShowWindow = false, unsigned int _ExecutionTimeout = oInfiniteWait);

// Pool system for all processes to be relatively idle (i.e. <3% CPU usage). 
// This is primarily intended to determine heuristically when a computer is 
// "fully booted" meaning all system services and startup applications are done.
// (This arose from Windows apps stealing focus during startup and the 
// application at the time needed to be a startup app and go into fullscreen.
// Randomly, another startup app/service would steal focus, knocking our app out
// of fullscreen mode.)
// _ContinueIdling is an optional function that is called while waiting.  If it ever returns
// false the wait will terminate  
oAPI bool oSystemWaitIdle(unsigned int _TimeoutMS = oInfiniteWait, oFUNCTION<bool()> _ContinueIdling = nullptr);

oAPI bool oSystemGUIUsesGPUCompositing();
oAPI bool oSystemGUIEnableGPUCompositing(bool _Enable, bool _Force = false);

// Returns true if the system is running in a mode similar to Window's 
// Remote Desktop. Use this when interacting with displays or feature set that
// might not be accurate by the software emulation done to redirect output to a 
// remote session.
oAPI bool oSystemIsRemote();

// Returns true if code in this process can render/write to the virtual desktop
// in the system's GUI. This returns false if front-buffer access or drawing
// GUI elements will fail, such as when the system is logged out.
oAPI bool oSystemGUIIsWritable();

// Accessors for the environment variables passed into this process
oAPI bool oSystemSetEnvironmentVariable(const char* _Name, const char* _Value);
oAPI char* oSystemGetEnvironmentVariable(char* _Value, size_t _SizeofValue, const char* _Name);

// Translates % encoded environment variables
oAPI char* oSystemTranslateEnvironmentVariables(char* _StrDestination, size_t _SizeofStrDestination, const char* _RawString);

// Fills _StrEnvironment with all environment variables delimited by '\n'
oAPI char* oSystemGetEnvironmentString(char* _StrEnvironment, size_t _SizeofStrEnvironment);

enum oSYSPATH
{
	oSYSPATH_CWD, // current working directory
	oSYSPATH_APP, // application directory (path where exe is)
	oSYSPATH_APP_FULL, // full path (with filename) to application executable
	oSYSPATH_SYSTMP, // platform temporary directory
	oSYSPATH_SYS, // platform system directory
	oSYSPATH_OS, // platform installation directory
	oSYSPATH_DEV, // current project development root directory
	oSYSPATH_TESTTMP, // unit test temp path. destroyed if unit tests succeed
	oSYSPATH_COMPILER_INCLUDES, // location of compiler includes
	oSYSPATH_DESKTOP, // platform current user desktop
	oSYSPATH_DESKTOP_ALLUSERS, // platform shared desktop
	oSYSPATH_SCCROOT, // the root of the source control under which this code runs
	oSYSPATH_DATA, // the data path of the application
};

// Return a URI to one of the system paths enumerated by oSYSPATH.
oAPI char* oSystemGetURI(char* _StrSysURI, size_t _SizeofStrSysURI, oSYSPATH _SysPath);

// Return the full path to one of the system paths enumerated by oSYSPATH.
oAPI char* oSystemGetPath(char* _StrSysPath, size_t _SizeofStrSysPath, oSYSPATH _SysPath);

// Returns the name of the current machine
oAPI char* oSystemGetHostname(char* _StrDestination, size_t _SizeofStrDestination);

// Returns [hostname.processID.threadID]
oAPI char* oSystemGetExecutionPath(char* _StrDestination, size_t _SizeofStrDestination);

// This is the same as oURIToPath but allows for the authority to first be 
// tested using oStd::from_string to a conversion to an oSYSPATH value. If it can, 
// that path is prepended to the _URIParts path.
oAPI char* oSystemURIToPath(char* _Path, size_t _SizeofPath, const char* _URI);

// This is the same as oURIPartsToPath but allows for the authority to first be 
// tested using oStd::from_string to a conversion to an oSYSPATH value. If it can, 
// that path is prepended to the _URIParts path.
oAPI char* oSystemURIPartsToPath(char* _Path, size_t _SizeofPath, const oURIParts& _URIParts);

// Find a file in the specified system path. Returns _ResultingFullPath if successful,
// nullptr otherwise
oAPI char* oSystemFindInPath(char* _ResultingFullPath, size_t _SizeofResultingFullPath, oSYSPATH _SysPath, const char* _RelativePath, const char* _DotPath, const oFUNCTION<bool(const char* _Path)>& _PathExists);

// Searches all system and environment paths, as well as extraSearchPath which 
// is a string of paths delimited by semi-colons. _RelativePath is the filename/
// partial path to be matched against the various prefixes to get a full path.
// Returns _ResultingFullPath if successful, nullptr if no _RelativePath was 
// found.
oAPI char* oSystemFindPath(char* _ResultingFullPath, size_t _SizeofResultingFullPath, const char* _RelativePath, const char* _DotPath, const char* _ExtraSearchPath, const oFUNCTION<bool(const char* _Path)>& _PathExists);

// Sets the current working directory. Use oSystemGetPath to get CWD
oAPI bool oSetCWD(const char* _CWD);
oAPI const char *oSystemAppendDebugSuffixToDLL(const char *_pModuleName, char *_pAppendedName, int _SizeOfAppendedName);

// _____________________________________________________________________________
// Templated-on-size versions of the above API

template<size_t size> bool oSystemExecute(const char* _CommandLine, char (&_StrStdout)[size], int* _pExitCode = 0, unsigned int _ExecutionTimeout = oInfiniteWait) { return oSystemExecute(_CommandLine, _StrStdout, size, _pExitCode, _ExecutionTimeout); }
template<size_t size> char* oSystemGetEnvironmentVariable(char (&_Value)[size], const char* _Name) { return oSystemGetEnvironmentVariable(_Value, size, _Name); }
template<size_t size> char* oSystemTranslateEnvironmentVariables(char (&_Value)[size], const char* _Name) { return oSystemTranslateEnvironmentVariables(_Value, size, _Name); }
template<size_t size> char* oGetEnvironmentString(char (&_StrEnvironment)[size]) { return oSystemGetEnvironmentString(_StrEnvironment, size); }
template<size_t size> char* oSystemGetURI(char (&_StrSysURI)[size], oSYSPATH _SysPath) { return oSystemGetURI(_StrSysURI, _SizeofStrSysURI, _SysPath); }
template<size_t size> char* oSystemGetPath(char (&_StrSysPath)[size], oSYSPATH _SysPath) { return oSystemGetPath(_StrSysPath, size, _SysPath); }
template<size_t size> char* oSystemGetHostname(char (&_StrDestination)[size]) { return oSystemGetHostname(_StrDestination, size); }
template<size_t size> char* oSystemGetExecutionPath(char (&_StrDestination)[size]) { return oSystemGetExecutionPath(_StrDestination, size); }
template<size_t size> char* oSystemURIToPath(char (&_ResultingFullPath)[size], const char* _URI) { return oSystemURIToPath(_ResultingFullPath, size, _URI); }
template<size_t size> char* oSystemURIPartsToPath(char (&_ResultingFullPath)[size], const oURIParts& _URIParts) { return oSystemURIPartsToPath(_ResultingFullPath, size, _URIParts); }
template<size_t size> char* oSystemFindInPath(char(&_ResultingFullPath)[size], oSYSPATH _SysPath, const char* _RelativePath, const char* _DotPath, const oFUNCTION<bool(const char* _Path)>& _PathExists) { return oSystemFindInPath(_ResultingFullPath, size, _SysPath, _RelativePath, _DotPath, _PathExists); }
template<size_t size> char* oSystemFindPath(char (&_ResultingFullPath)[size], const char* _RelativePath, const char* _DotPath, const char* _ExtraSearchPath, const oFUNCTION<bool(const char* _Path)>& _PathExists) { return oSystemFindPath(_ResultingFullPath, size, _RelativePath, _DotPath, _ExtraSearchPath, _PathExists); }

#include <oStd/fixed_string.h>
template<size_t capacity> char* oSystemGetEnvironmentVariable(oStd::fixed_string<char, capacity>& _Value, const char* _Name) { return oSystemGetEnvironmentVariable(_Value, _Value.capacity(), _Name); }
template<size_t capacity> char* oSystemTranslateEnvironmentVariables(oStd::fixed_string<char, capacity>& _Value, const char* _Name) { return oSystemTranslateEnvironmentVariables(_Value, _Value.capacity(), _Name); }
template<size_t capacity> char* oGetEnvironmentString(oStd::fixed_string<char, capacity>& _StrEnvironment) { return oSystemGetEnvironmentString(_StrEnvironment, _StrEnvironment.capacity()); }
template<size_t capacity> char* oSystemGetURI(oStd::fixed_string<char, capacity>& _StrSysURI, oSYSPATH _SysPath) { return oSystemGetURI(_StrSysURI, _StrSysURI.capacity(), _SysPath); }
template<size_t capacity> char* oSystemGetPath(oStd::fixed_string<char, capacity>& _StrSysPath, oSYSPATH _SysPath) { return oSystemGetPath(_StrSysPath, _StrSysPath.capacity(), _SysPath); }
template<size_t capacity> char* oSystemGetHostname(oStd::fixed_string<char, capacity>& _StrDestination) { return oSystemGetHostname(_StrDestination, _StrDestination.capacity()); }
template<size_t capacity> char* oSystemGetExecutionPath(oStd::fixed_string<char, capacity>& _StrDestination) { return oSystemGetExecutionPath(_StrDestination, _StrDestination.capacity()); }
template<size_t capacity> char* oSystemURIToPath(oStd::fixed_string<char, capacity>& _ResultingFullPath, const char* _URI) { return oSystemURIToPath(_ResultingFullPath, _ResultingFullPath.capacity(), _URI); }
template<size_t capacity> char* oSystemURIPartsToPath(oStd::fixed_string<char, capacity>& _ResultingFullPath, const oURIParts& _URIParts) { return oSystemURIPartsToPath(_ResultingFullPath, _ResultingFullPath.capacity(), _URIParts); }
template<size_t capacity> char* oSystemFindInPath(oStd::fixed_string<char, capacity>& _ResultingFullPath, oSYSPATH _SysPath, const char* _RelativePath, const char* _DotPath, const oFUNCTION<bool(const char* _Path)>& _PathExists) { return oSystemFindInPath(_ResultingFullPath, _ResultingFullPath.capacity(), _SysPath, _RelativePath, _DotPath, _PathExists); }
template<size_t capacity> char* oSystemFindPath(oStd::fixed_string<char, capacity>& _ResultingFullPath, const char* _RelativePath, const char* _DotPath, const char* _ExtraSearchPath, const oFUNCTION<bool(const char* _Path)>& _PathExists) { return oSystemFindPath(_ResultingFullPath, _ResultingFullPath.capacity(), _RelativePath, _DotPath, _ExtraSearchPath, _PathExists); }

#endif
