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
#include <oBase/backoff.h>
#include <oBase/countdown_latch.h>
#include <oStd/shared_mutex.h>
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
	void dispatch(const std::function<void()>& _Task);

protected:
	template<typename Alloc> friend class detail::task_group;

	std::mutex LocalQueuesMutex;
	ouro::shared_mutex WorkingMutex;
	std::vector<concurrent_worklist<std::function<void()>, allocator_type>*> LocalQueues;
	std::vector<std::thread::id> WorkerIDs;
	static thread_local concurrent_worklist<std::function<void()>, allocator_type>* pLocalQueue;

	// @tony: Due to a bug in VS2010, instead of declaring work() with a 
	// task group as a parameter and passing null for worker threads and this for
	// task-stealing task groups, I had to work around the issue by setting up a 
	// 'global' variable. The bug has to do with std::bind and seems to affect
	// 2 or more parameters bound by functions, be they methods of functions. 
	// Revisit this in VS11.
	static thread_local detail::task_group<allocator_type>* pTaskGroup;

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

template<typename Alloc> thread_local concurrent_worklist<std::function<void()>, Alloc>* threadpool<Alloc>::pLocalQueue;
template<typename Alloc> thread_local oConcurrency::detail::task_group<Alloc>* oConcurrency::threadpool<Alloc>::pTaskGroup;

template<typename Alloc>
threadpool<Alloc>::threadpool(size_t _NumWorkers, const allocator_type& _Alloc)
{
	LocalQueues.resize(calc_num_workers(_NumWorkers));
	WorkerIDs.resize(LocalQueues.size());
	construct_workers(std::bind(&threadpool::worker_proc, this), _NumWorkers);
}

template<typename Alloc>
void threadpool<Alloc>::begin()
{
	// assign pLocalQueue as last step otherwise the main proc loop needs to check
	// for a separate initialized state rather than use its access to the local
	// queue as the valid flag itself.

	begin_thread("threadpool Worker");
	auto pNewQueue = new concurrent_worklist<std::function<void()>, allocator_type>(GlobalQueue.get_allocator());

	// don't use push_back so we can skip any validity checks when iterating 
	// through LocalQueues in the main proc loop.
	std::lock_guard<std::mutex> Lock(LocalQueuesMutex);
	for (size_t i = 0; i < LocalQueues.size(); i++)
	{
		if (!LocalQueues[i])
		{
			WorkerIDs[i] = std::this_thread::get_id();
			LocalQueues[i] = pNewQueue;
			pLocalQueue = pNewQueue;
			break;
		}
	}

	WorkingMutex.lock_shared();
}

template<typename Alloc>
void threadpool<Alloc>::end()
{
	WorkingMutex.unlock_shared(); // release shared lock to indicate out of work
	WorkingMutex.lock(); // wait for all others to get out of work
	WorkingMutex.unlock(); // that's it - just needed to ensure no work stealing will occur during teardown

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
void threadpool<Alloc>::dispatch(const std::function<void()>& _Task)
{
	if (Running)
	{
		if (pLocalQueue)
		{
			pLocalQueue->push_local(_Task);
			if (NumWorking == 0)
			{
				std::unique_lock<std::mutex> Lock(Mutex);
				WorkAvailable.notify_one();
			}
		}

		else
		{
			std::unique_lock<std::mutex> Lock(Mutex);
			GlobalQueue.push_back(_Task);
			if (NumWorking == 0)
				WorkAvailable.notify_one();
		}
	}

	else
		throw std::out_of_range("threadpool call after join");
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

		std::function<void()> task;
		if (!pLocalQueue->try_pop_local(task))
		{
			bool AlreadyTriedStealing = false;
			while (true)
			{
				{
					std::unique_lock<std::mutex> Lock(Mutex);

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
					if (*q && *q != pLocalQueue && (*q)->try_steal_for(task, std::chrono::milliseconds(200)))
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
		if (WorkerIDs[i] == std::this_thread::get_id())
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
	void run(const std::function<void()>& _Task);

	// blocks until all tasks associated with this task group are finished. While
	// waiting, this work steals.
	void wait();

protected:
	threadpool<allocator_type>& Threadpool;
	ouro::countdown_latch Latch;
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
void task_group<Alloc>::run(const std::function<void()>& _Task)
{
	Latch.reference();
	Threadpool.dispatch([&,_Task] { _Task(); Latch.release(); });
}

template<typename Alloc>
void task_group<Alloc>::wait()
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
		task_group<Alloc>* pOldTaskGroup = Threadpool.pTaskGroup;
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
		_TaskGroup.run(std::bind(_Task, _Begin));
}

template<size_t WorkChunkSize /* = 16*/, typename Alloc>
inline void parallel_for(threadpool<Alloc>& _Threadpool, size_t _Begin, size_t _End, const oINDEXED_TASK& _Task)
{
	task_group<Alloc> g(_Threadpool);
	const size_t kNumSteps = (_End - _Begin) / WorkChunkSize;
	for (size_t i = 0; i < kNumSteps; i++, _Begin += WorkChunkSize)
		g.run(std::bind(thread_local_parallel_for<Alloc>, std::ref(g), _Begin, _Begin + WorkChunkSize, _Task));

	if (_Begin < _End)
		g.run(std::bind(thread_local_parallel_for<Alloc>, std::ref(g), _Begin, _End, _Task));

	g.wait();
}

} // namespace detail

} // namespace oConcurrency

#endif
