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
#include <oBasis/oDispatchQueuePrivate.h>
#include <oBasis/oAssert.h>
#include <oBasis/oBackoff.h>
#include <oBasis/oConditionVariable.h>
#include <oBasis/oFixedString.h>
#include <oBasis/oInitOnce.h>
#include <oBasis/oNonCopyable.h>
#include <oBasis/oMacros.h>
#include <oBasis/oMutex.h>
#include <oBasis/oRefCount.h>
#include <oBasis/oStdAtomic.h>
#include <oBasis/oTask.h>
#include <oBasis/oThread.h>
#include <oBasis/oThreadsafe.h>
#include <exception> // std::terminate
#include <list>

struct oDispatchQueuePrivate_Impl : oDispatchQueuePrivate
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE2(oDispatchQueue, oDispatchQueuePrivate_Impl);

	oDispatchQueuePrivate_Impl(const char* _DebugName, size_t _InitialTaskCapacity, bool* _pSuccess);
	~oDispatchQueuePrivate_Impl();

	bool Dispatch(oTASK _Task) threadsafe override;
	void Flush() threadsafe override;
	bool Joinable() const threadsafe override { return ExecutionThread.joinable(); }
	void Join() threadsafe override;
	const char* GetDebugName() const threadsafe override { return DebugName->c_str(); }

protected:
	// A concurrent queue has its own atomics for queue state and for allocation
	// of nodes in the queue, so that's two syncs already. We'll need a 3rd, which
	// is the wrong direction for atomicity: we want one.
	typedef std::list<oTASK > tasks_t;
	tasks_t Tasks;

	// Tasks execute on this thread
	oThread ExecutionThread;

	// This protects pushes and pops AND
	// released when the execution thread sleeps
	oMutex	QueueMutex;

	// The mechanism used to sleep the thread if there is no work
	oConditionVariable WorkAvailable;

	// True in the normal state, this is set to false during the dtor to tear
	// down the execution thread
	oStd::atomic_bool Running;

	// If true, allow new tasks to be enqueued. Sometimes there is logic that
	// must be executed through the queue before the calling thread can progress,
	// such as creation of a resource that must occur on the other thread. This
	// is exposed as the parameter to Flush where the user can choose to either
	// temporarily get to a known point in execution and then continue, or prevent
	// any future work at all from every entering the queue, such as approaching
	// shutdown.
	oStd::atomic_bool AllowEnqueues;

	// The simplest solution to guarantee the execution thread is up and running
	// would be to wait for Running to turn true in the ctor, but because this
	// object could be used in static init, it is possible to deadlock on thread
	// creation with the initialization of CRT, so we need to ensure all API
	// waits until the execution thread is ready, and this bool indicates 
	// readiness.
	oStd::atomic_bool Initialized;

	// If there are several of these floating around, it helps to have their name
	// available.
	oInitOnce<oStringS> DebugName;

	oRefCount RefCount;

	tasks_t& ProtectedTasks() threadsafe { return thread_cast<tasks_t&>(Tasks); }
	void WorkerThreadProc();
	void WaitExecutionThreadInitialized() threadsafe;
	void WakeExecutionThread() threadsafe { WorkAvailable.notify_all(); }
	void WaitForExecutionThreadToSleep() threadsafe;
};

const oGUID& oGetGUID(threadsafe const oDispatchQueuePrivate_Impl* threadsafe const*)
{
	// {FF7615D2-C7C4-486A-927A-343EBCEA7363}
	static const oGUID IIDDispatchQueuePrivate = { 0xff7615d2, 0xc7c4, 0x486a, { 0x92, 0x7a, 0x34, 0x3e, 0xbc, 0xea, 0x73, 0x63 } };
	return IIDDispatchQueuePrivate;
}

