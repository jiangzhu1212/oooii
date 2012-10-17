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
// This is a building block used as a helper to implement 
// oDispatchQueueConcurrent implementations with platform APIs that don't 
// support the idea of Flush/Joining. Why don't platforms care to know when a
// scheduler is empty?
#pragma once
#ifndef oJoinable_h
#define oJoinable_h

#include <oBasis/oNonCopyable.h>
#include <oBasis/oMutex.h>

class oJoinable : oNoncopyable
{
public:
	oJoinable()
		: State(JOINABLE)
	{}

	// Lock around the underlying dispatch/execute/kickoff call, and only issue
	// if BeginDispatch returns true. This will have the behavior of ignoring new 
	// requests during a join, so some requests may be dropped. Remember a join
	// will invalidate the system, like a std::thread's join, so the idea is that
	// something big is happening and use the scheduler is no longer applicable.
	bool BeginDispatch() threadsafe
	{
		return State == JOINABLE && Mutex.try_lock_shared();
	}

	// If the BeginDispathc succeeds, call this after calling the underlying 
	// dispatch/execute/kickoff call.
	void EndDispatch() threadsafe
	{
		Mutex.unlock_shared();
	}

	// Call this as the first line of the Flush implementation to ensure new 
	// dispatches are locked out.
	void BeginFlush() threadsafe
	{
		Mutex.lock();
		State = FLUSHING;
	}

	// Call this once the schedule is empty to resume a valid dispatch state.
	void EndFlush() threadsafe
	{
		Mutex.unlock();
		State = JOINABLE;
	}

	// Call this as the first line of the join implementation to ensure new 
	// dispatches are locked out.
	void Join() threadsafe
	{
		oLockGuard<oSharedMutex> lock(Mutex);
		State = JOINED;
	}

	// Returns whether this is joinable or not.
	bool Joinable() const threadsafe
	{
		return State != JOINED;
	}

private:
	oSharedMutex Mutex;

	enum STATE
	{
		JOINABLE,
		FLUSHING,
		JOINED,
	};

	STATE State;
};

#endif
