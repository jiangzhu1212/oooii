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
// A simple thread pool that runs tasks on any thread at any time (no order 
// guarantees). Most likely client code should prefer parallel_for over using 
// this simple thread pool directly. This is a custom implementation that has
// 3 tiers of execution: 1. Look to a thread-local queue for work, 2. Look to a 
// global queue for work and 3. Look at other workers and steal work if 
// available. The intent here is not to replace robust solutions like TBB, but
// rather aid in platform porting by providing a semantically similar, simpler
// version.
#pragma once
#ifndef oConcurrency_threadpool_h
#define oConcurrency_threadpool_h

#include <oConcurrency/oConcurrency.h>
#include <oConcurrency/basic_threadpool.h>
#include <oConcurrency/concurrent_worklist.h>
#include <oConcurrency/countdown_latch.h>
#include <oConcurrency/thread_safe.h>
#include <oBase/backoff.h>
#include <stdexcept>
#include <vector>

namespace oConcurrency {

namespace detail { template<typename Alloc> class task_group; }

template<typename Alloc>
class threadpool : public basic_threadpool_base<Alloc>
{
	/** <citation
		usage="Paper" 
		reason="Synchronized basic_threadpool is slow" 
		author="Joe Duffy"
		description="http://www.bluebytesoftware.com/blog/2008/09/17/BuildingACustomThreadPoolSeriesPart3IncorporatingWorkStealingQueues.aspx"
		modifications="C++ version from the paper's C#; share common code with basic_threadpool_base."
	/>*/

public:
	typedef typename basic_threadpool_base<Alloc>::allocator_type allocator_type;

	// Pass 0 to allocate a worker thread for each hardware process found.
	threadpool(size_t _NumWorkers = 0, const allocator_type& _Alloc = allocator_type());

	// The task will execute on any given worker thread. There is no order-of-
	// execution guarantee.
	void dispatch(const oTASK& _Task) threadsafe;

protected:
	template<typename Alloc> friend class detail::task_group;

	mutex LocalQueuesMutex;
	std::vector<concurrent_worklist<oTASK, allocator_type>*> LocalQueues;
	std::vector<oStd::thread::id> WorkerIDs;
	static thread_local concurrent_worklist<oTASK, allocator_type>* pLocalQueue;

	// @oooii-tony: Due to a bug in VS2010, instead of declaring work() with a 
	// task group as a parameter and passing null for worker threads and this for
	// task-stealing task groups, I had to work around the issue by setting up a 
	// 'global' variable. The bug has to do with std::bind and seems to affect
	// 2 or more parameters bound by functions, be they methods of functions. 
	// Revisit this in VS11.
	static thread_local threadsafe detail::task_group<allocator_type>* pTaskGroup;

	void worker_proc();
	void work();

	// If threadpool is initialized in one module and used in another, the TLS may
	// not be initialized to the same thing, so fix it.
	void patch_local_queue();

	threadpool(const threadpool&); /* = delete */
	const threadpool& operator=(const threadpool&); /* = delete */