// _DebugName must be a constant string
oDispatchQueuePrivate_Impl::oDispatchQueuePrivate_Impl(const char* _DebugName, size_t _InitialTaskCapacity, bool* _pSuccess)
	: Tasks() // @oooii-tony: TODO _TaskCapacity cannot be done trivially with std::list/queue/deque... a new allocator needs to be made... so do that later.
	, AllowEnqueues(true)
	, Running(false)
	, Initialized(false)
	, DebugName(_DebugName)
{
	ExecutionThread = oThread(&oDispatchQueuePrivate_Impl::WorkerThreadProc, this);
	
	// Don't wait here because this could be called from static init and since 
	// oStd::thread uses CRT-compatible _beginthreadex, it blocks waiting for
	// CRT and static init to be done, causing a deadlock. Instead, allow 
	// execution to continue and block only on first usage.
	// WaitExecutionThreadInitialized();

	*_pSuccess = true;
}

oDispatchQueuePrivate_Impl::~oDispatchQueuePrivate_Impl()
{
	WaitExecutionThreadInitialized();
	if (Joinable())
	{
		oASSERT(false, "Calling std::terminate because a Joinable oDispatchQueuePrivate was destroyed");
		std::terminate();
	}
}

bool oDispatchQueueCreatePrivate(const char* _DebugName, size_t _InitialTaskCapacity, threadsafe oDispatchQueuePrivate** _ppDispatchQueue)
{
	bool success = false;
	oCONSTRUCT(_ppDispatchQueue, oDispatchQueuePrivate_Impl(_DebugName, _InitialTaskCapacity, &success));
	return !!*_ppDispatchQueue;
}

void oDispatchQueuePrivate_Impl::WaitExecutionThreadInitialized() threadsafe
{
	oBackoff bo;
	while (!Initialized)
		bo.Pause();
}

void oDispatchQueuePrivate_Impl::WaitForExecutionThreadToSleep() threadsafe
{
	WaitExecutionThreadInitialized();
	while (!ProtectedTasks().empty()) // concurrency is protected by the Lock below, not this test, so the cast is ok
		oLockGuard<oMutex> Lock(QueueMutex);
}

void oDispatchQueuePrivate_Impl::WorkerThreadProc()
{
	oTASK Task;
	bool PopSucceeded = false;
	oTaskRegisterThisThread(*DebugName);
	Running.exchange(true);
	Initialized.exchange(true);

	oUniqueLock<oMutex> Lock(QueueMutex);

	while (Running)
	{
		while (Running && Tasks.empty())
			WorkAvailable.wait(Lock);

		if (Running)
		{
			Lock.unlock();
			Tasks.front()();
			Lock.lock();
		}

		if (!Tasks.empty())
			Tasks.pop_front();
	}

	oEndThread();
}

bool oDispatchQueuePrivate_Impl::Dispatch(oTASK _Task) threadsafe
{
	oASSERT(_Task, "An invalid task is being dispatched");
	bool Scheduled = false;
	WaitExecutionThreadInitialized();
	{
		oLockGuard<oMutex> Lock(QueueMutex);
		if (AllowEnqueues)
		{
			ProtectedTasks().push_back(_Task); // a mutex is protecting this so cast is ok
			Scheduled = true;
		}
	}

	if (Scheduled)
		WakeExecutionThread();

	return Scheduled;
}

void oDispatchQueuePrivate_Impl::Flush() threadsafe
{
	AllowEnqueues.exchange(false);
	WaitForExecutionThreadToSleep();
	AllowEnqueues.exchange(true);
}

void oDispatchQueuePrivate_Impl::Join() threadsafe
{
	if (Joinable())
	{
		AllowEnqueues.exchange(false);
		WaitForExecutionThreadToSleep();
		Running.exchange(false);
		WakeExecutionThread();
		ExecutionThread.join();
	}

	else
		oASSERT(ProtectedTasks().empty(), "Join called on a non-joinable oDispatchQueuePrivate that still has tasks. This is a case where client code isn't properly waiting for all work to be done before deinitializing a system."); // Task queue is protected by virtual the class is no longer joinable
}
