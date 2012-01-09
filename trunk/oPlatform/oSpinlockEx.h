// $(header)
#pragma once
#ifndef oSpinlockEx_h
#define oSpinlockEx_h

#include <oBasis/oStdAtomic.h>

class oSpinlockEx
{
	// @oooii-tony: This is an evolving pattern. There are cases where there are
	// user threads that can modify shared data and then there is a system 
	// consumer that usually funnels all those changes into a command queue of 
	// sorts. i.e.:
	//
	// Windows Message Pump on its own thread. Any client thread can change the
	// oWindow::DESC of the window, or the DESC of any child object, but all 
	// modifications must occur on the message pump thread. Thus this cannot use
	// oDispatchQueue because the queue is under the Windows API.
	//
	// oRenderer. Any client thread can change device children such as meshes and
	// actors. However when submit is called these modifications are locked for a
	// moment, the memory is packed up and shipped off to the render nodes, then
	// modification is reallowed. The memory footprint itself becomes the command
	// queue.
	//
	// In these and other cases it is common that we'll also want to abort client
	// threads when the manager that consumes client updates (Windows message 
	// thread or the render device) is to be destroyed. All in-flight 
	// modifications stalled on availability should enter an ABORT state 
	// indicating that the device/window/foundation object is no longer available.
	// Of course this only holds as long as this oSpinlockEx object is valid, so
	// very-latent operations like callbacks given to async file I/O operations
	// would not be addressed by this object.
	//
	// It's not clear that this class addresses meaningful needs well. For example
	// should it really be a spinlock or a mutex that allows a thread to sleep?
	// If a mutex, is it recursive? or a RWMutex that is non-recursive?

	enum STATE
	{
		AVAILABLE,
		MODIFYING,
		MODIFIED,
		APPLYING,
		LOCKOUT,
		ABORT,
	};

	oStd::atomic_int State;
	bool AllowConcurrentModifies;

public:
	oSpinlockEx(bool _AllowConcurrentModifies) : State((int)AVAILABLE), AllowConcurrentModifies(_AllowConcurrentModifies) {}
	~oSpinlockEx() { oASSERT(State == AVAILABLE || State == LOCKOUT || State == ABORT, "Destroying an oSpinLockEx that is not in a steady state (i.e. the lock is marked with pending work)"); }

	inline bool LockModify() threadsafe
	{
		while (!State.compare_exchange(AVAILABLE, MODIFYING) && (!AllowConcurrentModifies || !State.compare_exchange(MODIFIED, MODIFYING)))
			if (State == ABORT)
				return false;
		return true;
	}

	inline void UnlockModify(bool _CancelModified = false) threadsafe
	{
		oASSERT(State == MODIFYING, "");
		State.exchange(_CancelModified ? AVAILABLE : MODIFIED);
	}

	inline bool TryLockApply() threadsafe
	{
		return State.compare_exchange(MODIFIED, APPLYING);
	}

	inline bool LockApply() threadsafe
	{
		while (!State.compare_exchange(MODIFIED, APPLYING))
			if (State == ABORT)
				return false;
		return true;
	}

	inline void UnlockApply(bool _Lockout = false) threadsafe
	{
		oASSERT(State == APPLYING, "");
		State.exchange(_Lockout ? LOCKOUT : AVAILABLE);
	}

	inline void Abort() threadsafe
	{
		int oldState;
		while (1)
		{
			oldState = State;
			if (oldState == AVAILABLE || oldState == LOCKOUT || oldState == MODIFIED)
				if (State.compare_exchange(oldState, ABORT))
					break;
			
			if (oldState == ABORT)
				break;
		}
	}

	// works from either the lockout or aborted state
	inline void ResetAvailability() threadsafe
	{
		State.exchange(AVAILABLE);
	}
};

#endif
