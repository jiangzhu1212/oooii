/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
// A dispatch queue executes tasks in the order they are enqueued.
#ifndef oDispatchQueue_h
#define oDispatchQueue_h

#include <oBasis/oCallable.h>
#include <oBasis/oFunction.h>
#include <oBasis/oInterface.h>

interface oDispatchQueue : oInterface
{
	// Schedule a task to be executed in the order they were enqueued. Ensure 
	// client code respects the requirements of the underlying implementation 
	// because tasks may not always execute on the same thread. This returns true
	// if the task was scheduled for execution or executed locally (if the 
	// underlying scheduler deemed that appropriate). This returns false if the
	// task has not nor will ever execute because either the underlying 
	// scheduler's queue is at capacity or the object is in a shutdown mode where
	// it is ignoring new requests.
	virtual bool Dispatch(oTASK _Task) threadsafe = 0;
	oDEFINE_CALLABLE_RETURN_WRAPPERS(bool, Dispatch, threadsafe, Dispatch);

	// Block the calling thread until all currently queued tasks are executed
	virtual void Flush() threadsafe = 0;

	// Returns true if new tasks can be enqueued to the list or if the execution
	// thread is in the process of flushing
	virtual bool Joinable() const threadsafe = 0;

	// Flush any currently queued tasks and then join the execution thread. Like
	// std::thread, this must be called before the dtor, or std::terminate will
	// be called in the dtor and once joined this dispatch queue is no longer 
	// usable.
	virtual void Join() threadsafe = 0;

	virtual const char* GetDebugName() const threadsafe = 0;
};

#endif
