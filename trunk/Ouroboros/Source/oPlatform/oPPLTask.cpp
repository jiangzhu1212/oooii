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
#include <oBasis/oTask.h>
#include <oBasis/oStdFuture.h>
#include <oPlatform/oDebugger.h>
#include "oCRTLeakTracker.h"
#include <concrt.h>
#include <ppl.h>
#include "oDispatchQueueConcurrentPPL.h"

bool oDispatchQueueCreateConcurrent(const char* _DebugName, size_t _InitialTaskCapacity, threadsafe oDispatchQueueConcurrent** _ppDispatchQueue)
{
	return oDispatchQueueCreateConcurrentPPL(_DebugName, _InitialTaskCapacity, _ppDispatchQueue);
}

void oTaskRegisterThisThread(const char* _DebuggerName)
{
	oDebuggerSetThreadName(_DebuggerName);
	//oStd::future<void> f = oStd::async([=] { oTRACE("The thread '%s' (0x%x) has exited", oSAFESTRN(_DebuggerName), oAsUint(oStd::this_thread::get_id())); }); // Issuing a NOP task causes TBB to allocate this thread's memory
	//f.wait();
}

void oTaskInitScheduler()
{
	//TBBTaskSchedulerInit::Singleton();
}

void oTaskParallelFor(size_t _Begin, size_t _End, oINDEXED_TASK&& _Function)
{
	Concurrency::parallel_for(_Begin, _End, std::move(_Function));
}

class oPPLWaitableTask : public oStd::detail::oWaitableTask
{
public:
	oPPLWaitableTask(oTASK&& _Task)
	{
		ParentTask.run(std::move(_Task));
	}
	
	~oPPLWaitableTask()
	{
		wait();
	}

	void Reference() override { RefCount.Reference(); }
	void Release() override { if (RefCount.Release()) delete this; }
	void wait() override { ParentTask.wait(); }
private:
	oRefCount RefCount;
	Concurrency::task_group ParentTask;
};

namespace oStd {
	namespace detail {
		bool oWaitableTaskCreate(oTASK&& _Task, oWaitableTask** _ppWaitableTask)
		{
			*_ppWaitableTask = new oPPLWaitableTask(std::move(_Task));
			return !!*_ppWaitableTask;
		}

	} // detail
} // oStd

void oPPLExec(void* _pTask)
{
	oTASK* task = static_cast<oTASK*>(_pTask);
	(*task)();
	delete task;
}

void DEPRECATED_oTaskIssueAsync(oTASK _Task)
{
	oCRTLeakTracker::Singleton()->EnableThreadlocalTracking(false);
	oTASK* task = new oTASK(std::move(_Task));
	oCRTLeakTracker::Singleton()->EnableThreadlocalTracking();
	Concurrency::CurrentScheduler::ScheduleTask(oPPLExec, task);
}
