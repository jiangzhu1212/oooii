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
// Abstraction for a platform's process 
#pragma once
#ifndef oCore_process_h
#define oCore_process_h

#include <oStd/chrono.h>
#include <oBase/path.h>

namespace ouro {

class process
{
public:
	class id
	{
	public:
		id() : Handle(0) {}

		bool operator==(const id& _That) const { return Handle == _That.Handle; }
		bool operator!=(const id& _That) const { return !(*this == _That); }
		operator bool() const { return !!Handle; }

	private:
		unsigned int Handle;
	};

	enum show_type
	{
		hide,
		show,
		focused,
		minimized,
		minimized_focused,
	};

	struct info
	{
		info()
			: command_line(nullptr)
			, environment(nullptr)
			, initial_directory(nullptr)
			, stdout_buffer_size(0)
			, show(hide)
			, suspended(false)
		{}

		const char* command_line;
		const char* environment;
		const char* initial_directory;
		size_t stdout_buffer_size;
		show_type show;
		bool suspended;
	};

	struct memory_info
	{
		memory_info()
			: working_set(0)
			, working_set_peak(0)
			, nonshared_usage(0)
			, pagefile_usage(0)
			, pagefile_usage_peak(0)
			, page_fault_count(0)
		{}

		unsigned long long working_set;
		unsigned long long working_set_peak;
		unsigned long long nonshared_usage;
		unsigned long long pagefile_usage;
		unsigned long long pagefile_usage_peak;
		unsigned int page_fault_count;
	};

	struct time_info
	{
		time_info()
			: start(0)
			, exit(0)
			, kernel(0)
			, user(0)
		{}

		time_t start; // actual date
		time_t exit; // actual date (0 for currently-running processes)
		time_t kernel; // amount of time since start of application
		time_t user; // amount of time since start of application
	};

	virtual info get_info() const = 0;
		
	// if this process was created suspended, this resumes execution. This is not
	// needed if the process was started with suspected = false.
	virtual void start() = 0;

	virtual void kill(int _ExitCode) = 0;

	virtual void wait() = 0;

	// returns false if the wait times out
	template<typename Rep, typename Period>
	inline bool wait_for(const oStd::chrono::duration<Rep, Period>& _RelativeTime)
	{
		oStd::chrono::milliseconds ms = oStd::chrono::duration_cast<oStd::chrono::milliseconds>(_RelativeTime);
		return wait_for_ms(get_id(), static_cast<unsigned int>(ms.count()));
	}

	virtual id get_id() const = 0;
	virtual oStd::thread::id get_thread_id() const = 0;
		
	// Return exit code, if the process is finished. If not, this returns false.
	virtual bool exit_code(int* _pExitCode) const = 0;

	virtual size_t to_stdin(const void* _pSource, size_t _Size) = 0;
	virtual size_t from_stdout(void* _pDestination, size_t _Size) = 0;

	static std::shared_ptr<process> make(const info& _Info);

	static void enumerate(const std::function<bool(id _ID, id _ParentID, const path& _ProcessExePath)>& _Enumerator);
	static id get_id(const char* _Name);
	static inline bool exists(const char* _Name) { return process::id() != get_id(_Name); }
	static bool has_debugger_attached(id _ID);
	static void wait(id _ID);

	// returns false if the wait times out
	template<typename Rep, typename Period>
	static bool wait_for(id _ID, const oStd::chrono::duration<Rep, Period>& _RelativeTime)
	{
		oStd::chrono::milliseconds ms = oStd::chrono::duration_cast<oStd::chrono::milliseconds>(_RelativeTime);
		return wait_for_ms(_ID, static_cast<unsigned int>(ms.count()));
	}

	static void terminate(id _ID, int _ExitCode, bool _AllChildProcessesToo = true);
	static void terminate_children(id _ID, int _ExitCode, bool _AllChildProcessesToo = true);

	// Returns the short name of the process (module name) specified by its ID.
	static path get_name(id _ID);

	static memory_info get_memory_info(id _ID);
	static time_info get_time_info(id _ID);

	// Returns overall system CPU percentage [0,100] usage. This requires keeping
	// values around from a previous run. These should be initialized to 0 to 
	// indicate there's no prior value and from then on this function should be 
	// called regularly with those addresses passed in to retain the previous 
	// sample's values. The values should be unique per process queried.
	// WARNING: Because this samples diffs between two time periods, there should 
	// be some reasonable time between the two measurements, or strange results 
	// can come back. The sampling should also be regular, so the best best is to 
	// set up a separate monitor thread that sleeps for an interval and wakes up 
	// to call this.
	static double cpu_usage(id _ID, unsigned long long* _pPreviousSystemTime, unsigned long long* _pPreviousProcessTime);

protected:
	static bool wait_for_ms(id _ProcessID, unsigned int _TimeoutMS);
};

	namespace this_process {

process::id get_id();
process::id get_parent_id();
inline bool is_child() { return get_parent_id() != process::id(); }
bool has_debugger_attached();

inline double cpu_usage(unsigned long long* _pPreviousSystemTime, unsigned long long* _pPreviousProcessTime) { return ouro::process::cpu_usage(get_id(), _pPreviousSystemTime, _pPreviousProcessTime); }

// Returns the command line used to start this process. If _ParametersOnly is 
// true then the path of the exe will not be returned. The buffer must still be 
// large enough to hold that path.
char* command_line(char* _StrDestination, size_t _SizeofStrDestination, bool _ParametersOnly = false);
template<size_t size> char* command_line(char (&_StrDestination)[size], bool _ParametersOnly = false) { return command_line(_StrDestination, size, _ParametersOnly); }
template<size_t capacity> char* command_line(fixed_string<char, capacity>& _StrDestination, bool _ParametersOnly = false) { return command_line(_StrDestination, _StrDestination.capacity(), _ParametersOnly); }

// Call the specified function for each of the child processes of the current
// process. The function should return true to keep enumerating, or false to
// exit early. This function returns false if there is a failure, check 
// oErrorGetLast() for more information. The error can be 
// std::errc::no_child_process if there are no child processes.
void enumerate_children(const std::function<bool(process::id _ID, process::id _ParentID, const path& _ProcessExePath)>& _Enumerator);

	} // namespace this_process
} // namespace ouro

#endif
