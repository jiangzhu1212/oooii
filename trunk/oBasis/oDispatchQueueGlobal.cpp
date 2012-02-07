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
#include <oBasis/oDispatchQueueGlobal.h>
#include <oBasis/oAssert.h>
#include <oBasis/oFixedString.h>
#include <oBasis/oInitOnce.h>
#include <oBasis/oRef.h>
#include <oBasis/oRefCount.h>
#include <oBasis/oMutex.h>
#include <oBasis/oTask.h>
#include <list>

struct oDispatchQueueGlobal_Impl : public oDispatchQueueGlobal
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oDispatchQueueGlobal_Impl(const char* _DebugName, size_t _InitialTaskCapacity, bool* _pSuccess);
	~oDispatchQueueGlobal_Impl();

	virtual bool Dispatch(oTASK _Task) threadsafe override;
	virtual void Flush() threadsafe override;
	virtual void Join() threadsafe override;
	virtual bool Joinable() const threadsafe override{ return IsJoinable; }
	virtual const char* GetDebugName() const threadsafe { return DebugName->c_str(); }

	void ExecuteNext(oRef<threadsafe oDispatchQueueGlobal_Impl> _SelfRef, unsigned int _ExecuteKey) threadsafe;

	oInitOnce<oStringS> DebugName;
	typedef std::list<oTASK > tasks_t;
	tasks_t Tasks;
	oSharedMutex TaskLock;
	oSharedMutex FlushLock;
	bool IsJoinable;
	unsigned int ExecuteKey;
	oRefCount RefCount;

	tasks_t& ProtectedTasks() threadsafe { return thread_cast<tasks_t&>(Tasks); }
};

oDispatchQueueGlobal_Impl::oDispatchQueueGlobal_Impl(const char* _DebugName, size_t _InitialTaskCapacity, bool* _pSuccess)
	: Tasks() // @oooii-tony: TODO _TaskCapacity cannot be done trivially with std::list/queue/deque... a new allocator needs to be made... so do that later.
	, DebugName(_DebugName)
	, IsJoinable(true)
	, ExecuteKey(0)
{
	*_pSuccess = true;
}

oDispatchQueueGlobal_Impl::~oDispatchQueueGlobal_Impl()
{
	if (Joinable())
	{
		oASSERT(false, "Calling std::terminate because a Joinable oDispatchQueueGlobal was destroyed");
		std::terminate();
	}
}

bool oDispatchQueueCreateGlobal(const char* _DebugName, size_t _InitialTaskCapacity, threadsafe oDispatchQueueGlobal** _ppDispatchQueue)
{
	bool success = false;
	oCONSTRUCT(_ppDispatchQueue, oDispatchQueueGlobal_Impl(_DebugName, _InitialTaskCapacity, &success));
	return success;
}

bool oDispatchQueueGlobal_Impl::Dispatch(oTASK _Task) threadsafe
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
			oTaskIssueAsync(oBIND(&oDispatchQueueGlobal_Impl::ExecuteNext, this, oRef<threadsafe oDispatchQueueGlobal_Impl>(this), ExecuteKey));

		FlushLock.unlock_shared();
	}

	return Scheduled;
};

void oDispatchQueueGlobal_Impl::Flush() threadsafe
{
	Join();
	IsJoinable = true;
};

void oDispatchQueueGlobal_Impl::Join() threadsafe
{
	IsJoinable = false;
	oLockGuard<oSharedMutex> Lock(FlushLock);

	while (!ProtectedTasks().empty())
	{
		ProtectedTasks().front()();
		ProtectedTasks().pop_front();
	}
	++ExecuteKey; // Increment the execute key which will prevent old tasks from executing
};

void oDispatchQueueGlobal_Impl::ExecuteNext(oRef<threadsafe oDispatchQueueGlobal_Impl> _SelfRef, unsigned int _ExecuteKey) threadsafe
{
	if (IsJoinable && _ExecuteKey == ExecuteKey && FlushLock.try_lock_shared())
	{
		ProtectedTasks().front()();
		
		size_t TaskCount = 0;

		// Remove command
		{
			oLockGuard<oSharedMutex> Lock(TaskLock);
			// Add this command to the queue
			ProtectedTasks().pop_front();
			TaskCount = ProtectedTasks().size();
		}

		// If there are remaining Tasks execute the next
		if (TaskCount > 0)
			oTaskIssueAsync(oBIND(&oDispatchQueueGlobal_Impl::ExecuteNext, this, oRef<threadsafe oDispatchQueueGlobal_Impl>(this), ExecuteKey));

		FlushLock.unlock_shared();
	}
}
