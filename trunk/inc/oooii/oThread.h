// $(header)
#pragma once
#ifndef oThread_h
#define oThread_h

#include <oooii/oInterface.h>

interface oThread : oInterface
{
	enum PRIORITY
	{
		PRIORITY_LOWEST,
		PRIORITY_LOW,
		PRIORITY_NORMAL,
		PRIORITY_HIGH,
		PRIORITY_HIGHEST,
	};

	interface Proc : oInterface
	{
		// Runs one iteration of the thread's proc. Looping until exit is handled
		// by the thread itself.
		virtual void RunIteration() = 0;

		// Called once before going into Run()
		// If this returns false, the Run() loop is skipped and OnEnd() is called
		virtual bool OnBegin() = 0;

		// Called on thread exit
		virtual void OnEnd() = 0;
	};

	static bool Create(const char* _DebugName, size_t _StackSize, bool _StartSuspended, Proc* _Proc, threadsafe oThread** _ppThread);

	// Returns the current oThread. If the current thread was not created with
	// oThread::Create(), this returns NULL.
	static threadsafe oThread* Current();

	virtual void* GetNativeHandle() threadsafe = 0;
	virtual size_t GetID() const threadsafe = 0;
	virtual bool IsRunning() const threadsafe = 0;
	virtual bool IsSuspended() const threadsafe = 0;
	virtual const char* GetDebugName() const threadsafe = 0;

	virtual void SetAffinity(size_t _AffinityMask) threadsafe = 0;
	virtual size_t GetAffinity() const = 0;
	virtual void SetPriority(PRIORITY _Priority) threadsafe = 0;
	virtual PRIORITY GetPriority() threadsafe const = 0;
	virtual void Suspend() threadsafe = 0;
	virtual void Resume() threadsafe = 0;
	virtual void Exit() threadsafe = 0;

	// Returns true if wait was successful, false if wait timed out
	virtual bool Wait(unsigned int _TimeoutMS = oINFINITE_WAIT) threadsafe = 0;

	#if defined(_WIN32) || defined(_WIN64)
		static inline oThread::PRIORITY ConvertPriority(int _THREAD_PRIORITY) { return static_cast<oThread::PRIORITY>(_THREAD_PRIORITY+2); }
		static inline int ConvertPriority(oThread::PRIORITY _ThreadPRIORITY) { return static_cast<int>(_ThreadPRIORITY)-2; }
	#endif
};

#endif
