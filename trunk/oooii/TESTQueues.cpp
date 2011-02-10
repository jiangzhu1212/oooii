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
#include "pch.h"
#include <oooii/oConcurrentQueueMS.h>
#include <oooii/oConcurrentQueueOptimisticFIFO.h>
#include <oooii/oCPU.h>
#include <oooii/oRef.h>
#include <oooii/oRefCount.h>
#include <oooii/oTest.h>
#include <oooii/oThread.h>

template<typename T, typename QueueT> bool TestQueueBasicAPI(const char* _QueueName, char* _StrStatus, size_t _SizeofStrStatus)
{
	QueueT q(_QueueName);

	oTESTB(q.IsValid(), "%s is not valid", q.GetDebugName());
	oTESTB(q.GetCapacity() == 100000, "%s capacity default is not specified the same as other queues", q.GetDebugName());

	for (T i = 0; i < 100; i++)
		oTESTB(q.TryPush(i), "%s TryPush failed", q.GetDebugName());

	oTESTB(!q.IsEmpty(), "%s reports empty when it's not", q.GetDebugName());

	q.Clear();
	oTESTB(q.IsEmpty(), "%s cleared, but reports non-empty when it's empty", q.GetDebugName());

	for (T i = 0; i < 100; i++)
		oTESTB(q.TryPush(i), "%s TryPush failed", q.GetDebugName());

	int test = -1;
	for (T i = 0; i < 100; i++)
	{
		oTESTB(q.TryPop(test), "%s TryPop failed", q.GetDebugName());
		oTESTB(test == i, "%s value from TryPop failed", q.GetDebugName());
	}

	oTESTB(!q.TryPop(test), "%s TryPop on an empty list succeeded (it shouldn't)", q.GetDebugName());
	oTESTB(q.IsEmpty(), "%s not empty", q.GetDebugName());

	return true;
}

template<typename T, typename QueueT>
class TESTQueues_PushAndPop : public oThread::Proc
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);

	QueueT* pQueue;
	size_t NumPushPops;
	size_t NumIterations;
	oRefCount RefCount;

public:
	TESTQueues_PushAndPop(QueueT* _pQueue, size_t _NumPushPops, size_t _NumIterations)
		: pQueue(_pQueue)
		, NumPushPops(_NumPushPops)
		, NumIterations(_NumIterations)
	{}

	void RunIteration() override
	{
		if (--NumIterations == 0)
			oThread::Current()->Exit();
		else
		{
			for (size_t i = 0; i < NumPushPops; i++)
				pQueue->Push(1);

			int j = 0;
			for (size_t i = 0; i < NumPushPops; i++)
				pQueue->Pop(j);
		}
	}

	bool OnBegin() override { return true; }
	void OnEnd() override {}
};

template<typename T, typename QueueT> bool TestQueueConcurrency(const char* _QueueName, char* _StrStatus, size_t _SizeofStrStatus)
{
	QueueT q(_QueueName);

	oCPU::DESC cpuDesc;
	oTESTB(oCPU::GetDesc(0, &cpuDesc), "%s Getting CPU desc failed", q.GetDebugName());
	
	// Scope to ensure queue is cleaned up AFTER all threads.
	{
		oTestScopedArray<threadsafe oRef<oThread> > threadArray(cpuDesc.NumHardwareThreads + 5); // throw in some contention
		threadsafe oRef<oThread>* threads = threadArray.GetPointer();

		for (size_t i = 0; i < threadArray.GetCount(); i++)
		{
			char threadName[64];
			sprintf_s(threadName, "TestThread%03u", i);

			oRef<TESTQueues_PushAndPop<T, QueueT> > proc;
			proc /= new TESTQueues_PushAndPop<T, QueueT>(&q, 1000, 100);
			oTESTB(oThread::Create(threadName, 64*1024, false, proc, &threads[i]), "%s Failed to create thread \"%s\"", q.GetDebugName(), threadName);
		}
	}

	return true;
}

struct TESTQueues : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		if (!TestQueueBasicAPI<int, oConcurrentQueueMS<int> >("oConcurrentQueueMS", _StrStatus, _SizeofStrStatus))
			return FAILURE;

		if (!TestQueueConcurrency<int, oConcurrentQueueMS<int> >("oConcurrentQueueMS", _StrStatus, _SizeofStrStatus))
			return FAILURE;

		if (!TestQueueBasicAPI<int, oConcurrentQueueOptimisticFIFO<int> >("oConcurrentQueueOptimisticFIFO", _StrStatus, _SizeofStrStatus))
			return FAILURE;

		if (!TestQueueConcurrency<int, oConcurrentQueueOptimisticFIFO<int> >("oConcurrentQueueOptimisticFIFO", _StrStatus, _SizeofStrStatus))
			return FAILURE;

		return SUCCESS;
	}
};

TESTQueues TestQueues;
