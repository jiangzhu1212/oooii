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
// A thread pool that mainly uses an underlying task group as specified in TBB
// and PPL: basically providing the following APIs:
//   void run(const oTASK& _Task);
//   void wait();
// This is primarily intended to ease integration of 3rd-party concurrency
// solutions that support waitable tasks/task groups.
#pragma once
#ifndef oConcurrency_task_group_threadpool_h
#define oConcurrency_task_group_threadpool_h

#include <oConcurrency/joinable_threadpool_base.h>

namespace oConcurrency {

template<typename TaskGroupT>
class task_group_threadpool : public joinable_threadpool_base
{
public:
	typedef TaskGroupT task_group_type;

	// The task will execute on any given worker thread. There is no order-of-
	// execution guarantee.
	void dispatch(const oTASK& _Task) threadsafe;

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
void task_group_threadpool<TaskGroupT>::dispatch(const oTASK& _Task) threadsafe
{ 
	begin_dispatch();
	enable_leak_tracking_threadlocal(false);
	oThreadsafe(TaskGroup).run(_Task);
	enable_leak_tracking_threadlocal(true);
	end_dispatch();
}

template<typename TaskGroupT>
void task_group_threadpool<TaskGroupT>::flush() threadsafe
{ 
	begin_flush();
	oThreadsafe(TaskGroup).wait();
	end_flush();
}

template<typename TaskGroupT>
void task_group_threadpool<TaskGroupT>::join() threadsafe
{ 
	joinable_threadpool_base::join();
	oThreadsafe(TaskGroup).wait();
}

} // namespace oConcurrency

#endif
