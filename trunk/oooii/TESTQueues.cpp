// $(header)
#include <oooii/oConcurrentQueueMS.h>
#include <oooii/oConcurrentQueueOptimisticFIFO.h>
#include <oooii/oBuffer.h>
#include <oooii/oCPU.h>
#include <oooii/oLockedPointer.h>
#include <oooii/oRef.h>
#include <oooii/oRefCount.h>
#include <oooii/oSTL.h>
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
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<oThread::Proc>());

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
		std::vector<oRef<threadsafe oThread> > threadArray(cpuDesc.NumHardwareThreads + 5); // throw in some contention

		for (size_t i = 0; i < threadArray.size(); i++)
		{
			char threadName[64];
			sprintf_s(threadName, "TestThread%03u", i);

			oRef<TESTQueues_PushAndPop<T, QueueT> > proc;
			proc /= new TESTQueues_PushAndPop<T, QueueT>(&q, 1000, 100);
			oTESTB(oThread::Create(threadName, 64*1024, false, proc, &threadArray[i]), "%s Failed to create thread \"%s\"", q.GetDebugName(), threadName);
		}
	}

	return true;
}

void DeleteAndCount(void* _pPointer, size_t* _pCounter)
{
	(*_pCounter)++;
	delete [] _pPointer;
}

template<typename QueueT> oTest::RESULT TestQueueNonTrivialContents(const char* _QueueName, char* _StrStatus, size_t _SizeofStrStatus)
{
	QueueT q(_QueueName);

	static const size_t kNumBuffers = 3;
	static const size_t kBufferSize = oKB(1);

	size_t numDeleted = 0;
	
	// Create a list of buffers and push them onto the queue, then lose the refs
	// of the original buffers.
	{

		oRef<threadsafe oBuffer> Buffers[kNumBuffers];
		for (size_t i = 0; i < kNumBuffers; i++)
		{
			char name[32];
			sprintf_s(name, "TestBuffer%02u", i);

			void* p = new char[kBufferSize];
			memset(p, 123, kBufferSize);
			oTESTB(p, "Out of memory allocating test buffers");
			oTESTB(oBuffer::Create(name, p, kBufferSize, oBIND(DeleteAndCount, oBIND1, &numDeleted), &Buffers[i]), "Failed to create test buffer %u", i);
			q.Push(Buffers[i]);
		}
	}
	oTESTB(numDeleted == 0, "Test buffers were deleted when the queue should be holding a reference");

	oRef<threadsafe oBuffer> b;
	while (q.TryPop(b))
	{
		oLockedPointer<oBuffer> pBuffer(b);
		const char* p = pBuffer->GetData<const char>();
		for (size_t i = 0; i < kBufferSize; i++)
			oTESTB(*p++ == 123, "Buffer mismatch on %u%s byte", i, oOrdinal((int)i));
	}

	b = nullptr;

	oTESTB(q.IsEmpty() && numDeleted == kNumBuffers, "Queue is retaining a reference to an element though it's supposed to be empty.");

	return oTest::SUCCESS;
}

struct TESTQueues : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		if (!TestQueueBasicAPI<int, oConcurrentQueueMS<int> >("oConcurrentQueueMS", _StrStatus, _SizeofStrStatus))
			return FAILURE;

		if (!TestQueueConcurrency<int, oConcurrentQueueMS<int> >("oConcurrentQueueMS", _StrStatus, _SizeofStrStatus))
			return FAILURE;

		if (SUCCESS != TestQueueNonTrivialContents<oConcurrentQueueMS<oRef<threadsafe oBuffer> > >("oConcurrentQueueMS", _StrStatus, _SizeofStrStatus))
			return FAILURE;

		#ifdef oBUG_1216 // oConcurrentQueueOptimisticFIFO is not working with vcrt10
			if (!TestQueueBasicAPI<int, oConcurrentQueueOptimisticFIFO<int> >("oConcurrentQueueOptimisticFIFO", _StrStatus, _SizeofStrStatus))
				return FAILURE;

			if (!TestQueueConcurrency<int, oConcurrentQueueOptimisticFIFO<int> >("oConcurrentQueueOptimisticFIFO", _StrStatus, _SizeofStrStatus))
				return FAILURE;
		#endif

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTQueues);