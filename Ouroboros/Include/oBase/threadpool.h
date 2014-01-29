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
// A trivial thread pool that runs tasks on any thread at any time (no order 
// guarantees).
#pragma once
#ifndef oBase_threadpool_h
#define oBase_threadpool_h

#include <oBase/backoff.h>
#include <oBase/countdown_latch.h>
#include <condition_variable>
#include <deque>
#include <exception>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace ouro {

struct threadpool_default_traits
{
	// called once before worker thread main loop
	static void begin_thread(const char* _ThreadName) {}

	// called after each call to a dispatched user task()
	// (intended for RCU callouts)
	static void update_thread() {}

	// called once after worker thread main loop exits
	static void end_thread() {}
};

namespace detail { template<typename Traits, typename Alloc = std::allocator<std::function<void()>>> class task_group; }

template<typename Alloc>
class threadpool_base
{
public:
	typedef std::function<void()> task_type;
	typedef Alloc allocator_type;

	// Call construct_workers in most-derived ctor to initialize values.
	threadpool_base(const allocator_type& _Alloc = allocator_type());

	// Calls std::terminate() if join() wasn't explicitly called in client code,
	// the same as the behavior of std::thread.
	virtual ~threadpool_base();

	// Block until all workers are idle.
	void flush();

	// Returns true if the thread pool can still be joined.
	bool joinable() const;

	// Blocks until all workers are joined.
	void join();

protected:
	template<typename, typename> friend class detail::task_group;
	std::vector<std::thread> Workers;
	std::deque<task_type, allocator_type> GlobalQueue;
	std::mutex Mutex;
	std::condition_variable WorkAvailable;
	size_t NumWorking;
	bool Running;

	// Returns hardware_concurrency() if the specified number of workers is zero.
	size_t calc_num_workers(size_t _NumWorkersRequested) const;

	// To ensure all members are constructed at time of worker instantiation, 
	// separate out a call to be called from the most-derived constructor.
	void construct_workers(const task_type& _DoWork, size_t _NumWorkers = 0);

	threadpool_base(const threadpool_base&); /* = delete */
	const threadpool_base& operator=(const threadpool_base&); /* = delete */

	threadpool_base(threadpool_base&&); /* = delete */
	threadpool_base& operator=(threadpool_base&&); /* = delete */
};

template<typename Alloc>
size_t threadpool_base<Alloc>::calc_num_workers(size_t _NumWorkersRequested) const
{
	return _NumWorkersRequested ? _NumWorkersRequested : std::thread::hardware_concurrency();
}

template<typename Alloc>
inline void threadpool_base<Alloc>::construct_workers(const task_type& _DoWork, size_t _NumWorkers)
{
	NumWorking = calc_num_workers(_NumWorkers);
	Workers.resize(NumWorking);
	for (auto& w : Workers)
		w = std::thread(_DoWork);
	flush(); // wait until all have settled and thus ensure all have initialized
}

template<typename Alloc>
inline threadpool_base<Alloc>::threadpool_base(const allocator_type& _Alloc)
	: GlobalQueue(_Alloc)
	, NumWorking(0)
	, Running(true)
{}

template<typename Alloc>
inline threadpool_base<Alloc>::~threadpool_base()
{
	if (joinable())
		std::terminate();
}

template<typename Alloc>
inline void threadpool_base<Alloc>::flush()
{
	ouro::backoff bo;
	while (Running)
	{
		// GlobalQueue.empty() is true before Task() is done, so don't use queue 
		// emptiness as an indicator of flushed-ness.
		if (GlobalQueue.empty() && NumWorking == 0)
			break;
		bo.pause();
	}
}

template<typename Alloc>
inline bool threadpool_base<Alloc>::joinable() const
{
	return Running && Workers.front().joinable();
}

template<typename Alloc>
inline void threadpool_base<Alloc>::join()
{
	std::unique_lock<std::mutex> Lock(Mutex);
	Running = false;
	WorkAvailable.notify_all();
	Lock.unlock();
	for (auto& w : Workers)
		w.join();
}

template<typename Traits, typename Alloc = std::allocator<std::function<void()>>>
class threadpool : public threadpool_base<Alloc>
{
public:
	typedef typename threadpool_base<Alloc>::task_type task_type;
	typedef typename threadpool_base<Alloc>::allocator_type allocator_type;

	// Pass 0 to allocate a worker thread for each hardware process found.
	threadpool(size_t _NumWorkers = 0, const allocator_type& _Alloc = allocator_type());

	// The task will execute on any given worker thread. There is no order-of-
	// execution guarantee.
	void dispatch(const task_type& _Task);

private:
	void work();

	threadpool(const threadpool&); /* = delete */
	const threadpool& operator=(const threadpool&); /* = delete */

