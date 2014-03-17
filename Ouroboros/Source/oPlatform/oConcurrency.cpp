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

#include <oBasis/oDispatchQueueConcurrentT.h>

#undef interface
#undef INTERFACE_DEFINED

#include <tbb/tbb.h>
#include <tbb/task_scheduler_init.h>

class joinable_threadpool_base
{
	joinable_threadpool_base(const joinable_threadpool_base&); /* = delete */
	const joinable_threadpool_base& operator=(const joinable_threadpool_base&); /* = delete */

	joinable_threadpool_base(joinable_threadpool_base&&); /* = delete */
	joinable_threadpool_base& operator=(joinable_threadpool_base&&); /* = delete */

public:
	// Returns whether this is joinable_threadpool_base or not.
	bool joinable() const { return State != JOINED; }

protected:
	joinable_threadpool_base() : State(JOINABLE) {}

	// Lock around the underlying dispatch/execute/kickoff call, and only issue
	// if begin_dispatch doesn't throw an exception. Remember a join will 
	// invalidate the thread pool like a std::thread's join.
	void begin_dispatch()
	{
		if (State != JOINABLE || !Mutex.try_lock_shared())
			throw std::out_of_range("threadpool call after join");
	}

	// Call this after calling the underlying dispatch call.
	void end_dispatch() { Mutex.unlock_shared(); }

	// Call this as the first line of the flush implementation to ensure new 
	// dispatches are locked out.
	void begin_flush() { Mutex.lock(); State = FLUSHING; }

	// Call this once the schedule is empty to resume a valid dispatch state.
	void end_flush() { Mutex.unlock(); State = JOINABLE; }

	// Call this as the first line of the join implementation to ensure new 
	// dispatches are locked out.
	void join() { std::lock_guard<ouro::shared_mutex> lock(Mutex); State = JOINED; }

private:
	ouro::shared_mutex Mutex;
	enum STATE { JOINABLE, FLUSHING, JOINED, };
	STATE State;
};


template<typename TaskGroupT>
class task_group_threadpool : public joinable_threadpool_base
{
public:
	typedef TaskGroupT task_group_type;

	// The task will execute on any given worker thread. There is no order-of-
	// execution guarantee.
	void dispatch(const std::function<void()>& _Task) threadsafe;

	// Block until all workers are idle.
	void flush() threadsafe;

	// defined by base class
	// bool joinable() const threadsafe;

	// Blocks until all workers are joined.
	void join() threadsafe;

private:
	task_group_type TaskGroup;
};

template<typename TaskGroupT>
void task_group_threadpool<TaskGroupT>::dispatch(const std::function<void()>& _Task) threadsafe
{ 
	oThreadsafe(this)->begin_dispatch();
	oThreadsafe(TaskGroup).run(_Task);
	oThreadsafe(this)->end_dispatch();
}

template<typename TaskGroupT>
void task_group_threadpool<TaskGroupT>::flush() threadsafe
{ 
	oThreadsafe(this)->begin_flush();
	oThreadsafe(TaskGroup).wait();
	oThreadsafe(this)->end_flush();
}

template<typename TaskGroupT>
void task_group_threadpool<TaskGroupT>::join() threadsafe
{ 
	oThreadsafe(this)->joinable_threadpool_base::join();
	oThreadsafe(TaskGroup).wait();
}


using namespace tbb;
typedef task_group task_group_t;

typedef task_group_threadpool<task_group> threadpool_t;

const oGUID& oGetGUID(threadsafe const oDispatchQueueConcurrentT<threadpool_t>* threadsafe const*)
{
	// {d912957c-a621-4960-833f-55572a1a4abb}
	static const oGUID IID_oDispatchQueueConcurrentT = { 0xd912957c, 0xa621, 0x4960, { 0x83, 0x3f, 0x55, 0x57, 0x2a, 0x1a, 0x4a, 0xbb } };
	return IID_oDispatchQueueConcurrentT;
}

class task_group_impl : public ouro::task_group
{
	task_group_t g;
public:
	#if oHAS_oCONCURRENCY
		task_group_impl() : oConcurrency::detail::task_group<oScheduler::allocator_type>(oScheduler::Singleton()->Threadpool) {}
	#endif
	void run(const std::function<void()>& _Task) override { g.run(_Task); }
	void wait() override { g.wait(); }
	~task_group_impl() { wait(); }
};

bool oDispatchQueueCreateConcurrent(const char* _DebugName, size_t _InitialTaskCapacity, threadsafe oDispatchQueueConcurrent** _ppQueue)
{
	bool success = false;
	oCONSTRUCT(_ppQueue, oDispatchQueueConcurrentT<threadpool_t>(_DebugName, &success));
	return success;
}