	void begin();
	void end();
};

template<typename Alloc> thread_local concurrent_worklist<oTASK, Alloc>* threadpool<Alloc>::pLocalQueue;
template<typename Alloc> thread_local threadsafe oConcurrency::detail::task_group<Alloc>* oConcurrency::threadpool<Alloc>::pTaskGroup;

template<typename Alloc>
threadpool<Alloc>::threadpool(size_t _NumWorkers, const allocator_type& _Alloc)
{
	LocalQueues.resize(calc_num_workers(_NumWorkers));
	WorkerIDs.resize(LocalQueues.size());
	construct_workers(std::move(oBIND(&threadpool::worker_proc, this)), _NumWorkers);
}

template<typename Alloc>
void threadpool<Alloc>::begin()
{
	// assign pLocalQueue as last step otherwise the main proc loop needs to check
	// for a separate initialized state rather than use its access to the local
	// queue as the valid flag itself.

	begin_thread("threadpool Worker");
	auto pNewQueue = new concurrent_worklist<oTASK, allocator_type>(GlobalQueue.get_allocator());

	// don't use push_back so we can skip any validity checks when iterating 
	// through LocalQueues in the main proc loop.
	lock_guard<mutex> Lock(LocalQueuesMutex);
	for (size_t i = 0; i < LocalQueues.size(); i++)
	{
		if (!LocalQueues[i])
		{
			WorkerIDs[i] = oStd::this_thread::get_id();
			LocalQueues[i] = pNewQueue;
			pLocalQueue = pNewQueue;
			break;
		}
	}
}

template<typename Alloc>
void threadpool<Alloc>::end()
{
	// null pointer immediately so its validity can be determined more easily in
	// the main proc loop.

	LocalQueuesMutex.lock();
	auto p = pLocalQueue;
	pLocalQueue = nullptr;

	for (auto q = std::begin(LocalQueues); q != std::end(LocalQueues); ++q)
	{
		if (*q == p)
		{
			*q = nullptr;
			break;
		}
	}
	LocalQueuesMutex.unlock();
	delete p;
	end_thread();
}

template<typename Alloc>
void threadpool<Alloc>::dispatch(const oTASK& _Task) threadsafe
{
	if (Running)
	{
		if (pLocalQueue)
		{
			pLocalQueue->push_local(_Task);
			if (NumWorking == 0)
			{
				unique_lock<mutex> Lock(Mutex);
				WorkAvailable.notify_one();
			}
		}

		else
		{
			unique_lock<mutex> Lock(Mutex);
			oThreadsafe(GlobalQueue).push_back(_Task);
			if (NumWorking == 0)
				WorkAvailable.notify_one();
		}
	}

	else
		throw threadpool_error(threadpool_errc::call_after_join);
}

template<typename Alloc>
void threadpool<Alloc>::worker_proc()
{
	begin();
	work();
	end();
}

template<typename Alloc>
void threadpool<Alloc>::work()
{
	// NOTE: pLocalQueue is always guaranteed to be valid even across modules 
	// because initialization of the worker thread and thus the pLocalQueue's TLS 
	// happens at the same time this function is bound to the worker thread, so it
	// all happens at the same time and no one else calls this function... except
	// task_group, so see that for where protection against TLS's-per-module
	// happens.

	while (!pTaskGroup || pTaskGroup->Latch.outstanding() > 1)
	{
		// 1. Check local queue
		// 2. Check global queue
		// 3. Steal from another queue

		oTASK task;
		if (!pLocalQueue->try_pop_local(task))
		{
			bool AlreadyTriedStealing = false;
			while (true)
			{
				{
					unique_lock<mutex> Lock(Mutex);

					if (!Running && pLocalQueue->empty() && GlobalQueue.empty())
						return;

					if (!GlobalQueue.empty())
					{
						task = std::move(GlobalQueue.front());
						GlobalQueue.pop_front();
						break;
					}

					else if (AlreadyTriedStealing)
					{
						// don't block the thread if this is being called from a work-
						// stealing task_group.
						if (pTaskGroup)
							return;

						NumWorking--;
						WorkAvailable.wait(Lock);
						NumWorking++;

						if (!Running && pLocalQueue->empty() && GlobalQueue.empty())
							return;

						AlreadyTriedStealing = false;
						continue;
					}
				}

				bool StoleTask = false;

				// oFOR macro usage here confused release builds, so this first break 
				// was skipping the while loop, NOT the for loop just here, so it 
				// would skip actually running the task. Leave this for loop as-is.
				for (auto q = std::begin(LocalQueues); q != std::end(LocalQueues); ++q)
				{ 
					if (*q && *q != pLocalQueue && (*q)->try_steal_for(task, oStd::chrono::milliseconds(200)))
					{
						StoleTask = true;
						break;
					}
				}

				if (StoleTask)
					break;

				AlreadyTriedStealing = true;
			}
		}

		task();
	}
}

template<typename Alloc>
void threadpool<Alloc>::patch_local_queue()
{
	for (size_t i = 0; i < WorkerIDs.size(); i++)
	{
		if (WorkerIDs[i] == oStd::this_thread::get_id())
		{
			pLocalQueue = LocalQueues[i];
			return;
		}
	}
}

namespace detail {

template<typename Alloc>
class task_group
{
public:
	typedef Alloc allocator_type;

	// The challenge is that a task group must have access to its threadpool 
	// because it may be running tasks from non-worker threads, so even a 
	// thread_local pointer kept to the thread pool will not help. TBB and PPL 
	// assume a singleton scheduler, but this implementation does not and leaves
	// it explicit.
	task_group(threadpool<allocator_type>& _Threadpool);

	// Run a task as part of this task group
	void run(const oTASK& _Task) threadsafe;

	// blocks until all tasks associated with this task group are finished. While
	// waiting, this work steals.
	void wait() threadsafe;

protected:
	threadpool<allocator_type>& Threadpool;
	countdown_latch Latch;
	friend threadpool<allocator_type>;
};

template<typename Alloc>
task_group<Alloc>::task_group(threadpool<Alloc>& _Threadpool)
	: Threadpool(_Threadpool)
	, Latch(1)
{
	// Fix if we're being called for the first time from another module
	if (!Threadpool.pLocalQueue)
		Threadpool.patch_local_queue();
}

template<typename Alloc>
void task_group<Alloc>::run(const oTASK& _Task) threadsafe
{
	Latch.reference();
	Threadpool.dispatch([&,_Task] { _Task(); Latch.release(); });
}

template<typename Alloc>
void task_group<Alloc>::wait() threadsafe
{
	// only block the calling thread if it's not a worker. If a worker, work
	if (!Threadpool.pLocalQueue)
	{
		Latch.release();
		Latch.wait();
		Latch.reset(1); // reset to initial state so the task group can be reused
	}

	else
	{
		threadsafe task_group<Alloc>* pOldTaskGroup = Threadpool.pTaskGroup;
		Threadpool.pTaskGroup = this;
		Threadpool.work();
		Threadpool.pTaskGroup = pOldTaskGroup;
		Latch.release();
	}
}

template<typename Alloc>
inline void thread_local_parallel_for(task_group<Alloc>& _TaskGroup, size_t _Begin, size_t _End, oINDEXED_TASK _Task)
{
	for (; _Begin < _End; _Begin++)
		_TaskGroup.run(oBIND(_Task, _Begin));
}

template<size_t WorkChunkSize /* = 16*/, typename Alloc>
inline void parallel_for(threadpool<Alloc>& _Threadpool, size_t _Begin, size_t _End, const oINDEXED_TASK& _Task)
{
	task_group<Alloc> g(_Threadpool);
	const size_t kNumSteps = (_End - _Begin) / WorkChunkSize;
	for (size_t i = 0; i < kNumSteps; i++, _Begin += WorkChunkSize)
		g.run(oBIND(thread_local_parallel_for<Alloc>, oBINDREF(g), _Begin, _Begin + WorkChunkSize, _Task));

	if (_Begin < _End)
		g.run(oBIND(thread_local_parallel_for<Alloc>, oBINDREF(g), _Begin, _End, _Task));

	g.wait();
}

} // namespace detail

} // namespace oConcurrency

#endif
