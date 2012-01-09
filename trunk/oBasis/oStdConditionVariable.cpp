// $(header)
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

oStd::cv_status oStd::condition_variable::wait_for(oStd::unique_lock<mutex>& _Lock, unsigned int _TimeoutMS)
{
	oASSERT(_Lock.mutex(), "Invalid mutex");
	oASSERT(_Lock.owns_lock(), "Lock must own the mutex lock");
	if (!SleepConditionVariableSRW((PCONDITION_VARIABLE)&Footprint, (PSRWLOCK)_Lock.mutex()->native_handle(), _TimeoutMS, 0))
	{
		DWORD err = GetLastError();
		oASSERT(err == ERROR_TIMEOUT || err == WAIT_TIMEOUT, "");
		return oStd::timeout;
	}

	return oStd::no_timeout;
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
	oThreadAtExit(oBIND(InternalNotify, new ctx(_ConditionVariable, std::move(_Lock))));
}
