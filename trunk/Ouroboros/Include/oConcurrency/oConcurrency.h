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
// Interfaces for concurrent programming. NOTE: These declarations and this 
// header exist at a very low level. Generic code and algorithms can leverage
// concurrent techniques for performance and that idea is something that should
// be encouraged. Alas efficient and robust concurrency handling remains very
// platform-specific, so declare a minimal API and allow usage in generic 
// libraries and leave the implementation to be downstream somewhere, so expect
// link errors unless there is middleware or other platform implementation to 
// implement these interfaces.
#pragma once
#ifndef oConcurrencyRequirements_h
#define oConcurrencyRequirements_h

#include <oStd/callable.h>
#include <oConcurrency/thread_safe.h>
#include <memory>
#include <system_error>

typedef std::function<void()> oTASK;
typedef std::function<void(size_t _Index)> oINDEXED_TASK;

namespace oConcurrency {

// This function should be implemented for queue types since some "queues" have 
// valid reasons for being LIFOs.
template<typename T> struct is_fifo : std::true_type {};

// _____________________________________________________________________________
// Platform requirements

class task_group
{
public:
	// dispatches the task while flagging it as part of this task group.
	virtual void run(const oTASK& _Task) = 0;

	// blocks until all dispatches associated with this task_group are complete.
	// In this way one scheduler can be used while dependencies on only a subset
	// of the work can be described and flushed.
	virtual void wait() = 0;
};

// Initialize the global task scheduler and its memory. Some schedules allocate
// memory that could be detected as a leak. Exposing this API allows client code
// to allocate that memory at a known time so that a leak tracker can be 
// disabled.
void init_task_scheduler();

// Returns the name of the middleware or algorithm used to implement dispatch()
// and parallel_for().
const char* task_scheduler_name();

// Inserts a task to be executed by the task scheduler. Tasks should be non-
// blocking (no slow file io) and synchronization should be kept to a minimum.
// There are no guarantees for order of execution or when the specified task is
// finished.
void dispatch(const std::function<void()>& _Task);

// Runs the specified task on [_Begin, _End), passing the index of the counter 
// to the client code task.
void parallel_for(size_t _Begin, size_t _End, const std::function<void(size_t _Index)>& _Task);

// Registers the specified function to be run just before the current thread 
// exits. More than one function can be registered and each will be executed in 
// order of registration.
void thread_at_exit(const std::function<void()>& _AtExit);
oDEFINE_CALLABLE_WRAPPERS(thread_at_exit,, thread_at_exit);

// _____________________________________________________________________________
// Basic utilities

// Runs the specified task (a drop-in debug replacement for oConcurrency::dispatch)
inline void dispatch_serial(const std::function<void()>& _Task) { _Task(); }

inline void serial_for(size_t _Begin, size_t _End, const std::function<void(size_t _Index)>& _Task)
{
	for (size_t i = _Begin; i < _End; i++)
		_Task(i);
}

} // namespace oConcurrency

#endif
