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
#pragma once
#ifndef oBarrier_h
#define oBarrier_h

#include <oooii/oEvent.h>

class oBarrier : oNoncopyable
{
	// A barrier is like a reverse semaphore: on creation refcount is 0 and 
	// state is waiting (blocking) any reference increases the count, which puts 
	// the barrier into a wait state when the outstanding references are reduced 
	// to 0, the state reenters the wait state (blocking). On reenter, all calls
	// to Reference() block until Reset() is called. This way the calling code 
	// can issue a bunch of work and wait on this barrier for the flush of the 
	// work. Then while flushing, all references that try to stop the flush will
	// block. Calling reset flags that the flush is finished and the system is 
	// ready to process more work.
public:
	oBarrier(const char* _DebugName = 0) : Event(_DebugName), r(0) {}
	inline void* GetNativeHandle() threadsafe { return Event.GetNativeHandle(); }
	inline const char* GetDebugName() const threadsafe { return Event.GetDebugName(); }
	inline unsigned int GetNumOutstanding() const threadsafe { return r; }
	unsigned int Reference() threadsafe;
	unsigned int Release() threadsafe;

	inline bool Wait(unsigned int _TimeoutMS = oINFINITE_WAIT) threadsafe { return Event.Wait(_TimeoutMS); }
	static size_t WaitMultiple(threadsafe oBarrier** _ppBarriers, size_t _NumBarriers, bool _WaitAll = true, unsigned int _TimeoutMS = oINFINITE_WAIT);

protected:
	threadsafe oEvent Event;
	int r;
};

#endif
