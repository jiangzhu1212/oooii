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
// Thin wrapper for Std::condition_variable that provides threadsafe 
// specification to reduce thread_cast throughout the code. Prefer using 
// these wrappers over using std::condition_variable directly.
#pragma once
#ifndef oConcurrency_condition_variable_h
#define oConcurrency_condition_variable_h

#include <oConcurrency/mutex.h>
#include <oConcurrency/thread_safe.h>
#include <oStd/condition_variable.h>

namespace oConcurrency {

class condition_variable
{
	oStd::condition_variable ConditionVariable;
	oStd::condition_variable& CV() const threadsafe { return thread_cast<oStd::condition_variable&>(ConditionVariable); }
	oStd::unique_lock<oStd::mutex>& L(unique_lock<mutex>& _Lock) const threadsafe
	{
		return *(oStd::unique_lock<oStd::mutex>*)&_Lock;
	}

	condition_variable(condition_variable const&); /* = delete */
	condition_variable& operator=(condition_variable const&); /* = delete */

public:
	condition_variable() {}
	~condition_variable() {}

	void notify_one() threadsafe { CV().notify_one(); }
	void notify_all() threadsafe { CV().notify_all(); }

	void wait(unique_lock<mutex>& _Lock) const threadsafe { CV().wait(L(_Lock)); }

	template <typename Predicate>
	void wait(unique_lock<mutex>& _Lock, Predicate _Predicate) const threadsafe
	{
		CV().wait(L(_Lock), _Predicate);
	}

	template <typename Clock, typename Duration>
	oStd::cv_status::value wait_until(unique_lock<mutex>& _Lock, const oStd::chrono::time_point<Clock, Duration>& _AbsoluteTime) const threadsafe
	{
		return CV().wait_until(L(_Lock), _AbsoluteTime);
	}

	template <typename Clock, typename Duration, typename Predicate>
	bool wait_until(unique_lock<mutex>& _Lock, const oStd::chrono::time_point<Clock, Duration>& _AbsoluteTime, Predicate _Predicate) const threadsafe
	{
		return CV().wait_until(L(_Lock), _AbsoluteTime, _Predicate);
	}

	template <typename Rep, typename Period>
	oStd::cv_status::value wait_for(unique_lock<mutex>& _Lock, const oStd::chrono::duration<Rep, Period>& _RelativeTime) const threadsafe
	{
		return CV().wait_for(L(_Lock), _RelativeTime);
	}

	template <typename Rep, typename Period, typename Predicate>
	bool wait_for(unique_lock<mutex>& _Lock, const oStd::chrono::duration<Rep, Period>& _RelativeTime, Predicate _Predicate) const threadsafe
	{
		return CV().wait_for(L(_Lock), _RelativeTime, _Predicate);
	}

	typedef oStd::condition_variable::native_handle_type native_handle_type;
	native_handle_type native_handle() threadsafe { return CV().native_handle(); }
};

} // namespace oConcurrency

#endif
