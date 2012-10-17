/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#pragma once
#ifndef oDispatchQueueGlobalT_h
#define oDispatchQueueGlobalT_h

#include <oBasis/oRef.h>
#include <oBasis/oRefCount.h>
#include <oBasis/oMutex.h>
#include <oBasis/oAssert.h>
#include <oBasis/oFixedString.h>
#include <oBasis/oInitOnce.h>
#include <oBasis/oThread.h>
#include <list>

//Template for creating dispatch queues that run in order, but each task may run on any thread including the caller of flush/join.
//	Type T must be non polymorphic or you will get a compile error
//	Type T must have a function " bool Init(oInterface* _pSelf)" an oInterface* to the actual instance will be passed, return a bool for success
//  Type T must have a function with this signature "void IssueImpl(oTASK&& _task) threadsafe". This should dispatch the task to an "async" operation
//	Type T mus implement ReferenceImpl and ReleaseImpl. similar to oInterface Reference adn Release, but release return a bool, true if this should be deleted, false otherwise
//	This class provides all the thread safety, you provide the actual async call, you do not need to add any additional thread safety to Issue though.
template<typename T>
struct oDispatchQueueGlobalT : public oDispatchQueueGlobal, T
{
	int Reference() threadsafe override
	{ 
		return ReferenceImpl();
	}

	void Release() threadsafe override
	{ 
		if(ReleaseImpl())
			delete this;
	}

	oDEFINE_NOOP_QUERYINTERFACE();

	oDispatchQueueGlobalT(const char* _DebugName, size_t _InitialTaskCapacity, bool* _pSuccess);
	~oDispatchQueueGlobalT();

	virtual bool Dispatch(oTASK _Task) threadsafe override;
	virtual void Flush() threadsafe override;
	virtual void Join() threadsafe override;
	virtual bool Joinable() const threadsafe override{ return IsJoinable; }
	virtual const char* GetDebugName() const threadsafe { return DebugName->c_str(); }

	void ExecuteNext(oRef<threadsafe oDispatchQueueGlobalT> _SelfRef, unsigned int _ExecuteKey) threadsafe;

	oInitOnce<oStringS> DebugName;
	typedef std::list<oTASK> tasks_t;
	tasks_t Tasks;
	oSharedMutex TaskLock;
	oSharedMutex FlushLock;
	bool IsJoinable;
	unsigned int ExecuteKey;

	unsigned int ExecuteThreadID;

	tasks_t& ProtectedTasks() threadsafe { return thread_cast<tasks_t&>(Tasks); }
};

template<typename T>
oDispatchQueueGlobalT<T>::oDispatchQueueGlobalT(const char* _DebugName, size_t _InitialTaskCapacity, bool* _pSuccess)
	: Tasks() // @oooii-tony: TODO _TaskCapacity cannot be done trivially with std::list/queue/deque... a new allocator needs to be made... so do that later.
	, DebugName(_DebugName)
	, IsJoinable(true)
	, ExecuteKey(0)
{
	static_assert(!std::is_polymorphic<T>::value, "Policy class given to oDispatchQueueGlobalT must be non polymorphic, want the vtable to contain oDispatchQueueGlobal interface only");
	*_pSuccess = Init(this);
}

template<typename T>
oDispatchQueueGlobalT<T>::~oDispatchQueueGlobalT()
{
	if (Joinable())
	{
		oASSERT(false, "Calling std::terminate because a Joinable oDispatchQueueGlobal was destroyed");
		std::terminate();
	}
}

template<typename T>
bool oDispatchQueueGlobalT<T>::Dispatch(oTASK _Task) threadsafe
{
	bool Scheduled = false;

	if (IsJoinable && FlushLock.try_lock_shared())
	{
		size_t TaskCount = 0;

		// Queue up command
		{
			oLockGuard<oSharedMutex> Lock(TaskLock);
			// Add this command to the queue
			ProtectedTasks().push_back(_Task);
			TaskCount = ProtectedTasks().size();
			Scheduled = true;
		}

		// If this command is the only one in the queue kick off the execution
		if (1 == TaskCount)
			Issue(oBIND(&oDispatchQueueGlobalT::ExecuteNext, this, oRef<threadsafe oDispatchQueueGlobalT>(this), ExecuteKey));

		FlushLock.unlock_shared();
	}

	return Scheduled;
};

template<typename T>
void oDispatchQueueGlobalT<T>::Flush() threadsafe
{
	Join();
	IsJoinable = true;
};

template<typename T>
void oDispatchQueueGlobalT<T>::Join() threadsafe
{
	IsJoinable = false;

	auto flushTasks = [&](){
		while (!ProtectedTasks().empty())
		{
			ProtectedTasks().front()();
			ProtectedTasks().pop_front();
		}
	};

	//Only way for this thread to be an ExecuteThread is if we got here by destroying a task in ExecuteNext
	//	and that task was the only thing directly or indirectly holding a reference to this queue.
	//	If that happens we will deadlock if we flush lock, and the lock isn't needed anyway. other threads
	//	will still lock on the flush lock, and we can just clear the remaining tasks ourselves.
	//	This condition happens when a task is holding an oref to an object that holds this queue, and is the
	//	last reference. then when the task is destroyed, the parent object is destroyed, and that can call join.
	if(ExecuteThreadID != oAsUint(oStd::this_thread::get_id()))
	{
		oLockGuard<oSharedMutex> Lock(FlushLock);
		flushTasks();
	}
	else
	{
		ProtectedTasks().pop_front(); //in this case there will be a garbage task at the front that needs to be removed. that task will be how we got here.
		flushTasks();
	}

	++ExecuteKey; // Increment the execute key which will prevent old tasks from executing
};

template<typename T>
void oDispatchQueueGlobalT<T>::ExecuteNext(oRef<threadsafe oDispatchQueueGlobalT> _SelfRef, unsigned int _ExecuteKey) threadsafe
{
	if (IsJoinable && _ExecuteKey == ExecuteKey && FlushLock.try_lock_shared())
	{
		ExecuteThreadID = oAsUint(oStd::this_thread::get_id());

		size_t TaskCount = 0;
		{
			//need to free the task before releasing the flush lock, and changing the size of the task queue
			//	in case the task itself is the only thing left keeping this queue alive, indirectly from a
			//	capture by value oref
			//	can't execute directly from the queue, as join could be called which modifies the queue, which would be bad.
			oTASK task;
			task = std::move(ProtectedTasks().front());
			task();
		}
		oLockGuard<oSharedMutex> Lock(TaskLock);
		// Remove command
		//if deleting the task triggered a join, its possible there isn't a task to remove. if that happens, its guaranteed the task list will be empty, not just missing a single task.
		if(!ProtectedTasks().empty()) 
			ProtectedTasks().pop_front();
		TaskCount = ProtectedTasks().size();
		ExecuteThreadID = 0;
			
		// If there are remaining Tasks execute the next
		if (TaskCount > 0)
			Issue(oBIND(&oDispatchQueueGlobalT::ExecuteNext, this, oRef<threadsafe oDispatchQueueGlobalT>(this), ExecuteKey));

		FlushLock.unlock_shared();
	}
}

#endif
