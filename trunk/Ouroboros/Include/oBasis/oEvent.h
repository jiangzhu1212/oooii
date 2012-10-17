/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
// how the Window's event is implemented. It does seem a better pattern to use
// condition_variables directly whenever possible, but sometimes an event is 
// enough. This also serves as a simple documentation of how 
// condition_variable's work, so it's a good starting place for more complex 
// usages. Remember condition_variables unblock with a locked mutex, so if more
// than one function call or memory write is needed, it's necessary to use 
// condition_variable directly.
#pragma once
#ifndef oEvent2_h
#define oEvent2_h

#include <oBasis/oInvalid.h>
#include <oBasis/oStdConditionVariable.h>
#include <oBasis/oStdMutex.h>
#include <oBasis/oThreadsafe.h>

namespace detail {
	class event
	{
		// Here's an event built entirely on std namespace objects, but doesn't have
		// the const/threadsafe keywording.
	public:
		enum AutoReset_t { AutoReset };

		event()
			: DoAutoReset(false)
			, Set(false)
		{}

		event(AutoReset_t _AutoReset)
			: DoAutoReset(true)
			, Set(false)
		{}

		// Sets the event, thus unblocking any threads waiting on this event
		void set()
		{
			if (!Set)
			{
				if (DoAutoReset)
					CV.notify_one();
				else
				{
					{
						oStd::lock_guard<oStd::mutex> lock(M);
						Set = true;
					}
					CV.notify_all();
				}
			}
		}

		// Reset the event. If AutoReset is not specified, no wait will block after a 
		// Set() call until this is called.
		void reset() { oStd::lock_guard<oStd::mutex> lock(M); Set = false; }

		// Poll the state of the event. Use of this API is discouraged, but is 
		// sometimes useful for other similar interrogatory API.
		bool is_set() const { return Set; }

		// Sleep the calling thread until some other thread calls Set().
		void wait() { oStd::unique_lock<oStd::mutex> lock(M); while (!Set) CV.wait(lock); }

		// Sleep the calling thread until some other thread calls Set() or the timeout
		// threshold is reached.
		template <typename Clock, typename Duration>
		bool wait_until(const oStd::chrono::time_point<Clock, Duration>& _AbsoluteTime)
		{
			oStd::unique_lock<oStd::mutex> lock(M);
			while (!Set)
				if (oStd::cv_status::timeout == CV.wait_until(lock, _AbsoluteTime))
					return Set;
			return true;
		}

		// Sleep the calling thread until some other thread calls Set() or the timeout
		// threshold is reached.
		template <typename Rep, typename Period>
		bool wait_for(const oStd::chrono::duration<Rep, Period>& _RelativeTime)
		{
			oStd::unique_lock<oStd::mutex> lock(M);
			while (!Set)
				if (oStd::cv_status::timeout == CV.wait_for(lock, _RelativeTime))
					return Set;
			return true;
		}

	private:
		oStd::mutex M;
		oStd::condition_variable CV;
		bool DoAutoReset;

		// volatile largely to support better behavior of is_set(), which should not
		// be used. However we saw a case where a call to set() was done in thread A
		// and thread B was in a while (!e.is_set()) {}. This is not a good pattern,
		// but the behavior was that the look never exited. That meant that the loop 
		// was tight enough that it never saw a need to refresh the cache? To be 
		// certain, have the compiler fence this value to ensure that's what was 
		// going on.
		volatile bool Set;
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

	void Set() threadsafe { E().set(); }
	void Reset() threadsafe { E().reset(); }
	bool IsSet() const threadsafe { return E().is_set(); }
	bool Wait(unsigned int _TimeoutMS = oInfiniteWait) const threadsafe
	{
		if (_TimeoutMS == oInfiniteWait)
			E().wait();
		else
			return E().wait_for(oStd::chrono::milliseconds(_TimeoutMS));
		return true;
	}

private:
	detail::event e;
	detail::event& E() const threadsafe { return thread_cast<detail::event&>(e); }
};

#endif
