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
#include <oBasis/oTask.h>
#include <oBasis/oStdFuture.h>
#include <oPlatform/oDebugger.h>
#include "oDispatchQueueConcurrentSerial.h"

bool oDispatchQueueCreateConcurrent(const char* _DebugName, size_t _InitialTaskCapacity, threadsafe oDispatchQueueConcurrent** _ppDispatchQueue)
{
	return oDispatchQueueCreateConcurrentSerial(_DebugName, _InitialTaskCapacity, _ppDispatchQueue);
}

void oTaskRegisterThisThread(const char* _DebuggerName)
{
	oDebuggerSetThreadName(_DebuggerName);
}

void oTaskInitScheduler()
{
}

void oTaskParallelFor(size_t _Begin, size_t _End, oINDEXED_TASK&& _Function)
{
	oTaskSerialFor(_Begin, _End, std::move(_Function));
}

class oSerialWaitableTask : public oStd::detail::oWaitableTask
{
public:
	oSerialWaitableTask(oTASK&& _Task)
	{
		_Task();
	}
	
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();
	void wait() override {}
private:
	oRefCount RefCount;
};

namespace oStd {
	namespace detail {
		bool oWaitableTaskCreate(oTASK&& _Task, oWaitableTask** _ppWaitableTask)
		{
			*_ppWaitableTask = new oSerialWaitableTask(std::move(_Task));
			return !!*_ppWaitableTask;
		}

	} // detail
} // oStd

void DEPRECATED_oTaskIssueAsync(oTASK _Task)
{
	_Task();
}
