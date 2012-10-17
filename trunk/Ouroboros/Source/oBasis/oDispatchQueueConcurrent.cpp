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
#include <oBasis/oDispatchQueueConcurrent.h>
#include <oBasis/oBackoff.h>
#include <oBasis/oConcurrentQueue.h>
#include <oBasis/oConditionVariable.h>
#include <oBasis/oError.h>
#include <oBasis/oFor.h>
#include <oBasis/oInitOnce.h>
#include <oBasis/oMutex.h>
#include <oBasis/oRef.h>
#include <oBasis/oRefCount.h>
#include <oBasis/oTask.h>
#include <oBasis/oThread.h>
#include <vector>

struct oWorkerThread
{
	oWorkerThread(threadsafe struct oDispatchQueueConcurrent_Impl* _pThreadpool);
	void Flush() threadsafe;
	bool Joinable() const threadsafe { return Thread.joinable(); }
	void Join() threadsafe;
	void WakeUp() threadsafe;
	void Exit() threadsafe;
	void Proc();
	oThread::id GetID() const threadsafe { return Thread.get_id(); }
protected:
	threadsafe oDispatchQueueConcurrent_Impl* pThreadpool; // raw to prevent circular refs
	threadsafe oMutex WorkAvailableMutex;
	threadsafe oConditionVariable WorkAvailable;
	oStd::atomic_bool Running;
	oThread Thread;
};

oWorkerThread::oWorkerThread(threadsafe oDispatchQueueConcurrent_Impl* _pThreadpool)
	: pThreadpool(_pThreadpool)
	, Running(false)
{
	Thread = oThread(&oWorkerThread::Proc, this);
	while (!Running);
}

void oWorkerThread::Join() threadsafe
{
	Running.exchange(false);
	WorkAvailable.notify_all();
	Thread.join();
}

void oWorkerThread::WakeUp() threadsafe
{
	WorkAvailable.notify_all();
}

void oWorkerThread::Exit() threadsafe
{
	Running.exchange(false);
	WakeUp();
}

struct oDispatchQueueConcurrent_Impl : oDispatchQueueConcurrent
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE2(oDispatchQueue, oDispatchQueueConcurrent_Impl);

	oDispatchQueueConcurrent_Impl(const char* _DebugName, size_t _InitialTaskCapacity, bool* _pSuccess);
	~oDispatchQueueConcurrent_Impl();

	bool Dispatch(oTASK _Task) threadsafe override;
	void Flush() threadsafe override;
	bool Joinable() const threadsafe override { return Workers->size() >= 1 && Workers->front()->Joinable(); }
	void Join() threadsafe override;
	const char* GetDebugName() const threadsafe override { return Tasks.debug_name(); }
	void WaitForWorkersToSleep() threadsafe;

	typedef std::vector<oWorkerThread*> workers_t;
	oInitOnce<workers_t> Workers;

	typedef oConcurrentQueue<oTASK> queue_t;
	queue_t Tasks;

	oStd::atomic_bool AllowEnqueues;
	oRefCount RefCount;

	void InitializeWorkers(workers_t& _Workers);
};

const oGUID& oGetGUID(threadsafe const oDispatchQueueConcurrent_Impl* threadsafe const*)
{
	// {C86BB586-33C0-493C-B940-75ED91F336A2}
	static const oGUID oIIDDispatchQueueConcurrent = { 0xc86bb586, 0x33c0, 0x493c, { 0xb9, 0x40, 0x75, 0xed, 0x91, 0xf3, 0x36, 0xa2 } };
	return oIIDDispatchQueueConcurrent;
}

void oWorkerThread::Flush() threadsafe
{
	while (!pThreadpool->Tasks.empty()) // concurrency is protected by the Lock below, not this test, so the cast is ok
		oLockGuard<oMutex> Lock(WorkAvailableMutex);
};

void oWorkerThread::Proc()
{
	oTaskRegisterThisThread("oDQConcurrent Worker");
	oBackoff bo;
	Running.exchange(true);

	oUniqueLock<oMutex> Lock(WorkAvailableMutex);
	while (Running)
	{
		oTASK task;
		if (pThreadpool->Tasks.try_pop(task))
		{
			task();
			bo.Reset();
		}

		else if (!bo.TryPause())
			WorkAvailable.wait(Lock);
	}

	oEndThread();
}

void oDispatchQueueConcurrent_Impl::InitializeWorkers(workers_t& _Workers)
{
	_Workers.resize(oThread::hardware_concurrency());
	for (size_t i = 0; i < _Workers.size(); i++)
		_Workers[i] = new oWorkerThread(this);
}

oDispatchQueueConcurrent_Impl::oDispatchQueueConcurrent_Impl(const char* _DebugName, size_t _InitialTaskCapacity, bool* _pSuccess)
	: Tasks(_DebugName, _InitialTaskCapacity)
	, AllowEnqueues(true)
{
	InitializeWorkers(Workers.Initialize());
	*_pSuccess = true;
}

oDispatchQueueConcurrent_Impl::~oDispatchQueueConcurrent_Impl()
{
	if (Joinable())
	{
		oASSERT(false, "Calling std::terminate because a Joinable oDispatchQueueConcurrent was destroyed");
		std::terminate();
	}

	oFOR(oWorkerThread* w, *Workers)
		delete w;
}

bool oDispatchQueueConcurrent_Impl::Dispatch(oTASK _Task) threadsafe
{
	if (AllowEnqueues)
	{
		if (Workers->empty())
			_Task();
		else
		{
			Tasks.push(_Task);
			oFOR(oWorkerThread* w, *Workers)
				w->WakeUp();
		}
		
		return true;
	}

	return false;
}

void oDispatchQueueConcurrent_Impl::Flush() threadsafe
{
	AllowEnqueues.exchange(false);
	oFOR(oWorkerThread* w, *Workers)
		w->Flush();
	AllowEnqueues.exchange(true);
}

void oDispatchQueueConcurrent_Impl::Join() threadsafe
{
	AllowEnqueues.exchange(false);
	oFOR(oWorkerThread* w, *Workers)
		w->Flush();

	oFOR(oWorkerThread* w, *Workers)
		w->Exit();

	oFOR(oWorkerThread* w, *Workers)
		w->Join();
};

bool oDispatchQueueCreateConcurrentGeneric(const char* _DebugName, size_t _InitialTaskCapacity, threadsafe oDispatchQueueConcurrent** _ppDispatchQueue)
{
	if (!_DebugName || !_ppDispatchQueue)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	bool success = false;
	oCONSTRUCT(_ppDispatchQueue, oDispatchQueueConcurrent_Impl(_DebugName, _InitialTaskCapacity, &success));
	return !!*_ppDispatchQueue;
}
