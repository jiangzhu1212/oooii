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
#include <oBasis/oConcurrentQueue.h>
#include <oBasis/oError.h>
#include <oBasis/oMacros.h>
#include <oBasis/oRef.h>
#include <oBasis/oRefCount.h>
#include <oBasis/oString.h>
#include <oBasis/oStdThread.h>
#include <vector>

#define oTESTB(test, msg, ...) do { if (!(test)) return oErrorSetLast(oERROR_GENERIC, msg, ## __VA_ARGS__); } while(false)

template<typename T, typename QueueT> bool TestQueueBasicAPI(const char* _QueueName)
{
	QueueT q(_QueueName);

	for (T i = 0; i < 100; i++)
		oTESTB(q.try_push(i), "%s TryPush failed", q.debug_name());

	oTESTB(!q.empty(), "%s reports empty when it's not", q.debug_name());

	q.clear();
	oTESTB(q.empty(), "%s cleared, but reports non-empty when it's empty", q.debug_name());

	for (T i = 0; i < 100; i++)
		oTESTB(q.try_push(i), "%s TryPush failed", q.debug_name());

	int test = -1;
	for (T i = 0; i < 100; i++)
	{
		oTESTB(q.try_pop(test), "%s TryPop failed", q.debug_name());
		oTESTB(test == i, "%s value from TryPop failed", q.debug_name());
	}

	oTESTB(!q.try_pop(test), "%s TryPop on an empty list succeeded (it shouldn't)", q.debug_name());
	oTESTB(q.empty(), "%s not empty", q.debug_name());

	oErrorSetLast(oERROR_NONE, "");
	return true;
}

template<typename T, typename QueueT>
void TESTQueues_PushAndPop(QueueT* _pQueue, size_t _NumPushPops, size_t _NumIterations)
{
	while (--_NumIterations)
	{
		for (size_t i = 0; i < _NumPushPops; i++)
			_pQueue->push(1);

		int j = 0;
		for (size_t i = 0; i < _NumPushPops; i++)
			_pQueue->try_pop(j);
	}
}

template<typename T, typename QueueT> bool TestQueueConcurrency(const char* _QueueName)
{
	QueueT q(_QueueName);

	// Scope to ensure queue is cleaned up AFTER all threads.
	{
		std::vector<oStd::thread> threadArray(oStd::thread::hardware_concurrency() + 5); // throw in some contention

		for (size_t i = 0; i < threadArray.size(); i++)
		{
			oStringS threadName;
			oPrintf(threadName, "TestThread%03u", i);
			threadArray[i] = oStd::thread(TESTQueues_PushAndPop<T, QueueT>, &q, 1000, 100);
		}

		for (size_t i = 0; i < threadArray.size(); i++)
			threadArray[i].join();
	}

	oErrorSetLast(oERROR_NONE, "");
	return true;
}

struct oTestBuffer
{
	oTestBuffer(size_t _Size, size_t* _pNumDeleted) : Pointer(new char[_Size]), pNumDeleted(_pNumDeleted) {}
	~oTestBuffer() { (*pNumDeleted)++; delete [] Pointer; }

	void Reference() threadsafe { RefCount.Reference(); }
	void Release() threadsafe { if (RefCount.Release()) delete this; }

	char* Pointer;
	size_t* pNumDeleted;
	oRefCount RefCount;
};

void intrusive_ptr_add_ref(threadsafe oTestBuffer* p) { p->Reference(); }
void intrusive_ptr_release(threadsafe oTestBuffer* p) { p->Release(); }

bool oTestBufferCreate(size_t _Size, size_t* _pNumDeleted, threadsafe oTestBuffer** _ppBuffer)
{
	*_ppBuffer = new oTestBuffer(_Size, _pNumDeleted);
	return true;
}

