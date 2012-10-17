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

#include "oDispatchQueueConcurrentTBB.h"
#include <oBasis/oArray.h>
#include <oBasis/oConcurrentQueue.h>
#include <oBasis/oJoinable.h>
#include <oBasis/oLockThis.h>
#include <oBasis/oMutex.h>
#include <oBasis/oRefCount.h>
#include <oBasis/oTask.h>

// gets <windows.h> all ready for tbb usage... i.e. including this prevents the
// "macro 'interface' redefinition" warning
#include <oPlatform/Windows/oWindows.h> 
#include <tbb/tbb.h> 

// @oooii-tony: oTaskGroup was exposed as a public concept for a while, but then we finished out a oStd::async
// implementation that will be more similar to how C++11 specs stuff, so favor that instead and keep oTaskGroup
// as a detail of implementation, not as a cross-platform concept.

// There is an implicit Join when this object is destroyed.
// oTaskGroup acts as a "parent" to any tasks issued by it. 
interface oTaskGroup : oInterface
{
	// Wait for any tasks that have been issued for this group to complete.
	// Issue will fail if called while flush is running.
	virtual void Flush() threadsafe = 0;
	virtual bool Issue(oTASK _task) threadsafe = 0;
};

const oGUID& oGetGUID( threadsafe const oTaskGroup* threadsafe const * )
{
	// {b5aa254d-6143-484d-ab9d-0b3eae1b1660}
	static const oGUID oIIDoTaskGroup = { 0xb5aa254d, 0x6143, 0x484d, { 0xab, 0x9d, 0x0b, 0x3e, 0xae, 0x1b, 0x16, 0x60 } };
	return oIIDoTaskGroup; 
}

const oGUID& oGetGUID( threadsafe const class oTBBTaskGroupImpl* threadsafe const * )
{
	// {d912957c-a621-4960-833f-55572a1a4abb}
	static const oGUID oIIDoTBBTaskGroupImpl = { 0xd912957c, 0xa621, 0x4960, { 0x83, 0x3f, 0x55, 0x57, 0x2a, 0x1a, 0x4a, 0xbb } };
	return oIIDoTBBTaskGroupImpl; 
}

class oTBBTaskGroupImpl : public oTaskGroup
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE2(oTaskGroup, oTBBTaskGroupImpl);

	oTBBTaskGroupImpl(bool* _pSuccess);
	~oTBBTaskGroupImpl() { Join(); }

	void Flush() threadsafe override;
	bool Issue(oTASK _task) threadsafe override;

private:
	void Join() threadsafe;

	oRefCount RefCount;
	tbb::task_group TBBGroup;
	oJoinable Joinable;
};

oTBBTaskGroupImpl::oTBBTaskGroupImpl(bool* _pSuccess)
{
	*_pSuccess = true;
}

void oTBBTaskGroupImpl::Join() threadsafe
{ 
	Joinable.Join();
	oInherentlyThreadsafe()->TBBGroup.wait();
}

void oTBBTaskGroupImpl::Flush() threadsafe
{ 
	Joinable.BeginFlush();
	oInherentlyThreadsafe()->TBBGroup.wait();
	Joinable.EndFlush();
}

#include "oCRTLeakTracker.h"
bool oTBBTaskGroupImpl::Issue(oTASK _task) threadsafe
{ 
	if (Joinable.BeginDispatch())
	{
		oCRTLeakTracker::Singleton()->EnableThreadlocalTracking(false);
		oInherentlyThreadsafe()->TBBGroup.run(_task);
		oCRTLeakTracker::Singleton()->EnableThreadlocalTracking();
		Joinable.EndDispatch();
		return true;
	}
	return false;
}

bool oTaskGroupCreate(threadsafe oTaskGroup** _ppObject)
{
	bool success = false;
	oCONSTRUCT(_ppObject, oTBBTaskGroupImpl(&success));
	return success;
}

struct oDispatchQueueConcurrentTBB_Impl : public oDispatchQueueConcurrent
{
	oDEFINE_NOOP_QUERYINTERFACE();
	oDEFINE_REFCOUNT_INTERFACE(Refcount);

	oDispatchQueueConcurrentTBB_Impl(const char* _DebugName, size_t _InitialTaskCapacity, bool* _pSuccess);

	bool Dispatch(oTASK _Command) threadsafe override { return TaskGroup ? TaskGroup->Issue(_Command) : false; }
	void Join() threadsafe override { oRef<threadsafe oTaskGroup> SelfRef = TaskGroup; TaskGroup = nullptr; } // Self ref when nulling TaskGroup to ensure it doesn't go out of scope prior to assignment
	void Flush() threadsafe override { if(TaskGroup){TaskGroup->Flush();} }
	bool Joinable() const threadsafe override { return TaskGroup != nullptr; } //TODO: might want to look at this more.
	const char* GetDebugName() const threadsafe override { return DebugName; }

	oStringS DebugName;
	oRef<threadsafe oTaskGroup> TaskGroup;
	oRefCount Refcount;
};

bool oDispatchQueueCreateConcurrentTBB(const char* _DebugName, size_t _InitialTaskCapacity, threadsafe oDispatchQueueConcurrent** _ppQueue)
{
	bool success = false;
	oCONSTRUCT(_ppQueue, oDispatchQueueConcurrentTBB_Impl(_DebugName, _InitialTaskCapacity, &success));
	return success;
}

oDispatchQueueConcurrentTBB_Impl::oDispatchQueueConcurrentTBB_Impl(const char* _DebugName, size_t _InitialTaskCapacity, bool* _pSuccess)
	: DebugName(_DebugName)
{
	*_pSuccess = oTaskGroupCreate(&TaskGroup);
}
