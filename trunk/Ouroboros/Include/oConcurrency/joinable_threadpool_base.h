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
// This is a building block used as a helper to implement a thread pool based on 
// an underlying implementation that does not directly support flush and join.
// Why don't platforms care to know when a scheduler is empty? Without that 
// information unit testing and exiting is difficult.
#pragma once
#ifndef oConcurrency_joinable_threadpool_base_h
#define oConcurrency_joinable_threadpool_base_h

#include <oConcurrency/oConcurrency.h>
#include <oStd/mutex.h>

namespace oConcurrency {

class joinable_threadpool_base
{
	joinable_threadpool_base(const joinable_threadpool_base&); /* = delete */
	const joinable_threadpool_base& operator=(const joinable_threadpool_base&); /* = delete */

	joinable_threadpool_base(joinable_threadpool_base&&); /* = delete */
	joinable_threadpool_base& operator=(joinable_threadpool_base&&); /* = delete */

public:
	// Returns whether this is joinable_threadpool_base or not.
	bool joinable() const { return State != JOINED; }

protected:
	joinable_threadpool_base() : State(JOINABLE) {}

	// Lock around the underlying dispatch/execute/kickoff call, and only issue
	// if begin_dispatch doesn't throw an exception. Remember a join will 
	// invalidate the thread pool like a std::thread's join.
	void begin_dispatch()
	{
		if (State != JOINABLE || !Mutex.try_lock_shared())
			throw std::out_of_range("threadpool call after join");
	}

	// Call this after calling the underlying dispatch call.
	void end_dispatch() { Mutex.unlock_shared(); }

	// Call this as the first line of the flush implementation to ensure new 
	// dispatches are locked out.
	void begin_flush() { Mutex.lock(); State = FLUSHING; }

	// Call this once the schedule is empty to resume a valid dispatch state.
	void end_flush() { Mutex.unlock(); State = JOINABLE; }

	// Call this as the first line of the join implementation to ensure new 
	// dispatches are locked out.
	void join() { std::lock_guard<ouro::shared_mutex> lock(Mutex); State = JOINED; }

private:
	ouro::shared_mutex Mutex;
	enum STATE { JOINABLE, FLUSHING, JOINED, };
	STATE State;
};

} // namespace oConcurrency

#endif
