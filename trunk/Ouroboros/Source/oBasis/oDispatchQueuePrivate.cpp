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
	bool Running;

	// If true, allow new tasks to be enqueued. Sometimes there is logic that
	// must be executed through the queue before the calling thread can progress,
	// such as creation of a resource that must occur on the other thread. This
	// is exposed as the parameter to Flush where the user can choose to either
	// temporarily get to a known point in execution and then continue, or prevent
	// any future work at all from every entering the queue, such as approaching
	// shutdown.
	bool AllowEnqueues;

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

	void InternalFlush(bool _ReallowEnqueues) threadsafe;
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

void oDispatchQueuePrivate_Impl::WorkerThreadProc()
{
	oTASK Task;
	bool PopSucceeded = false;
	oTaskRegisterThisThread(*DebugName);

	oUniqueLock<oMutex> QueueLock(QueueMutex, oStd::defer_lock);
	Running = true;
	Initialized.exchange(true);

	while (true)
	{
		QueueLock.lock();

		while (Running && Tasks.empty())
			WorkAvailable.wait(QueueLock);

		if (!Tasks.empty())
		{
			// This has to unlock while the task is executing to allow other threads
			// to call dispatch. It also needs to call the task while the task is 
			// still in the list so if a flush is looking for the task list to be 
			// empty it won't be until AFTER the task is finished.

			QueueLock.unlock();

			// Make a copy of the task in case the task's destruction (from pop_front) 
			// causes a Dispatch into the queue which will deadlock on QueueLock
			auto CurrentTask = Tasks.front();
			CurrentTask();
			QueueLock.lock();
			Tasks.pop_front();
			QueueLock.unlock();
		}

		else if (!Running)
		{
			QueueLock.unlock();
			break;
		}

		else
			QueueLock.unlock();
	}

	oASSERT(Tasks.empty(), "Task list is not empty");

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
		WorkAvailable.notify_all();

	return Scheduled;
}

void oDispatchQueuePrivate_Impl::InternalFlush(bool _ReallowEnqueues) threadsafe
{
	QueueMutex.lock();
	AllowEnqueues = false;
	Running = _ReallowEnqueues; // if we're going to reallow, it's because we're still running. If we're not reallowing, it's because we're exiting.

	QueueMutex.unlock();

	// Wait for the task queue to be empty, then reallow enqueues
	while (true)
	{
		QueueMutex.lock();

		if (ProtectedTasks().empty())
		{
			AllowEnqueues = _ReallowEnqueues;
			QueueMutex.unlock();
			break;
		}

		QueueMutex.unlock();
		WorkAvailable.notify_all();
	}
}

void oDispatchQueuePrivate_Impl::Flush() threadsafe
{
	WaitExecutionThreadInitialized();
	InternalFlush(true);
}

void oDispatchQueuePrivate_Impl::Join() threadsafe
{
	WaitExecutionThreadInitialized();
	oASSERT(ExecutionThread.get_id() != oStd::this_thread::get_id(), "Join should not be called from the execution thread because it will be waiting on itself.");

	if (Joinable())
	{
		InternalFlush(false);
		WorkAvailable.notify_all();
		ExecutionThread.join();

		oASSERT(!ExecutionThread.joinable(), "ExecutionThread is back from a join, and yet still considers itself joinable");
	}

	else
		oASSERT(ProtectedTasks().empty(), "Join called on a non-joinable oDispatchQueuePrivate that still has tasks. This is a case where client code isn't properly waiting for all work to be done before deinitializing a system."); // Task queue is protected by virtual the class is no longer joinable
}
