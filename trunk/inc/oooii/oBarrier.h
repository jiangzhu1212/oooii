// $(header)

// A barrier is like a reverse semaphore: on creation refcount is 0 and 
// state is waiting (blocking) any reference increases the count, which puts 
// the barrier into a wait state when the outstanding references are reduced 
// to 0, the state reenters the wait state (blocking). On reenter, all calls
// to Reference() block until Reset() is called. This way the calling code 
// can issue a bunch of work and wait on this barrier for the flush of the 
// work. Then while flushing, all references that try to stop the flush will
// block. Calling reset flags that the flush is finished and the system is 
// ready to process more work.
#pragma once
#ifndef oBarrier_h
#define oBarrier_h

#include <oooii/oEvent.h>

class oBarrier : oNoncopyable
{
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
