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
// Approximation of the upcoming C++11 condition_variable interface.
// NOTE: According to the C++11 spec, spurious wakeups are allowable. The
// Windows implementation documents that spurious wakeups indeed occur. Use
// the Guarded Suspension pattern (Google it) to address this issue.

#pragma once
#ifndef oStdConditionVariable_h
#define oStdConditionVariable_h

#include <oStd/oStdChrono.h>
#include <oStd/oStdMutex.h>
#include <cassert>

// To keep the main classes neat, collect all the platform-specific forward
// declaration here. This is done in this vague manner to avoid including 
// platform headers in this file.
#if defined(_WIN32) || defined(_WIN64)
	#define oCONDITION_VARIABLE_FOOTPRINT() void* Footprint;
#else
	#error Unsupported platform (oCONDITION_VARIABLE_FOOTPRINT)
#endif

namespace oStd {
	namespace condition_variable_requirements
	{
		void thread_at_exit(const std::function<void()>& _AtExit);

	} // namespace condition_variable_requirements

namespace detail { class promised_base; }

/*enum class*/ namespace cv_status { enum value { no_timeout, timeout }; };

class condition_variable
{
	oCONDITION_VARIABLE_FOOTPRINT();

	condition_variable(condition_variable const&); /* = delete */
	condition_variable& operator=(condition_variable const&); /* = delete */

	friend class detail::promised_base;
	cv_status::value wait_for(unique_lock<mutex>& _Lock, unsigned int _TimeoutMS);

public:
	condition_variable();
	~condition_variable();

	void notify_one();
	void notify_all();

	void wait(unique_lock<mutex>& _Lock);

	template <typename Predicate>
	void wait(unique_lock<mutex>& _Lock, Predicate _Predicate)
	{
		while (!_Predicate())
			wait(_Lock);
	}

	template <typename Clock, typename Duration>
	cv_status::value wait_until(unique_lock<mutex>& _Lock, const chrono::time_point<Clock, Duration>& _AbsoluteTime)
	{
		chrono::high_resolution_clock::duration duration = time_point_cast<chrono::high_resolution_clock::time_point>(_AbsoluteTime) - chrono::high_resolution_clock::now();
		return wait_for(_Lock, duration);
	}

	template <typename Clock, typename Duration, typename Predicate>
	bool wait_until(unique_lock<mutex>& _Lock, const chrono::time_point<Clock, Duration>& _AbsoluteTime, Predicate _Predicate)
	{
		assert(_Lock.owns_lock() && "Lock must own the mutex lock");
		while (!_Predicate())
		{
			if (wait_until(_Lock, _AbsoluteTime) == timeout)
				return _Predicate();
		}

		return true;
	}

	template <typename Rep, typename Period>
	cv_status::value wait_for(unique_lock<mutex>& _Lock, const chrono::duration<Rep, Period>& _RelativeTime)
	{
		chrono::milliseconds ms = chrono::duration_cast<chrono::milliseconds>(_RelativeTime);
		return wait_for(_Lock, static_cast<unsigned int>(ms.count()));
	}

	template <typename Rep, typename Period, typename Predicate>
	bool wait_for(unique_lock<mutex>& _Lock, const chrono::duration<Rep, Period>& _RelativeTime, Predicate _Predicate)
	{
		while (!_Predicate())
		{
			cv_status::value status = wait_for(_Lock, _RelativeTime);
			if (status == timeout)
				return _Predicate();
		}
		return true;
	}

	typedef void* native_handle_type;
	native_handle_type native_handle() { return Footprint; }
};

void notify_all_at_thread_exit(condition_variable& _ConditionVariable, unique_lock<mutex> _Lock);

} // namespace oStd

#endif
