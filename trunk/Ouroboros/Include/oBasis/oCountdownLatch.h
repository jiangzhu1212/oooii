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
// Synchronization object often described as a reverse semaphore. This object
// gets initialized with a count and gives the system API to decrement the 
// count. When the count reaches 0, this object becomes unblocked. This object
// must be manually reset to a new count in order to be reused.
#pragma once
#ifndef oCountdownLatch_h
#define oCountdownLatch_h

#include <oBasis/oAssert.h>
#include <oBasis/oConditionVariable.h>
#include <oBasis/oMutex.h>
#include <oBasis/oNonCopyable.h>
#include <oBasis/oStdChrono.h>
#include <oBasis/oThreadsafe.h>

class oCountdownLatch : oNoncopyable
{
	oConditionVariable ZeroReferences;
	oMutex Mutex;
	int NumOutstanding;
	const char* DebugName;

public:

	// In either construction or Reset(), setting _InitialCount to a negative 
	// number will immediately set the underlying event. Setting _InitialCount
	// to zero or above will reset the event and allow functionality to proceed,
	// so setting the event to zero will require a reference then a release to 
	// really make sense. _DebugName must be a constant string.
	oCountdownLatch(const char* _DebugName, int _InitialCount)
		: NumOutstanding(_InitialCount)
		, DebugName(_DebugName)
	{
		// to centralize code, have all operations on NumOutStanding go through 
		// release, so add one first so this all ends up being a noop
		NumOutstanding++;
		Release();
	}

	// This should only be used for a user-facing update/UI or debug spew. This
	// should not be used in control logic, use Wait() instead.
	inline int GetNumOutstanding() const threadsafe { return NumOutstanding; }

	// Resets the countdown latch to the specified initial count. This is exposed
	// so a countdown latch can be reused after its Wait() has unblocked.
	inline void Reset(int _InitialCount) threadsafe
	{
		Mutex.lock();
		NumOutstanding = _InitialCount + 1;
		Mutex.unlock();
		Release();
	}
	
	// For systems that cannot know an initial count, this will make the latch 
	// require one more Release() call before unblocking. This is not a preferred
	// practice because there is an ABA race condition where between the decision 
	// to go from 1 to 0 and thus trigger the event and actually triggering the 
	// event, another Reference may sneak in. This means the system isn't well-
	// behaved and should be programmed to not accept any new References (new work)
	// while a thread is waiting an any queued tasks to flush. One way to do this
	// is to keep a scheduling reference separate from any of the work references
	// and only release the scheduling reference at a time known not to allow 
	// additional work references to be added, such as a frame-to-frame 
	// application.
	inline void Reference() threadsafe
	{
		oLockGuard<oMutex> Lock(Mutex);
		oASSERT(NumOutstanding > 0, "oCountdownLatch::Reference() called on an invalid refcount, meaning this reference is too late to keep any waiting threads blocked. This is a race condition, but really it is a poorly-behaved system because references are being added after the countdown had finished. If possible, use Reset or the ctor to set an initial count beforehand and don't use Reference() at all (that's to what a semaphore limits the options), or ensure that a separate reference is kept by the system to protect against the event firing and only release that reference when it is known that new reference will not be made.");
		NumOutstanding++;
	}
	
	// Call this to reduce the number of references keeping the latch locked
	inline void Release() threadsafe
	{
		oLockGuard<oMutex> Lock(Mutex);
		if (--NumOutstanding <= 0)
			ZeroReferences.notify_all();
	}

	// Block the calling thread indefinitely for the number of outstanding items 
	// reaches zero.
	inline void Wait() threadsafe
	{
		oUniqueLock<oMutex> Lock(Mutex);
		while (NumOutstanding > 0) // Guarded Suspension
			ZeroReferences.wait(Lock);
	}

	// Block the calling thread for a time or until the number of outstanding items 
	// reaches zero. Returns false if this times out, true otherwise.
	inline bool Wait(unsigned int _TimeoutMS) threadsafe
	{
		oStd::cv_status::value status = oStd::cv_status::no_timeout;
		oUniqueLock<oMutex> Lock(Mutex);
		while (oStd::cv_status::no_timeout == status && NumOutstanding > 0)
			status = ZeroReferences.wait_for(Lock, oStd::chrono::milliseconds(_TimeoutMS));
		return oStd::cv_status::no_timeout == status;
	}
};

#endif
