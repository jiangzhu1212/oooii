/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
// Thin wrapper for Std::oConditionVariable that provides threadsafe 
// specification to reduce thread_cast throughout the code. Prefer using 
// these wrappers over using std::oConditionVariable directly.
#pragma once
#ifndef oConditionVariable_h
#define oConditionVariable_h

#include <oBasis/oStdConditionVariable.h>
#include <oBasis/oMutex.h>
#include <oBasis/oThreadsafe.h>

class oConditionVariable
{
	oStd::condition_variable ConditionVariable;
	oStd::condition_variable& CV() const threadsafe { return thread_cast<oStd::condition_variable&>(ConditionVariable); }
	oStd::unique_lock<oStd::mutex>& L(oUniqueLock<oMutex>& _Lock) const threadsafe
	{
		return *(oStd::unique_lock<oStd::mutex>*)&_Lock;
	}

	oConditionVariable(oConditionVariable const&); /* = delete */
	oConditionVariable& operator=(oConditionVariable const&); /* = delete */

public:
	oConditionVariable() {}
	~oConditionVariable() {}

	void notify_one() threadsafe { CV().notify_one(); }
	void notify_all() threadsafe { CV().notify_all(); }

	void wait(oUniqueLock<oMutex>& _Lock) const threadsafe { CV().wait(L(_Lock)); }

	template <typename Predicate>
	void wait(oUniqueLock<oMutex>& _Lock, Predicate _Predicate) const threadsafe
	{
		CV().wait(L(_Lock), _Predicate);
	}

	template <typename Clock, typename Duration>
	oStd::cv_status::value wait_until(oUniqueLock<oMutex>& _Lock, const oStd::chrono::time_point<Clock, Duration>& _AbsoluteTime) const threadsafe
	{
		return CV().wait_until(L(_Lock), _AbsoluteTime);
	}

	template <typename Clock, typename Duration, typename Predicate>
	bool wait_until(oUniqueLock<oMutex>& _Lock, const oStd::chrono::time_point<Clock, Duration>& _AbsoluteTime, Predicate _Predicate) const threadsafe
	{
		return CV().wait_until(L(_Lock), _AbsoluteTime, _Predicate);
	}

	template <typename Rep, typename Period>
	oStd::cv_status::value wait_for(oUniqueLock<oMutex>& _Lock, const oStd::chrono::duration<Rep, Period>& _RelativeTime) const threadsafe
	{
		return CV().wait_for(L(_Lock), _RelativeTime);
	}

	template <typename Rep, typename Period, typename Predicate>
	bool wait_for(oUniqueLock<oMutex>& _Lock, const oStd::chrono::duration<Rep, Period>& _RelativeTime, Predicate _Predicate) const threadsafe
	{
		return CV().wait_for(L(_Lock), _RelativeTime, _Predicate);
	}

	typedef oStd::condition_variable::native_handle_type native_handle_type;
	native_handle_type native_handle() threadsafe { return CV().native_handle(); }
};

#endif
