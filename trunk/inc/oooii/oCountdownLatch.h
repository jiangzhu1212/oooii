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
// Synchronization object often described as a reverse semaphore. This object
// gets initialized with a count and gives the system API to decrement the 
// count. When the count reaches 0, this object becomes unblocked. This object
// must be manually reset to a new count in order to be reused. The Reset()
// function is not threadsafe because it could cause an ABA race condition where
// the logic determines it is time to set an event (if statement) but has not
// yet set the event when a reset could come in and thus the wait would unblock,
// yet there was more work to do as described by the Reset() call.
#pragma once
#ifndef oCountdownLatch_h
#define oCountdownLatch_h

#include <oooii/oEvent.h>
#include <oooii/oRefCount.h>

class oCountdownLatch : oNoncopyable
{
public:

	// In either construction or Reset(), setting _InitialCount to a negative 
	// number will immediately set the underlying event. Setting _InitialCount
	// to zero or above will reset the event and allow functionality to proceed,
	// so setting the event to zero will require a reference then a release to 
	// really make sense.

	oCountdownLatch(const char* _DebugName, int _InitialCount);
	void* GetNativeHandle() threadsafe;
	const char* GetDebugName() const threadsafe;
	int GetNumOutstanding() threadsafe;
	void Reset(int _InitialCount);
	
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
	void Reference() threadsafe;
	
	void Release() threadsafe;
	
	bool Wait(unsigned int _TimeoutMS = oINFINITE_WAIT) threadsafe;
	static bool WaitMultiple(threadsafe oCountdownLatch** _ppCountdownLatches, size_t _NumCountdownLatches, size_t* _pWaitBreakingCountdownLatchIndex, unsigned int _TimeoutMS = oINFINITE_WAIT);

protected:
	threadsafe oEvent Event;
	oRefCount RefCount;
};

#endif
