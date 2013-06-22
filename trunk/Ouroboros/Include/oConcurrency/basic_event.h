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
// A cross-platform event build on std::condition_variable, which is basically
// how the Window's event is implemented. This supports the "WaitMultiple" 
// concept by storing a 32-bit mask of bools, rather than just one bool like
// the Windows event API suggests. Event flags can be defined then tested as 
// appropriate. The wait_any() APIs are more like WaitSingle. In more complex
// cases use condition_variables directly. Using this implementation as a 
// reference but quite often an event is enough. NOTE: For a trivial bool-like
// event, just use default parameters.
#pragma once
#ifndef oConcurrency_basic_event_h
#define oConcurrency_basic_event_h

#include <oStd/oStdConditionVariable.h>
#include <oStd/oStdMutex.h>
#include <oStd/oStdThread.h>

namespace oConcurrency {

namespace autoreset_t { enum value { autoreset }; }

class basic_event
{
public:
	// Starts in the reset state. A call to set() sets the state and reset() 
	// resets the state explicitly. If the set() (and thus wake) and then reset()
	// needs to be atomic, use the autoreset constructor.
	basic_event();

	// When set() is called, this will unblock all threads waiting on this event 
	// but then atomically leaves the event in the reset state.
	basic_event(autoreset_t::value _AutoReset);

	// Sets the event mask by or'ing it with the existing mask, thus unblocking 
	// any threads waiting on this event.
	void set(int _Mask = 1);

	// Reset all set events in the specified mask. If autoreset is not specified 
	// no wait will block after a set() call until this is called.
	void reset(int _Mask = ~0);

	// Sleep the calling thread until all bits in the specified mask are Set().
	void wait(int _Mask = 1);

	// Sleep the calling thread until any bit in the specified mask is Set(). 
	// This will return the Set mask as it is when the wait is unblocked.
	int wait_any(int _Mask = 1);

	// Sleep the calling thread until the Set mask matches the one specified, or
	// absolute the timeout threshold is reached.
	template <typename Clock, typename Duration>
	bool wait_until(const oStd::chrono::time_point<Clock, Duration>& _AbsoluteTime, int _Mask = 1);

	// Sleep the calling thread until any bit in the Set mask is set, or the 
	// absolute timeout threshold is reached. This returns the mask as it is 
	// when the wait is unblocked.
	template <typename Clock, typename Duration>
	int wait_until_any(const oStd::chrono::time_point<Clock, Duration>& _AbsoluteTime, int _Mask = 1);

	// Sleep the calling thread until the Set mask matches the one specified, or
	// the relative timeout threshold is reached.
	template <typename Rep, typename Period>
	bool wait_for(const oStd::chrono::duration<Rep, Period>& _RelativeTime, int _Mask = 1);

	// Sleep the calling thread until the Set mask matches the one specified, or
	// the relative timeout threshold is reached. This returns the mask as it is
	// when the wait is unblocked.
	template <typename Rep, typename Period>
	int wait_for_any(const oStd::chrono::duration<Rep, Period>& _RelativeTime, int _Mask = 1);

	// Poll the state of the event mask and returns true if the masks match 
	// exactly. Use of this API is discouraged, but is sometimes useful for 
	// debug interrogatory API.
	bool is_set(int _Mask = 1) const;

	// Like is_set, but will return true if any bit in the mask is set
	bool is_any_set(int _Mask = 1) const;

private:
	oStd::mutex M;
	oStd::condition_variable CV;
	int Set;
	bool DoAutoReset;
};

inline basic_event::basic_event()
	: Set(0)
	, DoAutoReset(false)
{}

inline basic_event::basic_event(autoreset_t::value _AutoReset)
	: Set(0)
	, DoAutoReset(true)
{}

inline void basic_event::set(int _Mask)
{
	if (!is_set(_Mask))
	{
		if (DoAutoReset)
			CV.notify_one();
		else
		{
			M.lock();
			Set |= _Mask;
			M.unlock();
			CV.notify_all();
		}
	}
}

inline void basic_event::reset(int _Mask)
{
	oStd::lock_guard<oStd::mutex> lock(M);
	Set &=~ _Mask;
}

inline void basic_event::wait(int _Mask)
{
	oStd::unique_lock<oStd::mutex> lock(M);
	while (!is_set(_Mask))
		CV.wait(lock);
}

inline int basic_event::wait_any(int _Mask)
{
	oStd::unique_lock<oStd::mutex> lock(M);
	while (!is_any_set(_Mask))
		CV.wait(lock);
	return Set;
}

template <typename Clock, typename Duration>
inline bool basic_event::wait_until(
	const oStd::chrono::time_point<Clock, Duration>& _AbsoluteTime, int _Mask)
{
	oStd::unique_lock<oStd::mutex> lock(M);
	while (!is_set(_Mask))
		if (oStd::cv_status::timeout == CV.wait_until(lock, _AbsoluteTime))
			return false;
	return true;
}

template <typename Clock, typename Duration>
inline int basic_event::wait_until_any(
	const oStd::chrono::time_point<Clock, Duration>& _AbsoluteTime, int _Mask)
{
	oStd::unique_lock<oStd::mutex> lock(M);
	while (!is_any_set(_Mask))
		if (oStd::cv_status::timeout == CV.wait_until(lock, _AbsoluteTime))
			return 0;
	return Set;
}

template <typename Rep, typename Period>
inline bool basic_event::wait_for(const oStd::chrono::duration<Rep, Period>& _RelativeTime, int _Mask)
{
	oStd::unique_lock<oStd::mutex> lock(M);
	while (!is_set(_Mask))
		if (oStd::cv_status::timeout == CV.wait_for(lock, _RelativeTime))
			return false;
	return true;
}

template <typename Rep, typename Period>
inline int basic_event::wait_for_any(const oStd::chrono::duration<Rep, Period>& _RelativeTime, int _Mask)
{
	oStd::unique_lock<oStd::mutex> lock(M);
	while (!is_any_set(_Mask))
		if (oStd::cv_status::timeout == CV.wait_for(lock, _RelativeTime))
			return 0;
	return Set;
}

inline bool basic_event::is_set(int _Mask) const
{
	return (Set & _Mask) == _Mask;
}

inline bool basic_event::is_any_set(int _Mask) const
{
	return !!(Set & _Mask);
}

} // namespace oConcurrency

#endif
