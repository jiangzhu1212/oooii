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
#include "oDispatchQueueConcurrentWTP.h"
#include <oBasis/oAssert.h>
#include <oBasis/oError.h>
#include <oBasis/oInitOnce.h>
#include <oBasis/oRef.h>
#include <oBasis/oRefCount.h>
#include <oPlatform/Windows/oWindows.h>

#if defined(_WIN32) || defined(_WIN64)

struct oTASKPooled
{
public:
	static oTASKPooled* Create(oTASK _Task, threadsafe oBlockAllocatorGrowableT<oTASKPooled>* _pPool)
	{
		return new(_pPool->Allocate()) oTASKPooled(_Task, _pPool);
	}
	inline void operator()() { Task(); this->~oTASKPooled(); pPool->Deallocate(this); }
	
private:
	oTASKPooled(oTASK _Task, threadsafe oBlockAllocatorGrowableT<oTASKPooled>* _pPool)
		: Task(_Task)
		, pPool(_pPool)
	{}
	~oTASKPooled()
	{}

	oTASK Task;
	threadsafe oBlockAllocatorGrowableT<oTASKPooled>* pPool;
};

struct oDispatchQueueConcurrentWTP : oDispatchQueueConcurrent
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE2(oDispatchQueue, oDispatchQueueConcurrentWTP);

	oDispatchQueueConcurrentWTP(const char* _DebugName, size_t _InitialTaskCapacity, bool* _pSuccess);
	~oDispatchQueueConcurrentWTP();

	bool Dispatch(oTASK _Task) threadsafe override;
	void Flush() threadsafe override;
	bool Joinable() const threadsafe override { return IsJoinable; }
	void Join() threadsafe override;
	const char* GetDebugName() const threadsafe override { return *DebugName; } // safe because DebugName is immutable
	void Destroy();

	PTP_CLEANUP_GROUP TPCleanupGroup;
	PTP_POOL hPool;
	TP_CALLBACK_ENVIRON TPEnvironment;
	oInitOnce<const char*> DebugName;
	oStd::atomic_bool AllowEnqueue;
	oStd::atomic_bool IsJoinable;
	oRefCount RefCount;
	oBlockAllocatorGrowableT<oTASKPooled> Pool;

	static VOID CALLBACK WorkCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work) { (*(oTASKPooled*)Context)(); }
};

const oGUID& oGetGUID(threadsafe const oDispatchQueueConcurrentWTP* threadsafe const*)
{
	// {C4728B88-5CFE-4B84-A8CF-922F83282A88}
	static const oGUID oIIDDispatchQueueConcurrentWTP = { 0xc4728b88, 0x5cfe, 0x4b84, { 0xa8, 0xcf, 0x92, 0x2f, 0x83, 0x28, 0x2a, 0x88 } };
	return oIIDDispatchQueueConcurrentWTP;
}

oDispatchQueueConcurrentWTP::oDispatchQueueConcurrentWTP(const char* _DebugName, size_t _InitialTaskCapacity, bool* _pSuccess)
	:	TPCleanupGroup(CreateThreadpoolCleanupGroup())
	, hPool(CreateThreadpool(nullptr))
	, AllowEnqueue(true)
	, IsJoinable(true)
{
	// @oooii-tony: I don't see a way of hinting to WTP the _InitialTaskCapacity value... do you?

	DebugName.Initialize(_DebugName);

	oASSERT(TPCleanupGroup, "CreateThreadpoolCleanupGroup failed");
	InitializeThreadpoolEnvironment(&TPEnvironment);
	SetThreadpoolCallbackPool(&TPEnvironment, hPool);
	SetThreadpoolCallbackCleanupGroup(&TPEnvironment, TPCleanupGroup, nullptr);

	DWORD dwNumThreads = oStd::thread::hardware_concurrency();
	if (dwNumThreads == 0)
		dwNumThreads = 1;

	if (!SetThreadpoolThreadMinimum(hPool, dwNumThreads))
		oASSERT(false, "SetThreadPoolThreadMinimum(hPool=%x, MinConcurrency=%u) failed", hPool, dwNumThreads);
	SetThreadpoolThreadMaximum(hPool, dwNumThreads);

	*_pSuccess = true;
}

oDispatchQueueConcurrentWTP::~oDispatchQueueConcurrentWTP()
{
	if (Joinable())
	{
		oASSERT(false, "Calling std::terminate because a Joinable oDispatchQueueConcurrentWTP was destroyed");
		std::terminate();
	}
}

bool oDispatchQueueCreateConcurrentWTP(const char* _DebugName, size_t _InitialTaskCapacity, threadsafe oDispatchQueueConcurrent** _ppDispatchQueue)
{
#if defined(_WIN32) || defined(_WIN64)

	if (!_DebugName || !_ppDispatchQueue)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	bool success = false;
	oCONSTRUCT(_ppDispatchQueue, oDispatchQueueConcurrentWTP(_DebugName, _InitialTaskCapacity, &success));
	return !!*_ppDispatchQueue;

#else
	return oErrorSetLast(oERROR_NOT_FOUND, "Window's Threadpool algorithm not found on this system");
#endif
}

bool oDispatchQueueConcurrentWTP::Dispatch(oTASK _Task) threadsafe
{
	if (AllowEnqueue)
	{
		if (oStd::thread::hardware_concurrency() <= 1)
			_Task();
		else
		{
			oTASKPooled* pTask = oTASKPooled::Create(_Task, &Pool);
			PTP_WORK TPWork = CreateThreadpoolWork(WorkCallback, pTask, (PTP_CALLBACK_ENVIRON)&TPEnvironment);
			SubmitThreadpoolWork(TPWork);
		}

		return true;
	}

	return false;
}

void oDispatchQueueConcurrentWTP::Flush() threadsafe
{
	AllowEnqueue.exchange(false);
	CloseThreadpoolCleanupGroupMembers(TPCleanupGroup, false, nullptr);
	AllowEnqueue.exchange(true);
}

void oDispatchQueueConcurrentWTP::Destroy()
{
	CloseThreadpoolCleanupGroupMembers(TPCleanupGroup, false, nullptr);
	CloseThreadpoolCleanupGroup(TPCleanupGroup);
	CloseThreadpool(hPool);
	DestroyThreadpoolEnvironment(&TPEnvironment);
}

void oDispatchQueueConcurrentWTP::Join() threadsafe
{
	IsJoinable.exchange(false);
	AllowEnqueue.exchange(false);
	thread_cast<oDispatchQueueConcurrentWTP*>(this)->Destroy(); // safe because it is protected by atomics above and this is the end-of-life/deinitialization call
}

#endif
