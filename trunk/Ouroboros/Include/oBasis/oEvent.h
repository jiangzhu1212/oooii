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
// cases, use condition_variables directly, using this implementation as a 
// reference but quite often an event is enough. NOTE: For a trivial event, just
// use default parameters.
#pragma once
#ifndef oEvent_h
#define oEvent_h

#include <oBasis/oInvalid.h>
#include <oBasis/oStdConditionVariable.h>
#include <oBasis/oStdMutex.h>
#include <oBasis/oThreadsafe.h>

namespace detail {
	class event
	{
		// detail::event is built entirely on std namespace objects, but doesn't 
		// have the const/threadsafe keywording oEvent has below.
	public:
		enum AutoReset_t { AutoReset };

		event()
			: DoAutoReset(false)
			, Set(0)
		{}

		event(AutoReset_t _AutoReset)
			: DoAutoReset(true)
			, Set(0)
		{}

		// Poll the state of the event mask and returns true if the masks match 
		// exactly. Use of this API is discouraged, but is sometimes useful for 
		// debug interrogatory API.
		bool is_set(int _Mask = 1) const { return (Set & _Mask) == _Mask; }
		
		// Like is_set, but will return true if any bit in the mask is set
		bool is_any_set(int _Mask = 1) const { return !!(Set & _Mask); }

		// Sets the event mask, thus unblocking any threads waiting on this event.
		void set(int _Mask = 1)
		{
			if (!is_set(_Mask))
			{
				if (DoAutoReset)
					CV.notify_one();
				else
				{
					{
						oStd::lock_guard<oStd::mutex> lock(M);
						Set |= _Mask;
					}
					CV.notify_all();
				}
			}
		}

		// Reset all set events in the specified mask. If AutoReset is not 
		// specified, no wait will block after a Set() call until this is called.
		void reset(int _Mask = ~0) { oStd::lock_guard<oStd::mutex> lock(M); Set &=~ _Mask; }

		// Sleep the calling thread until all bits in the specified mask are Set().
		void wait(int _Mask = 1) { oStd::unique_lock<oStd::mutex> lock(M); while (!is_set(_Mask)) CV.wait(lock); }

		// Sleep the calling thread until any bit in the specified mask is Set(). 
		// This will return the Set mask as it is when the wait is unblocked.
		int wait_any(int _Mask = 1) { oStd::unique_lock<oStd::mutex> lock(M); while (!is_any_set(_Mask)) CV.wait(lock); return Set; }

		// Sleep the calling thread until the Set mask matches the one specified, or
		// absolute the timeout threshold is reached.
		template <typename Clock, typename Duration>
		bool wait_until(const oStd::chrono::time_point<Clock, Duration>& _AbsoluteTime, int _Mask = 1)
		{
			oStd::unique_lock<oStd::mutex> lock(M);
			while (!is_set(_Mask))
				if (oStd::cv_status::timeout == CV.wait_until(lock, _AbsoluteTime))
					return false;
			return true;
		}

		// Sleep the calling thread until any bit in the Set mask is set, or the 
		// absolute timeout threshold is reached. This returns the mask as it is 
		// when the wait is unblocked.
		template <typename Clock, typename Duration>
		int wait_any_until(const oStd::chrono::time_point<Clock, Duration>& _AbsoluteTime, int _Mask = 1)
		{
			oStd::unique_lock<oStd::mutex> lock(M);
			while (!is_any_set(_Mask))
				if (oStd::cv_status::timeout == CV.wait_until(lock, _AbsoluteTime))
					return 0;
			return Set;
		}

		// Sleep the calling thread until the Set mask matches the one specified, or
		// the relative timeout threshold is reached.
		template <typename Rep, typename Period>
		bool wait_for(const oStd::chrono::duration<Rep, Period>& _RelativeTime, int _Mask = 1)
		{
			oStd::unique_lock<oStd::mutex> lock(M);
			while (!is_set(_Mask))
				if (oStd::cv_status::timeout == CV.wait_for(lock, _RelativeTime))
					return false;
			return true;
		}

		// Sleep the calling thread until the Set mask matches the one specified, or
		// the relative timeout threshold is reached. This returns the mask as it is
		// when the wait is unblocked.
		template <typename Rep, typename Period>
		int wait_any_for(const oStd::chrono::duration<Rep, Period>& _RelativeTime, int _Mask = 1)
		{
			oStd::unique_lock<oStd::mutex> lock(M);
			while (!is_any_set(_Mask))
				if (oStd::cv_status::timeout == CV.wait_for(lock, _RelativeTime))
					return 0;
			return Set;
		}

	private:
		oStd::mutex M;
		oStd::condition_variable CV;
		bool DoAutoReset;
		int Set;
	};
} // namespace detail

class oEvent
{
	// API docs/comments are in detail::event

public:
	enum AutoReset_t { AutoReset };

	oEvent() {}
	oEvent(AutoReset_t _AutoReset)
		: e(detail::event::AutoReset)
	{}

	void Set(int _Mask = 1) threadsafe { E().set(_Mask); }
	void Reset(int _Mask = ~0) threadsafe { E().reset(_Mask); }
	bool IsSet(int _Mask = 1) const threadsafe { return E().is_set(_Mask); }
	bool IsAnySet(int _Mask = 1) const threadsafe { return E().is_any_set(_Mask); }
	bool Wait(unsigned int _TimeoutMS = oInfiniteWait, int _Mask = 1) const threadsafe
	{
		if (_TimeoutMS == oInfiniteWait)
			E().wait(_Mask);
		else
			return E().wait_for(oStd::chrono::milliseconds(_TimeoutMS), _Mask);
		return true;
	}

	int WaitAny(unsigned int _TimeoutMS = oInfiniteWait, int _Mask = 1) const threadsafe
	{
		if (_TimeoutMS == oInfiniteWait)
			return E().wait_any(_Mask);
		return E().wait_for(oStd::chrono::milliseconds(_TimeoutMS), _Mask);
	}

private:
	detail::event e;
	detail::event& E() const threadsafe { return thread_cast<detail::event&>(e); }
};

#endif
