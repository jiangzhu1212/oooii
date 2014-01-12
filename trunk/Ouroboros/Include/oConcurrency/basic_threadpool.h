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
// guarantees). Most likely client code should prefer parallel_for 
// over using this basic thread pool directly. This is a custom implementation 
// using one queue to feed worker threads and exists primarily for easy porting 
// and to benchmark other thread pool implementations.
#pragma once
#ifndef oConcurrency_basic_threadpool_h
#define oConcurrency_basic_threadpool_h

#include <oConcurrency/oConcurrency.h>
#include <oConcurrency/condition_variable.h>
#include <oConcurrency/mutex.h>
#include <oConcurrency/thread_safe.h>
#include <oBase/backoff.h>
#include <oStd/for.h>
#include <deque>
#include <exception>
#include <vector>

namespace oConcurrency {

template<typename Alloc>
class basic_threadpool_base
{
public:
	typedef oTASK task_type;
	typedef Alloc allocator_type;

	// Call construct_workers in most-derived ctor to initialize values.
	basic_threadpool_base(const allocator_type& _Alloc = allocator_type());

	// Calls std::terminate() if join() wasn't explicitly called in client code,
	// the same as the behavior of std::thread.
	virtual ~basic_threadpool_base();

	// Block until all workers are idle.
	void flush() threadsafe;

	// Returns true if the thread pool can still be joined.
	bool joinable() const threadsafe;

	// Blocks until all workers are joined.
	void join() threadsafe;

protected:
	std::vector<oStd::thread> Workers;
	std::deque<task_type, allocator_type> GlobalQueue;
	mutex Mutex;
	condition_variable WorkAvailable;
	size_t NumWorking;
	bool Running;

	// Returns hardware_concurrency() if the specified number of workers is zero.
	size_t calc_num_workers(size_t _NumWorkersRequested) const;

	// To ensure all members are constructed at time of worker instantiation, 
	// separate out a call to be called from the most-derived constructor.
	void construct_workers(const task_type& _DoWork, size_t _NumWorkers = 0);

	basic_threadpool_base(const basic_threadpool_base&); /* = delete */
	const basic_threadpool_base& operator=(const basic_threadpool_base&); /* = delete */

	basic_threadpool_base(basic_threadpool_base&&); /* = delete */
	basic_threadpool_base& operator=(basic_threadpool_base&&); /* = delete */
};

template<typename Alloc>
size_t basic_threadpool_base<Alloc>::calc_num_workers(size_t _NumWorkersRequested) const
{
	return _NumWorkersRequested ? _NumWorkersRequested : oStd::thread::hardware_concurrency();
}

template<typename Alloc>
inline void basic_threadpool_base<Alloc>::construct_workers(const task_type& _DoWork, size_t _NumWorkers)
{
	NumWorking = calc_num_workers(_NumWorkers);
	Workers.resize(NumWorking);
	oFOR(auto& w, Workers)
		w = oStd::thread(_DoWork);

	// wait until all have settled and thus ensure all have initialized
	flush();
}

template<typename Alloc>
inline basic_threadpool_base<Alloc>::basic_threadpool_base(const allocator_type& _Alloc)
	: GlobalQueue(_Alloc)
	, NumWorking(0)
	, Running(true)
{}

template<typename Alloc>
inline basic_threadpool_base<Alloc>::~basic_threadpool_base()
{
	if (joinable())
		std::terminate();
}

template<typename Alloc>
inline void basic_threadpool_base<Alloc>::flush() threadsafe
{
	ouro::backoff bo;
	while (Running)
	{
		// GlobalQueue.empty() is true before Task() is done, so don't use queue 
		// emptiness as an indicator of flushed-ness.
		if (oThreadsafe(GlobalQueue).empty() && NumWorking == 0)
			break;
		bo.pause();
	}
}

template<typename Alloc>
inline bool basic_threadpool_base<Alloc>::joinable() const threadsafe
{
	return Running && oThreadsafe(Workers).front().joinable();
}

template<typename Alloc>
inline void basic_threadpool_base<Alloc>::join() threadsafe
{
	unique_lock<mutex> Lock(Mutex);
	Running = false;
	WorkAvailable.notify_all();
	Lock.unlock();
	oFOR(auto& w, oThreadsafe(Workers))
		w.join();
}

template<typename Alloc = std::allocator<TaskT>>
class basic_threadpool : public basic_threadpool_base<Alloc>
{
public:
	typedef typename basic_threadpool_base<Alloc>::task_type task_type;
	typedef typename basic_threadpool_base<Alloc>::allocator_type allocator_type;

	// Pass 0 to allocate a worker thread for each hardware process found.
	basic_threadpool(size_t _NumWorkers = 0, const allocator_type& _Alloc = allocator_type());

	// The task will execute on any given worker thread. There is no order-of-
	// execution guarantee.
	void dispatch(const task_type& _Task) threadsafe;

private:
	void work();

	basic_threadpool(const basic_threadpool&); /* = delete */
	const basic_threadpool& operator=(const basic_threadpool&); /* = delete */

	basic_threadpool(basic_threadpool&&); /* = delete */
	basic_threadpool& operator=(basic_threadpool&&); /* = delete */
};

template<typename Alloc>
inline basic_threadpool<Alloc>::basic_threadpool(size_t _NumWorkers, const allocator_type& _Alloc)
	: basic_threadpool_base(_Alloc)
{
	construct_workers(std::bind(&basic_threadpool::work, this), _NumWorkers);
}

template<typename Alloc>
inline void basic_threadpool<Alloc>::dispatch(const task_type& _Task) threadsafe
{
	if (Running)
	{
		unique_lock<mutex> Lock(Mutex);
		oThreadsafe(GlobalQueue).push_back(_Task);
		if (NumWorking == 0)
			WorkAvailable.notify_one();
	}

	else
		throw std::out_of_range("threadpool call after join");
}

template<typename Alloc>
inline void basic_threadpool<Alloc>::work()
{
	begin_thread("basic_threadpool Worker");
	while (true)
	{
		unique_lock<mutex> Lock(Mutex);

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
	}
	end_thread();
}

} // namespace oConcurrency

#endif
