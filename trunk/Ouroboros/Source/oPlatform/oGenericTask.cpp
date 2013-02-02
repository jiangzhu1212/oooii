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
#include <oBasis/oDispatchQueueConcurrent.h>
#include <oBasis/oEvent.h>
#include <oBasis/oStdFuture.h>
#include <oPlatform/oDebugger.h>
#include <oPlatform/oSingleton.h>
#include <oBasis/oGUID.h>
#include "oCRTLeakTracker.h"

bool oDispatchQueueCreateConcurrent(const char* _DebugName, size_t _InitialTaskCapacity, threadsafe oDispatchQueueConcurrent** _ppDispatchQueue)
{
	return oDispatchQueueCreateConcurrentGeneric(_DebugName, _InitialTaskCapacity, _ppDispatchQueue);
}

void oTaskRegisterThisThread(const char* _DebuggerName)
{
	oDebuggerSetThreadName(_DebuggerName);
}

class oDispatchQueueGenericContext : public oProcessSingleton<oDispatchQueueGenericContext>
{
public:
	static const oGUID GUID;

	oDispatchQueueGenericContext()
	{
		oVERIFY(oDispatchQueueCreateConcurrentGeneric("oDispatchQueueConcurrentGeneric.Singleton", 100000, &Queue));
	}

	oRef<threadsafe oDispatchQueueConcurrent> Queue;
};

// {AC624482-13BC-4942-89AC-0525BDF62B1F}
const oGUID oDispatchQueueGenericContext::GUID = { 0xac624482, 0x13bc, 0x4942, { 0x89, 0xac, 0x5, 0x25, 0xbd, 0xf6, 0x2b, 0x1f } };
oSINGLETON_REGISTER(oDispatchQueueGenericContext);

void oTaskInitScheduler()
{
	oDispatchQueueGenericContext::Singleton();
}

static void oTaskParallelForInternal(size_t _Begin, size_t _End, oINDEXED_TASK& _Function)
{
	for (size_t i = _Begin; i < _End; i++)
		oDispatchQueueGenericContext::Singleton()->Queue->Dispatch(oBIND(_Function, i));
}

void oTaskParallelFor(size_t _Begin, size_t _End, oINDEXED_TASK&& _Function)
{
	oDispatchQueueGenericContext::Singleton()->Queue->Dispatch(oBIND(oTaskParallelForInternal, _Begin, _End, _Function));
}

class oGenericWaitableTask : public oStd::detail::oWaitableTask
{
	// @oooii-tony: This was written to just cover bases when factoring out 
	// possible oTask implementations, so I apologize for no work stealing here.

	// This also can have a horrible deadlock condition. What if you have a 4-core
	// machine and there are 4 waits on each thread? Deadlock. Again, this is 
	// placeholdery at the moment and would need more love if this is to be used
	// extensively.

public:
	oGenericWaitableTask(oTASK&& _Task)
	{
		oCRTLeakTracker::Singleton()->EnableThreadlocalTracking(false);
		oDispatchQueueGenericContext::Singleton()->Queue->Dispatch([_Task, this]() { _Task(); e.Set(); });
		oCRTLeakTracker::Singleton()->EnableThreadlocalTracking();
	}
	
	~oGenericWaitableTask()
	{
		wait();
	}

	void Reference() override { RefCount.Reference(); }
	void Release() override { if (RefCount.Release()) delete this; }
	void wait() override
	{
		e.Wait();
	}

private:
	oEvent e;
	oRefCount RefCount;
};

namespace oStd {
	namespace detail {
		bool oWaitableTaskCreate(oTASK&& _Task, oWaitableTask** _ppWaitableTask)
		{
			*_ppWaitableTask = new oGenericWaitableTask(std::move(_Task));
			return !!*_ppWaitableTask;
		}

	} // detail
} // oStd

void DEPRECATED_oTaskIssueAsync(oTASK _Task)
{
	oCRTLeakTracker::Singleton()->EnableThreadlocalTracking(false);
	oDispatchQueueGenericContext::Singleton()->Queue->Dispatch(std::move(_Task));
	oCRTLeakTracker::Singleton()->EnableThreadlocalTracking();
}
