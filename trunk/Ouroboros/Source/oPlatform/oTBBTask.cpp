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
#ifndef oNO_TBB
#include <oBasis/oStdFuture.h>
#include <oPlatform/Windows/oCRTHeap.h>
#include <oPlatform/oDebugger.h>
#include <oPlatform/oReporting.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/Windows/oWindows.h>
#include <tbb/tbb.h>
#include <tbb/task_scheduler_init.h>
#include <oPlatform/oProcessHeap.h>
#include "oCRTLeakTracker.h"
#include "oDispatchQueueConcurrentTBB.h"

bool oDispatchQueueCreateConcurrent(const char* _DebugName, size_t _InitialTaskCapacity, threadsafe oDispatchQueueConcurrent** _ppDispatchQueue)
{
	return oDispatchQueueCreateConcurrentTBB(_DebugName, _InitialTaskCapacity, _ppDispatchQueue);
}

void oTaskRegisterThisThread(const char* _DebuggerName)
{
	oDebuggerSetThreadName(_DebuggerName);
	oStd::future<void> f = oStd::async([=] { oTRACE("The thread '%s' (0x%x) has exited", oSAFESTRN(_DebuggerName), oAsUint(oStd::this_thread::get_id())); }); // Issuing a NOP task causes TBB to allocate this thread's memory
	f.wait();
}

using namespace tbb;

class TBBTaskObserver : public task_scheduler_observer
{
public:
	TBBTaskObserver()
	{
		observe();
	}

	void on_scheduler_entry(bool is_worker) override
	{
		if (is_worker)
			oDebuggerSetThreadName("TBB Worker");
	}

	void on_scheduler_exit(bool is_worker) override
	{
		if (is_worker)
			oEndThread();
	}
};

class TBBTaskSchedulerInit : public oProcessSingleton<TBBTaskSchedulerInit>
{
	task_scheduler_init* init;
	TBBTaskObserver* observer;

public:
	static const oGUID GUID;

	TBBTaskSchedulerInit()
	{
		bool oldEnabled = oCRTLeakTracker::Singleton()->IsEnabled();
		oCRTLeakTracker::Singleton()->Enable(false);
		observer = new TBBTaskObserver;
		init = new task_scheduler_init;
		oCRTLeakTracker::Singleton()->EnableThreadlocalTracking(oldEnabled);
	}
	~TBBTaskSchedulerInit()
	{
		delete init;
		delete observer;
	}
};

// {CFEBF25E-97EA-4BAB-AC50-D53474D3C758}
const oGUID TBBTaskSchedulerInit::GUID = { 0xcfebf25e, 0x97ea, 0x4bab, { 0xac, 0x50, 0xd5, 0x34, 0x74, 0xd3, 0xc7, 0x58 } };

void oTaskInitScheduler()
{
	// Instantiate the singleton
	TBBTaskSchedulerInit::Singleton();
}

void oTaskParallelFor(size_t _Begin, size_t _End, oINDEXED_TASK&& _Function)
{
	parallel_for(_Begin, _End, std::move(_Function));
}

class TBBTask : public task
{
public:
	TBBTask(oTASK _Task) :
	  Task(_Task)
	{}
	task* execute() 
	{
		Task();
		return nullptr;
	}
private:
	oTASK Task;
};

class oTBBWaitableTask : public oStd::detail::oWaitableTask
{
	// You can only wait on a spawned task, and spawned tasks can only come from 
	// a parent task, so create an empty task that can wait on its children, which
	// will always be count==1, (one child) and will be the task actually doing 
	// work.

public:
	oTBBWaitableTask(oTASK&& _Task)
	{
		pParentTask = new(task::allocate_root(Context)) empty_task;
		pParentTask->set_ref_count(1);
		pChildTask = new( pParentTask->allocate_additional_child_of(*pParentTask)) tbb::internal::function_task<oTASK>(std::move(_Task));
		pParentTask->spawn(*pChildTask);
	}
	
	~oTBBWaitableTask()
	{
		if (pParentTask->ref_count() > 1)
		{
			// Don't wait if we ourselves are the task to be closed out
			if (&tbb::task::self() != pChildTask)
				pParentTask->wait_for_all();
			else
				pParentTask->decrement_ref_count(); // but do to the ref count what wait_for_all does (1->0)
		}

		task::destroy(*pParentTask);
	}
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();
	void wait() override { pParentTask->wait_for_all(); }
private:
	oRefCount RefCount;
	tbb::task* pParentTask;
	tbb::task* pChildTask;
	task_group_context Context;
};

namespace oStd {
	namespace detail {
		bool oWaitableTaskCreate(oTASK&& _Task, oWaitableTask** _ppWaitableTask)
		{
			*_ppWaitableTask = new oTBBWaitableTask(std::move(_Task));
			return !!*_ppWaitableTask;
		}

	} // detail
} // oStd

void DEPRECATED_oTaskIssueAsync(oTASK _Task)
{
	// @oooii-kevin: For tasks with no dependency we use task::enqueue this ensures the main thread never has to participate 
	// in TBB threading and prioritizes tasks that are issued without dependency as these tend to be tasks that 
	// are longer running and behave more like raw threads
	//
	// task::enqueue vs task::spawn
	// from http://software.intel.com/en-us/blogs/2010/05/04/tbb-30-new-today-version-of-intel-threading-building-blocks/
	// The TBB 3.0 schedule supports task::enqueue, which is effectively a “run me after the other things already pending” request. 
	// Although similar to spawning a task, an enqueued task is scheduled in a different manner.  Enqueued tasks are valuable when 
	// approximately first-in first-out behavior is important, such as in situations where latency of response is more important than 
	// efficient throughput.

	// @oooii-tony: This can report a false-positive leak with the Microsoft CRT 
	// leak reporter if task::allocate_root() is called in the middle of a memory
	// state check block. (working with _CrtMemState elsewhere). See oBug_1856
	// for more information.
	// NOTE: I tried to wrap this in a disable-tracking, reenable-after block, but
	// that can cause deadlocks as all of CRT it seems shares the same mutex. Also
	// just setting the state allows for any number of threads to have their 
	// allocs ignored during the disabled period. I tried having 

	oCRTLeakTracker::Singleton()->EnableThreadlocalTracking(false);
	task& taskToSpawn = *new(task::allocate_root()) TBBTask(_Task);
	oCRTLeakTracker::Singleton()->EnableThreadlocalTracking();
	task::enqueue(taskToSpawn);
}

#endif //OOOII_NO_TBB