	threadpool(threadpool&&); /* = delete */
	threadpool& operator=(threadpool&&); /* = delete */
};

template<typename Traits, typename Alloc>
inline threadpool<Traits, Alloc>::threadpool(size_t _NumWorkers, const allocator_type& _Alloc)
	: threadpool_base(_Alloc)
{
	construct_workers(std::bind(&threadpool::work, this), _NumWorkers);
}

template<typename Traits, typename Alloc>
inline void threadpool<Traits, Alloc>::dispatch(const task_type& _Task)
{
	if (Running)
	{
		std::unique_lock<std::mutex> Lock(Mutex);
		GlobalQueue.push_back(_Task);
		if (NumWorking == 0)
			WorkAvailable.notify_one();
	}

	else
		throw std::system_error(std::make_error_code(std::errc::invalid_argument), "dispatch called after join");
}

template<typename Traits, typename Alloc>
inline void threadpool<Traits, Alloc>::work()
{
	Traits::begin_thread("threadpool Worker");
	while (true)
	{
		std::unique_lock<std::mutex> Lock(Mutex);

		while (Running && GlobalQueue.empty())
		{
			NumWorking--;
			WorkAvailable.wait(Lock);
			NumWorking++;
		}

		if (!Running && GlobalQueue.empty())
			break;

		task_type task = std::move(GlobalQueue.front());
		GlobalQueue.pop_front();
		Lock.unlock();
		task();

		Traits::update_thread();
	}
	Traits::end_thread();
}

// task_group and parallel_for need to know the instance of a threadpool to work with, but 
// there should be only one threadpool in a system. oBase is not the place to solidify that
// policy, so provide the implementation here, but use it to implement a ouro-namespaced 
// task_group and parallel_for elsewhere.
	namespace detail {

template<typename Traits, typename Alloc>
class task_group
{
public:
	typedef Alloc allocator_type;
	typedef threadpool<Traits, Alloc> threadpool_type;
	task_group(threadpool_type& _Threadpool);

	// Run a task as part of this task group
	void run(const std::function<void()>& _Task);

	// blocks until all tasks associated with this task group are finished. While
	// waiting, this work steals.
	void wait();

private:
	threadpool_type& Threadpool;
	ouro::countdown_latch Latch;
};

template<typename Traits, typename Alloc>
task_group<Traits, Alloc>::task_group(threadpool_type& _Threadpool)
	: Threadpool(_Threadpool)
	, Latch(1)
{}

template<typename Traits, typename Alloc>
void task_group<Traits, Alloc>::run(const std::function<void()>& _Task)
{
	Latch.reference();
	Threadpool.dispatch([&,_Task] { _Task(); Latch.release(); });
}

template<typename Traits, typename Alloc>
void task_group<Traits, Alloc>::wait()
{
	// should this do a NumWorking++ somewhere?

	Latch.release();
	while (Latch.outstanding())
	{
		std::unique_lock<std::mutex> Lock(Threadpool.Mutex);

		if (!Threadpool.Running)
			throw std::system_error(std::make_error_code(std::errc::invalid_argument), "threadpool shut down before task group could complete");

		if (Threadpool.GlobalQueue.empty())
		{
			Lock.unlock();
			Latch.wait();
		}

		else
		{
			auto task = std::move(Threadpool.GlobalQueue.front());
			Threadpool.GlobalQueue.pop_front();
			Lock.unlock();
			task();
		}
	}

	Latch.reset(1); // allow task group to be resued
}

template<typename Traits, typename Alloc>
inline void thread_local_parallel_for(task_group<Traits, Alloc>& _TaskGroup, size_t _Begin, size_t _End, const std::function<void(size_t _Index)>& _Task)
{
	for (; _Begin < _End; _Begin++)
		_TaskGroup.run(std::bind(_Task, _Begin));
}

template<size_t WorkChunkSize /* = 16*/, typename Traits, typename Alloc>
inline void parallel_for(threadpool<Traits, Alloc>& _Threadpool, size_t _Begin, size_t _End, const std::function<void(size_t _Index)>& _Task)
{
	task_group<Traits, Alloc> g(_Threadpool);
	const size_t kNumSteps = (_End - _Begin) / WorkChunkSize;
	for (size_t i = 0; i < kNumSteps; i++, _Begin += WorkChunkSize)
		g.run(std::bind(thread_local_parallel_for<Traits, Alloc>, std::ref(g), _Begin, _Begin + WorkChunkSize, _Task));
	if (_Begin < _End)
		g.run(std::bind(thread_local_parallel_for<Traits, Alloc>, std::ref(g), _Begin, _End, _Task));
	g.wait();
}

	} // namespace detail

} // namespace ouro

#endif
