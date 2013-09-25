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
#ifndef oCore_system_h
#define oCore_system_h

#include <functional>

namespace ouro {
	namespace system {

struct heap_info
{
	unsigned long long total_used;
	unsigned long long avail_physical;
	unsigned long long total_physical;
	unsigned long long avail_virtual_process;
	unsigned long long total_virtual_process;
	unsigned long long avail_paged;
	unsigned long long total_paged;
};

heap_info get_heap_info();

// Get the system's impression of current date/time in UTC (GMT).
template<typename dateT> void now(dateT* _pDate);

// Convert from a local timezone-specific time to UTC (GMT) time.
date from_local(const date& _LocalDate);

// Convert from UTC (GMT) time to a local timezone-specific time.
date to_local(const date& _UTCDate);

template<typename date1T, typename date2T> bool from_local(const date1T& _LocalDate, date2T* _pUTCDate)
{
	date local, utc;
	local = date_cast<date>(_LocalDate);
	if (!from_local(local, &utc)) return false;
	*_pUTCDate = date_cast<date2T>(utc);
	return true;
}

template<typename date1T, typename date2T> bool to_local(const date1T& _UTCDate, date2T* _pLocalDate)
{
	date local, utc;
	utc = date_cast<date>(_UTCDate);
	if (!to_local(utc, &local)) return false;
	*_pLocalDate = date_cast<date2T>(local);
	return true;
}

// All OS rules apply, so this is a request that UI may have an active user or 
// application deny.
void reboot();
void shutdown();
void sleep();
void allow_sleep(bool _Allow = true);
void schedule_wakeup(time_t _Time, const std::function<void()>& _OnWake);

// Poll system for all processes to be relatively idle (i.e. <3% CPU usage). 
// This is primarily intended to determine heuristically when a computer is 
// "fully booted" meaning all system services and startup applications are done.
// (This arose from Windows apps stealing focus during startup and the 
// application at the time needed to be a startup app and go into fullscreen.
// Randomly, another startup app/service would steal focus, knocking our app out
// of fullscreen mode.)
// _ContinueWaiting is an optional function called while waiting. If it returns 
// false the wait will terminate.
void wait_idle(const std::function<bool()>& _ContinueWaiting = nullptr);
bool wait_for_idle(unsigned int _TimeoutMS, const std::function<bool()>& _ContinueWaiting = nullptr);

bool uses_gpu_compositing();
void enable_gpu_compositing(bool _Enable = true, bool _Force = false);

// Returns true if the system is running in a mode similar to Window's 
// Remote Desktop. Use this when interacting with displays or feature set that
// might not be accurate by the software emulation done to redirect output to a 
// remote session.
bool is_remote_session();

// If the system is logged out, GUI draw commands will fail
bool gui_is_drawable();

char* host_name(char* _StrDestination, size_t _SizeofStrDestination);
template<size_t size> char* host_name(char (&_StrDestination)[size]) { return host_name(_StrDestination, size); }
template<size_t capacity> char* host_name(fixed_string<char, capacity>& _StrDestination) { return host_name(_StrDestination, _StrDestination.capacity()); }

char* workgroup_name(char* _StrDestination, size_t _SizeofStrDestination);
template<size_t size> char* workgroup_name(char (&_StrDestination)[size]) { return workgroup_name(_StrDestination, size); }
template<size_t capacity> char* workgroup_name(fixed_string<char, capacity>& _StrDestination) { return workgroup_name(_StrDestination, _StrDestination.capacity()); }

// fills destination with the string "[<hostname>.process_id.thread_id]"
char* exec_path(char* _StrDestination, size_t _SizeofStrDestination);
template<size_t size> char* exec_path(char (&_StrDestination)[size]) { return exec_path(_StrDestination, size); }
template<size_t capacity> char* exec_path(fixed_string<char, capacity>& _StrDestination) { return exec_path(_StrDestination, _StrDestination.capacity()); }

void setenv(const char* _EnvVarName, const char* _Value);
char* getenv(char* _Value, size_t _SizeofValue, const char* _EnvVarName);
template<size_t size> char* getenv(char (&_Value)[size], const char* _EnvVarName) { return getenv(_Value, size, _EnvVarName); }
template<size_t capacity> char* getenv(fixed_string<char, capacity>& _Value, const char* _EnvVarName) { return getenv(_Value, _Value.capacity(), _EnvVarName); }

// retrieve entire environment string
char* envstr(char* _StrEnvironment, size_t _SizeofStrEnvironment);
template<size_t size> char* envstr(char (&_StrEnvironment)[size]) { return envstr(_StrEnvironment, size); }
template<size_t capacity> char* envstr(fixed_string<char, capacity>& _StrEnvironment) { return envstr(_StrEnvironment, _StrEnvironment.capacity()); }

// Spawns a child process to execute the specified command line. For each line
// emitted to stdout by the process, _GetLine is called so this calling process
// can react to the child's output. This returns the exit code of the process,
// or if the timeout is reached, this will return std::errc::timed_out. This 
// can also return std::errc::operation_in_process if the process did not time 
// out but still is not ready to return an exit code.
int spawn(const char* _CommandLine
	, const std::function<void(char* _Line)>& _GetLine
	, bool _ShowWindow
	, unsigned int _ExecutionTimeout);

template<typename Rep, typename Period>
int spawn(const char* _CommandLine
	, const std::function<void(char* _Line)>& _GetLine
	, bool _ShowWindow
	, const oStd::chrono::duration<Rep, Period>& _Timeout)
{
	oStd::chrono::milliseconds ms = oStd::chrono::duration_cast<oStd::chrono::milliseconds>(_Timeout);
	return spawn(_CommandLine, _GetLine, _ShowWindow, static_cast<unsigned int>(ms.count()));
}

// Same as above, but no timeout
int spawn(const char* _CommandLine
	, const std::function<void(char* _Line)>& _GetLine
	, bool _ShowWindow);

	} // namespace system
} // namespace ouro

#endif
