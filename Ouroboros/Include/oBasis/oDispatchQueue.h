// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// A dispatch queue executes tasks in the order they are enqueued.
#ifndef oDispatchQueue_h
#define oDispatchQueue_h

#include <oBasis/oInterface.h>
#include <oBase/callable.h>
#include <functional>

typedef std::function<void()> oTASK;
typedef std::function<void(size_t _Index)> oINDEXED_TASK;

// {85260463-6AA5-4BAB-951F-E1B044E9F692}
oDEFINE_GUID_I(oDispatchQueue, 0x85260463, 0x6aa5, 0x4bab, 0x95, 0x1f, 0xe1, 0xb0, 0x44, 0xe9, 0xf6, 0x92);
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
	virtual bool Dispatch(const oTASK& _Task) threadsafe = 0;
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
