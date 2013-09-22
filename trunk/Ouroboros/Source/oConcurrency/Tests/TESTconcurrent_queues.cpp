/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
#include <oConcurrency/concurrent_queue.h>
#include <oConcurrency/concurrent_queue_opt.h>
#include <oConcurrency/concurrent_worklist.h>
#include <oConcurrency/event.h>
#include <oBase/finally.h>
#include <oBase/assert.h>
#include <oStd/for.h>
#include <oBase/function.h>
#include <oStd/atomic.h>
#include <oStd/thread.h>
#include <oBase/throw.h>
#include <oBase/timer.h>
#include <vector>

using namespace ouro;

namespace oConcurrency {
	namespace tests {

struct oTestBuffer
{
	enum Dummy_t { Dummy };

	oTestBuffer(Dummy_t _Dummy) : Pointer(nullptr), pNumDeleted(nullptr) {}
	oTestBuffer(size_t _Size, size_t* _pNumDeleted) : Pointer(new char[_Size]), pNumDeleted(_pNumDeleted) {}
	~oTestBuffer() { (*pNumDeleted)++; delete [] Pointer; }
	bool operator==(const oTestBuffer& _That) const { return Pointer == _That.Pointer; }

	char* Pointer;
	size_t* pNumDeleted;
};

template<typename T, typename QueueT>
static void test_queue_basics(const char* _QueueName)
{
	QueueT q;

	static const T kNumPushes = 10;

	for (T i = 0; i < kNumPushes; i++)
		q.push(i);

	oCHECK(!q.empty(), "%s reports empty when it's not", _QueueName);
	oCHECK(q.size() == kNumPushes, "%s reports size %u, expected %u", _QueueName, q.size(), 100);

	int test = -1;
	for (T i = 0; i < (kNumPushes-1); i++)
		q.pop(test);

	oCHECK(!q.empty(), "%s should have 1 element, but reports as empty", _QueueName);
	oCHECK(q.size() == 1, "%s reports size %u, expected 1", _QueueName, q.size());

	q.clear();
	oCHECK(q.empty(), "%s cleared, but reports non-empty when it's empty", _QueueName);
	oCHECK(q.size() == 0, "%s reports size %u, expected %u", _QueueName, q.size(), 0);

	for (T i = 0; i < kNumPushes; i++)
		q.push(i);

	test = -1;
	if (is_fifo<QueueT>::value)
	{
		for (T i = 0; i < kNumPushes; i++)
		{
			oCHECK(q.try_pop(test), "%s TryPop failed", _QueueName);
			oCHECK(test == i, "%s value from TryPop failed", _QueueName);
		}
	}

	else
	{
		for (T i = kNumPushes-1; i >= 0; i--)
		{
			oCHECK(q.try_pop(test), "%s TryPop failed", _QueueName);
			oCHECK(test == i, "%s value from TryPop failed", _QueueName);
		}
	}

	oCHECK(!q.try_pop(test), "%s TryPop on an empty list succeeded (it shouldn't)", _QueueName);
	oCHECK(q.empty(), "%s not empty", _QueueName);
	oCHECK(q.size() == 0, "%s should be empty and thus report size 0, got size %u", _QueueName, q.size());
}

template<typename T, typename QueueT>
static void push_task(QueueT* _pQueue, size_t _NumPushes)
{
	for (size_t i = 0; i < _NumPushes; i++)
		_pQueue->push(1);
}

template<typename T, typename QueueT>
static void pop_task(QueueT* _pQueue, size_t _NumPops)
{
	T e;
	for (size_t i = 0; i < _NumPops; i++)
		_pQueue->pop(e);
}

template<typename T, typename QueueT>
static void push_and_pop_task(QueueT* _pQueue, size_t _NumPushPops, size_t _NumIterations)
{
	while (--_NumIterations)
	{
		for (size_t i = 0; i < _NumPushPops; i++)
			_pQueue->push(1);

		int j = 0;
		for (size_t i = 0; i < _NumPushPops; i++)
			_pQueue->pop(j);
	}

	//oTRACE("TESTQueues_PushAndPop finished (0x%x)", asuint(this_thread::get_id()));
}

template<typename T, typename QueueT>
static void push_and_pop_task_performance(QueueT* _pQueue, size_t _NumPushPops, size_t _NumIterations, event* _pStart)
{
	_pStart->wait();
	push_and_pop_task<T, QueueT>(_pQueue, _NumPushPops, _NumIterations);
	oTRACE("Done thread.");
}

template<typename T, typename QueueT>
static void test_concurrent_pushes(const char* _QueueName)
{
	QueueT q;
	ouro::finally ClearQueue([&] { q.clear(); });

	const size_t NumPushes = 1000;
	size_t ExpectedSize = 0;
	oCHECK(q.size() == ExpectedSize, "Empty queue not reporting 0 elements");

	// Scope to ensure queue is cleaned up AFTER all threads.
	{
		std::vector<oStd::thread> threadArray(oStd::thread::hardware_concurrency() + 5); // throw in some contention
		ExpectedSize = NumPushes * threadArray.size();

		for (size_t i = 0; i < threadArray.size(); i++)
			threadArray[i] = oStd::thread(push_task<T, QueueT>, &q, NumPushes);

		for (size_t i = 0; i < threadArray.size(); i++)
			threadArray[i].join();
	}

	size_t ActualSize = q.size();
	oCHECK(ActualSize == ExpectedSize, "Queue doesn't have as many items as it should");
}

template<typename T, typename QueueT>
static void test_concurrent_pops(const char* _QueueName)
{
	QueueT q;
	try
	{
		size_t ActualSize = q.size();
		oCHECK(ActualSize == 0, "Queue should have initialized to zero items, but has %u", ActualSize);

		// Scope to ensure queue is cleaned up AFTER all threads.
		const size_t NumPops = 100;
		{
			std::vector<oStd::thread> threadArray(oStd::thread::hardware_concurrency() + 5); // throw in some contention
			size_t InitialSize = T(NumPops * threadArray.size());
			for (T i = 0; i < T(InitialSize); i++)
				q.push(i);

			ActualSize = q.size();
			oCHECK(ActualSize == InitialSize, "size (%u) != #pushes (%u)", ActualSize, InitialSize);

			for (size_t i = 0; i < threadArray.size(); i++)
				threadArray[i] = oStd::thread(pop_task<T, QueueT>, &q, NumPops);

			for (size_t i = 0; i < threadArray.size(); i++)
				threadArray[i].join();
		}

		oCHECK(q.empty(), "Queue should be empty, but isn't");
		ActualSize = q.size();
		oCHECK(ActualSize == 0, "Queue should have zero items, but has %u", ActualSize);
	}

	catch(std::exception&)
	{
		q.clear();
	}
}

template<typename T, typename QueueT>
static void test_concurrency(const char* _QueueName)
{
	QueueT q;

	// Scope to ensure queue is cleaned up AFTER all threads.
	{
		std::vector<oStd::thread> threadArray(oStd::thread::hardware_concurrency() + 5); // throw in some contention

		for (size_t i = 0; i < threadArray.size(); i++)
			threadArray[i] = oStd::thread(push_and_pop_task<T, QueueT>, &q, 1000, 100);

		for (size_t i = 0; i < threadArray.size(); i++)
			threadArray[i].join();
	}

	oCHECK(q.empty(), "Queue is not empty after all threads have been joined");
	oCHECK(q.size() == 0, "Queue is empty, but size is non-zero (%u)", q.size());
}

template<typename T, typename QueueT>
static double test_performance(const char* _QueueName)
{
	QueueT q;
	event Start;
	double time = 0.0;

	static const size_t NumPushPops = 5000;
	static const size_t NumIterations = 1000;

	// Scope to ensure queue is cleaned up AFTER all threads.
	{
		std::vector<thread> threadArray(thread::hardware_concurrency());
		std::vector<double> Times(threadArray.size(), 0.0);

		for (size_t i = 0; i < threadArray.size(); i++)
			threadArray[i] = std::move(thread(push_and_pop_task_performance<T, QueueT>, &q, NumPushPops, NumIterations, &Start));

		ouro::finally JoinThreads([&]
		{
			for (size_t i = 0; i < threadArray.size(); i++)
				threadArray[i].join();
		});

		// Get all thread setup out of the way
		this_thread::sleep_for(chrono::milliseconds(500));
		Start.set();

		local_timer t;

		//JoinThreads();
		for (size_t i = 0; i < threadArray.size(); i++)
			threadArray[i].join();
		oTRACE("Done main.");

		time = t.seconds();
	
		//this_thread::sleep_for(chrono::milliseconds(500));
	}

	return time;
}

bool oTestBufferCreate(size_t _Size, size_t* _pNumDeleted, threadsafe oTestBuffer** _ppBuffer)
{
	*_ppBuffer = new oTestBuffer(_Size, _pNumDeleted);
	return true;
}

template<typename QueueT>
static void test_nontrivial_contents(const char* _QueueName)
{
	QueueT q;

	static const size_t kNumBuffers = 3;
	static const size_t kBufferSize = 1024;

	size_t numDeleted = 0;
	
	// Create a list of buffers and push them onto the queue, then lose the refs
	// of the original buffers.
	{
		std::shared_ptr<threadsafe oTestBuffer> Buffers[kNumBuffers];
		for (size_t i = 0; i < kNumBuffers; i++)
		{
			Buffers[i] = std::make_shared<threadsafe oTestBuffer>(kBufferSize, &numDeleted);
			memset(Buffers[i]->Pointer, 123, kBufferSize);
			q.push(Buffers[i]);
		}
	}
	oCHECK(numDeleted == 0, "Test buffers were deleted when the queue should be holding a reference");

	std::shared_ptr<threadsafe oTestBuffer> b;
	while (q.try_pop(b))
	{
		const threadsafe char* p = b->Pointer;
		for (size_t i = 0; i < kBufferSize; i++)
			oCHECK(*p++ == 123, "Buffer mismatch on byte %u", i);
	}

	b = nullptr;

	oCHECK(q.empty() && numDeleted == kNumBuffers, "Queue is retaining a reference to an element though it's supposed to be empty.");
}

template<typename IntQueueT, typename oTestBufferRefQueueT> bool TestQueueT(const char* _QueueName)
{
	test_queue_basics<int, IntQueueT>(_QueueName);
	test_concurrent_pushes<int, IntQueueT>(_QueueName);
	test_concurrent_pops<int, IntQueueT>(_QueueName);
	test_concurrency<int, IntQueueT>(_QueueName);
	test_nontrivial_contents<oTestBufferRefQueueT>(_QueueName);

	// Enable this for some performance reporting (assuming it still passes the
	// above tests!)
	#if 0
		double time = test_performance<int, IntQueueT>(_QueueName);
		oTRACE("%s perf test: %.03fs", _QueueName, time);
	#endif
	
	return true;
}

#define oTEST_QUEUET(_QueueType) TestQueueT<_QueueType<int>, _QueueType<std::shared_ptr<threadsafe oTestBuffer>>>(#_QueueType)

void TESTconcurrent_queue()
{
	oTEST_QUEUET(concurrent_queue);
}

void TESTconcurrent_queue_opt()
{
	oTEST_QUEUET(concurrent_queue_opt);
}

static const int kNumTasks = 50000;
static const int kPoppedFlag = 0x80000000;
static const int kStolenFlag = 0x40000000;
static const int kTouchedMask = 0xc0000000;
static const int kValueMask = 0x3fffffff;

typedef concurrent_worklist<int, std::allocator<int>> worklist_t;

static void push_local(worklist_t& _Work, int* _Results, bool* _pDone)
{
	begin_thread("push_local");

	for (int i = 0; i < kNumTasks; i++)
		_Work.push_local(i);

	*_pDone = true;
	end_thread();
}

static void pop_local(worklist_t& _Work, int* _Results, bool* _pDone)
{
	begin_thread("pop_local");

	for (int i = 0; i < kNumTasks; i++)
	{
		int index = -1;
		if (_Work.try_pop_local(index))
		{
			oCHECK(index >= 0 && index < kNumTasks, "try_pop_local produced an out-of-range index");
			oStd::atomic_fetch_or(&_Results[index], int(index | kPoppedFlag));
		}
	}
	
	*_pDone = true;
	end_thread();
}

static void push_pop_local(worklist_t& _Work, int* _Results, bool* _pDone)
{
	begin_thread("push_pop_local");

	for (int i = 0; i < kNumTasks; i++)
		_Work.push_local(i);

	for (int i = 0; i < kNumTasks; i++)
	{
		int index = -1;
		if (_Work.try_pop_local(index))
		{
			oCHECK(index >= 0 && index < kNumTasks, "try_pop_local produced an out-of-range index");
			oStd::atomic_fetch_or(&_Results[index], int(index | kPoppedFlag));
		}
	}

	*_pDone = true;
	end_thread();
}

static void try_steal(worklist_t& _Work, int* _Results, bool* _pDone)
{
	begin_thread("thief");
	timer timer;

	while (timer.seconds() < 5.0)
	{
		int index = -1;
		if (_Work.try_steal(index))
		{
			oCHECK(index >= 0 && index < kNumTasks, "try_pop_local produced an out-of-range index");
			oStd::atomic_fetch_or(&_Results[index], int(index | kStolenFlag));
		}
		else if (_Work.empty() && *_pDone)
			break;
	}

	end_thread();
}

static void test_stealing(const oFUNCTION<void(worklist_t& _WorkParam, int* _Results, bool* _pDone)>& _Function
	, worklist_t& _Work
	, bool* _pWorkIsEmpty
	, int* _pNumStolen
	, int* _pNumUntouchedValues
	, int* _pNumDoubleTouchedValues
	, int* _pNumUnexpectedValues)
{
	ouro::finally clear_work([&]{ _Work.clear(); });

	int Results[kNumTasks];
	memset(Results, 0, sizeof(Results));
	bool Done = false;
	oStd::thread t(_Function, oBINDREF(_Work), Results, &Done);

	std::vector<oStd::thread> thieves;
	for (unsigned int i = 0; i < oStd::thread::hardware_concurrency() + 5; i++)
		thieves.push_back(std::move(oStd::thread(try_steal, oBINDREF(_Work), Results, &Done)));

	t.join();

	oFOR(auto& th, thieves)
		th.join();
	
	*_pWorkIsEmpty = _Work.empty();
	*_pNumStolen = 0;
	*_pNumUntouchedValues = 0;
	*_pNumDoubleTouchedValues = 0;
	*_pNumUnexpectedValues = 0;
	for (int i = 0; i < kNumTasks; i++)
	{
		int touchFlag = Results[i] & kTouchedMask;
		int value = Results[i] & kValueMask;

		if (touchFlag == kStolenFlag)
			(*_pNumStolen)++;
	
		if (touchFlag == 0)
			(*_pNumUntouchedValues)++;

		if (touchFlag == kTouchedMask)
			(*_pNumDoubleTouchedValues)++;

		if (value != i)
			(*_pNumUnexpectedValues)++;
	}
}

static void test_stealing_check(const char* _Name, bool _IsEmpty, int _NumStolen, int _NumUntouched, int _NumDoubleTouched, int _NumUnexpected)
{
	oCHECK(_IsEmpty, "(%s) worklist not empty after threads were finished", _Name);
	oCHECK(_NumUntouched == 0, "(%s) There were %d untouched values", _Name, _NumUntouched);
	oCHECK(_NumDoubleTouched == 0, "(%s) There were %d double-touched values", _Name, _NumDoubleTouched);
	oCHECK(_NumUnexpected == 0, "(%s) There were %d double-touched values", _Name, _NumUnexpected);
	oTRACEA("%.02f%% stolen during %s test", 100.0f * _NumStolen / static_cast<float>(kNumTasks), _Name);
}

static void test_stealing(const char* _QueueName)
{
	worklist_t Work;
	bool IsEmpty = false;
	int nStolen = 0, nUntouched = 0, nDoubleTouched = 0, nUnexpected = 0;
	
	test_stealing(push_local, Work, &IsEmpty, &nStolen, &nUntouched, &nDoubleTouched, &nUnexpected);
	test_stealing_check("push_local", IsEmpty, nStolen, nUntouched, nDoubleTouched, nUnexpected);

	// pre-init the worklist for testing just pops
	for (int i = 0; i < kNumTasks; i++)
		Work.push_local(i);

	test_stealing(pop_local, Work, &IsEmpty, &nStolen, &nUntouched, &nDoubleTouched, &nUnexpected);
	test_stealing_check("pop_local", IsEmpty, nStolen, nUntouched, nDoubleTouched, nUnexpected);

	test_stealing(push_pop_local, Work, &IsEmpty, &nStolen, &nUntouched, &nDoubleTouched, &nUnexpected);
	test_stealing_check("push_pop_local", IsEmpty, nStolen, nUntouched, nDoubleTouched, nUnexpected);
}

void TESTconcurrent_worklist()
{
	oTEST_QUEUET(concurrent_worklist);
	test_stealing("concurrent_worklist");
}

#if 0
// Prove concurrent_queue is dropin for concurrent_queue
}}
#include <concurrent_queue.h>
namespace oConcurrency { namespace tests {
template<typename T> class concurrent_queue_concrt : private ::Concurrency::concurrent_queue<T>, public concurrent_queue_base<T, concurrent_queue_concrt<T>>
{
	typedef Concurrency::concurrent_queue<T> queue_t;
public:
	oDEFINE_CONCURRENT_QUEUE_TYPE(T, queue_t::size_type);

	concurrent_queue_concrt() {}
	void push(const_reference _Element) threadsafe { oThreadsafe(this)->queue_t::push(_Element); }
	bool try_pop(reference _Element) threadsafe { return oThreadsafe(this)->queue_t::try_pop(_Element); }
	void clear() threadsafe { return concurrent_queue_base::clear(); }
	bool empty() const threadsafe { return oThreadsafe(this)->queue_t::empty(); }
	size_type size() const { return queue_t::unsafe_size(); }
};

void TESTconcurrent_queue_concrt()
{
	oTEST_QUEUET(concurrent_queue_concrt);
}

#else
void TESTconcurrent_queue_concrt()
{
	oTRACE("oErrorSetLast(std::errc::permission_denied, \"concurrent_queue_concrt test not enabled\");");
}
#endif

#if 0
}}
#include <tbb/concurrent_queue.h>
namespace oConcurrency { namespace tests {
template<typename T> class concurrent_queue_tbb : private tbb::concurrent_queue<T>, public concurrent_queue_base<T, concurrent_queue_tbb<T>>
{
	typedef tbb::concurrent_queue<T> queue_t;
public:
	oDEFINE_CONCURRENT_QUEUE_TYPE(T, queue_t::size_type);

	concurrent_queue_tbb() {}
	void push(const_reference _Element) threadsafe { oThreadsafe(this)->queue_t::push(_Element); }
	bool try_pop(reference _Element) threadsafe { return oThreadsafe(this)->queue_t::try_pop(_Element); }
	void clear() threadsafe { return concurrent_queue_base::clear(); }
	bool empty() const threadsafe { return oThreadsafe(this)->queue_t::empty(); }
	size_type size() const { return queue_t::unsafe_size(); }
};

void TESTconcurrent_queue_tbb()
{
	oTEST_QUEUET(concurrent_queue_tbb);
}
#else
void TESTconcurrent_queue_tbb()
{
	oTRACE("oErrorSetLast(std::errc::permission_denied, \"concurrent_queue_tbb test not enabled\");");
}
#endif

	} // namespace tests
} // namespace oConcurrency
