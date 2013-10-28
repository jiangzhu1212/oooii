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
// A wrapper for oBase::event that adds only threadsafe keywording. For primary
// documentation, see oBase/event.h.
#pragma once
#ifndef oConcurrency_event_h
#define oConcurrency_event_h

#include <oBase/event.h>
#include <oConcurrency/thread_safe.h>

namespace oConcurrency {

enum autoreset_t { autoreset };

class event
{
public:
	event() {}
	event(autoreset_t _AutoReset) : e(ouro::autoreset) {}
	void set(int _Mask = 1) threadsafe { E().set(_Mask); }
	void reset(int _Mask = ~0) threadsafe { E().reset(_Mask); }
	void wait(int _Mask = 1) const threadsafe { E().wait(_Mask); }
	int wait_any(int _Mask = 1) const threadsafe { return E().wait_any(_Mask); }
	
	template<typename Clock, typename Duration>
	bool wait_until(const oStd::chrono::time_point<Clock, Duration>& _AbsoluteTime, int _Mask = 1) const threadsafe { return E().wait_until(_AbsoluteTime, _Mask); }

	template<typename Clock, typename Duration>
	int wait_until_any(const oStd::chrono::time_point<Clock, Duration>& _AbsoluteTime, int _Mask = 1) const threadsafe { return E().wait_until_any(_AbsoluteTime, _Mask); }

	template<typename Rep, typename Period>
	bool wait_for(const oStd::chrono::duration<Rep, Period>& _RelativeTime, int _Mask = 1) const threadsafe { return E().wait_for(_RelativeTime, _Mask); }

	template<typename Rep, typename Period>
	int wait_for_any(const oStd::chrono::duration<Rep, Period>& _RelativeTime, int _Mask = 1) const threadsafe { return E().wait_for_any(_RelativeTime, _Mask); }

	bool is_set(int _Mask = 1) const threadsafe { return E().is_set(_Mask); }
	bool is_any_set(int _Mask = 1) const threadsafe { return E().is_any_set(_Mask); }

private:
	ouro::event e;
	ouro::event& E() const threadsafe { return thread_cast<ouro::event&>(e); }
};

} // namespace oConcurrency

#endif
