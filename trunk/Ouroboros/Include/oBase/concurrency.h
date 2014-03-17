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
// Concurrent interfaces. These are not implemented in oBase but are used by 
// oBase implementations since concurrency is such a big performance win and 
// should be provided by the C++1x standard. Until then, client code must 
// implement these interfaces.
#pragma once
#ifndef oBase_concurrency_h
#define oBase_concurrency_h

#include <functional>
#include <memory>

namespace ouro {

// This function should be implemented for queue types since some "queues" have 
// valid reasons for being LIFOs.
template<typename T> struct is_fifo : std::true_type {};

// defines a subset of functions that can be waited on.
class task_group
{
public:
	virtual ~task_group() {}

	// dispatches a task flagged as part of this group
	virtual void run(const std::function<void()>& _Task) = 0;

	// waits for only tasks dispatched with run() to finish
	virtual void wait() = 0;

	// cancels all queued tasks (executed tasks will have already been completed). 
	// This only flags tasks not to execution, but the threadpool still needs to 
	// process them before wait() will be unblocked to guarantee any resources 
	// bound in the run task are released.
	virtual void cancel() = 0;

	// returns true if cancel was called, but wait has not yet finished (or has 
	// not yet been called)
	virtual bool is_canceling() = 0;
};

// factory function for creating a task_group
std::shared_ptr<task_group> make_task_group();

// Returns the name of the current job scheduler
const char* scheduler_name();

// Ensures all initial allocations are done and the system is ready.
void ensure_scheduler_initialized();

// issues a task for asynchronous execution on any thread in the underlying
// threadpool.
void dispatch(const std::function<void()>& _Task);

// Implements the parallel for pattern, executing the specified task with the index
// from _Begin to _End.
void parallel_for(size_t _Begin, size_t _End, const std::function<void(size_t _Index)>& _Task);

// Runs a task when the calling thread exits.
void at_thread_exit(const std::function<void()>& _Task);

} // namespace ouro

#endif
