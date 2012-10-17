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
#include <oBasis/oStdConditionVariable.h>
#include <oBasis/oFunction.h>
#include <oBasis/oThread.h>
#include "oWinHeaders.h"

oStd::condition_variable::condition_variable()
{
	InitializeConditionVariable((PCONDITION_VARIABLE)&Footprint);
}

oStd::condition_variable::~condition_variable()
{
}

void oStd::condition_variable::notify_one()
{
	WakeConditionVariable((PCONDITION_VARIABLE)&Footprint);
}

void oStd::condition_variable::notify_all()
{
	WakeAllConditionVariable((PCONDITION_VARIABLE)&Footprint);
}

oStd::cv_status::value oStd::condition_variable::wait_for(oStd::unique_lock<mutex>& _Lock, unsigned int _TimeoutMS)
{
	oASSERT(_Lock.mutex(), "Invalid mutex");
	oASSERT(_Lock.owns_lock(), "Lock must own the mutex lock");
	if (!SleepConditionVariableSRW((PCONDITION_VARIABLE)&Footprint, (PSRWLOCK)_Lock.mutex()->native_handle(), _TimeoutMS, 0))
	{
		DWORD err = GetLastError();
		oASSERT(err == ERROR_TIMEOUT || err == WAIT_TIMEOUT, "");
		return oStd::cv_status::timeout;
	}

	return oStd::cv_status::no_timeout;
}

void oStd::condition_variable::wait(oStd::unique_lock<oStd::mutex>& _Lock)
{
	wait_for(_Lock, INFINITE);
}

struct ctx
{
	ctx(oStd::condition_variable& _ConditionVariable, oStd::unique_lock<oStd::mutex> _Lock)
		: ConditionVariable(_ConditionVariable)
		, Lock(std::move(_Lock))
	{}

	oStd::condition_variable& ConditionVariable;
	oStd::unique_lock<oStd::mutex> Lock;
};

static void InternalNotify(ctx* _pContext)
{
	_pContext->Lock.unlock();
	_pContext->ConditionVariable.notify_all();
}

void notify_all_at_thread_exit(oStd::condition_variable& _ConditionVariable, oStd::unique_lock<oStd::mutex> _Lock)
{
	oASSERT(_Lock.owns_lock(), "Lock must own the mutex lock");
	oThreadAtExit(InternalNotify, new ctx(_ConditionVariable, std::move(_Lock)));
}