template<typename QueueT> bool TestQueueNonTrivialContents(const char* _QueueName)
{
	QueueT q(_QueueName);

	static const size_t kNumBuffers = 3;
	static const size_t kBufferSize = oKB(1);

	size_t numDeleted = 0;
	
	// Create a list of buffers and push them onto the queue, then lose the refs
	// of the original buffers.
	{
		oRef<threadsafe oTestBuffer> Buffers[kNumBuffers];
		for (size_t i = 0; i < kNumBuffers; i++)
		{
			oTESTB(oTestBufferCreate(kBufferSize, &numDeleted, &Buffers[i]), "Failed to create test buffer %u", i);
			memset(Buffers[i]->Pointer, 123, kBufferSize);
			q.push(Buffers[i]);
		}
	}
	oTESTB(numDeleted == 0, "Test buffers were deleted when the queue should be holding a reference");

	oRef<threadsafe oTestBuffer> b;
	while (q.try_pop(b))
	{
		const threadsafe char* p = b->Pointer;
		for (size_t i = 0; i < kBufferSize; i++)
			oTESTB(*p++ == 123, "Buffer mismatch on %u%s byte", i, oOrdinal((int)i));
	}

	b = nullptr;

	oTESTB(q.empty() && numDeleted == kNumBuffers, "Queue is retaining a reference to an element though it's supposed to be empty.");

	oErrorSetLast(oERROR_NONE, "");
	return true;
}

bool oBasisTest_oConcurrentQueue()
{
	const char* n = "oConcurrentQueue";
	if (!TestQueueBasicAPI<int, oConcurrentQueue<int> >(n))
		return false;
	if (!TestQueueConcurrency<int, oConcurrentQueue<int> >(n))
		return false;
	if (!TestQueueNonTrivialContents<oConcurrentQueue<oRef<threadsafe oTestBuffer> > >(n))
		return false;

	return true;
}

#if 0
// Prove oConcurrentQueue is dropin for concurrent_queue
#include <concurrent_queue.h>
template<typename T> class concrt_concurrent_queue : public Concurrency::concurrent_queue<T>
{
	const char* DebugName;
public:
	concrt_concurrent_queue(const char* _DebugName)
		: DebugName(_DebugName)
	{}

	bool try_push(const T& _Element) { push(_Element); return true; }
	bool valid() const threadsafe { return true; }
	const char* debug_name() const threadsafe { return DebugName; }
	size_t capacity() const threadsafe { return 100000; }
};

bool oBasisTest_concrtConcurrentQueue()
{
	const char* n = "concrt_concurrent_queue";
	if (!TestQueueBasicAPI<int, concrt_concurrent_queue<int> >(n))
		return false;
	if (!TestQueueConcurrency<int, concrt_concurrent_queue<int> >(n))
		return false;
	if (!TestQueueNonTrivialContents<concrt_concurrent_queue<oRef<threadsafe oTestBuffer> > >(n))
		return false;
	return true;
}

#else
bool oBasisTest_concrtConcurrentQueue()
{
	return oErrorSetLast(oERROR_NOT_FOUND, "concrt queue test not enabled");
}
#endif

#if 0
#include <tbb/concurrent_queue.h>
template<typename T> class tbb_concurrent_queue : public tbb::concurrent_queue<T>
{
	const char* DebugName;
public:
	tbb_concurrent_queue(const char* _DebugName)
		: DebugName(_DebugName)
	{}

	bool try_push(const T& _Element) { push(_Element); return true; }
	bool valid() const threadsafe { return true; }
	const char* debug_name() const threadsafe { return DebugName; }
	size_t capacity() const threadsafe { return 100000; }
};

bool oBasisTest_tbbConcurrentQueue()
{
	const char* n = "tbb_concurrent_queue";
	if (!TestQueueBasicAPI<int, tbb_concurrent_queue<int> >(n))
		return false;
	if (!TestQueueConcurrency<int, tbb_concurrent_queue<int> >(n))
		return false;
	if (!TestQueueNonTrivialContents<tbb_concurrent_queue<oRef<threadsafe oTestBuffer> > >(n))
		return false;
	return true;
}
#else
bool oBasisTest_tbbConcurrentQueue()
{
	return oErrorSetLast(oERROR_NOT_FOUND, "tbb queue test not enabled");
}
#endif
