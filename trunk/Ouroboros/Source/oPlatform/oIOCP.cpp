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
#include "oIOCP.h"
#include <oBasis/oConcurrentIndexAllocator.h>
#include <oBasis/oCountdownLatch.h>
#include <oBasis/oFixedString.h>
#include <oBasis/oMutex.h>
#include <oBasis/oRef.h>
#include <oBasis/oRefCount.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/oReporting.h>
#include <oPlatform/oProcessHeap.h>
#include <oPlatform/oDebugger.h>

#define IOCPKEY_SHUTDOWN 1
#define IOCPKEY_USER_TASK 2

//#define DEBUG_IOCP_ALLOCATIONS

struct oIOCPContext
{
public:
	oIOCPOp* GetOp();
	void ReturnOp(oIOCPOp* _pOP);

	template<typename FUNC>
	void Callback(oIOCPOp* _pOP, FUNC _func);
	static void CallBackUser(oIOCPOp* _pOP);
	static void CallBackUserTask(oIOCPOp* _pOP);

	// Since this object outlives the underlying IOCP
	// it keeps the refcount, and lends it to the IOCP
	oRefCount& LendRefCount() { return ParentRefCount; }

private:
	friend struct oIOCP_Singleton;

	// The IOCP singleton carefully manages
	// the lifetime of the Context as it
	// outlives the actual IOCP object 
	// due to how windows handles IOCP completion status
	oIOCPContext( struct oIOCP_Impl* _pParent );
	~oIOCPContext();

	oIOCPOp*					pSocketOps;
	unsigned int*				pSocketIndices;
	oConcurrentIndexAllocator*	pSocketAllocator;
	struct oIOCP_Impl*			pParent;
	oRefCount					ParentRefCount;
};

bool IOCPCompletionRoutine(HANDLE _hIOCP, unsigned int _TimeoutMS = INFINITE)
{
	DWORD numberOfBytes;
	ULONG_PTR key;
	oIOCPOp* pSocketOp;

	if(GetQueuedCompletionStatus(_hIOCP, &numberOfBytes, &key, (WSAOVERLAPPED**)&pSocketOp, _TimeoutMS))
	{
		// Ignore all input if the Socket is trying to shut down. The callbacks may no longer exist.
		if( key == IOCPKEY_SHUTDOWN)
			return false;
		else if( key == IOCPKEY_USER_TASK)
			oIOCPContext::CallBackUserTask(pSocketOp);
		else
			oIOCPContext::CallBackUser(pSocketOp);
	}

	return true;
}

void IOCPThread(HANDLE	_hIOCP, unsigned int _Index, oCountdownLatch* _pLatch)
{
	oFixedString<char, 64> Name;
	oPrintf(Name, "IOCP Thread: %d", _Index);
	_pLatch->Release();
	_pLatch = NULL; // Drop the latch as it's not valid after releasing
	oTaskRegisterThisThread(Name); //Threre is a deadlock if this is before the latch is released. see bug 1999

	while(1)
	{
		if(!IOCPCompletionRoutine(_hIOCP))
			break;
	} 

	oEndThread();
}

struct oIOCP_Impl : public oIOCP
{
public:
	oDEFINE_TRIVIAL_QUERYINTERFACE(oIOCP);

	int Reference() threadsafe override
	{
		return pContext->LendRefCount().Reference();
	}

	void Release() threadsafe override;
	
	oIOCP_Impl(const DESC& _Desc, oTASK _ParentDestructionTask, bool* _pSuccess);

	oIOCPOp* AcquireSocketOp() override;
	void ReturnOp(oIOCPOp* _pIOCPOp) override;
	void DispatchIOTask(oTASK&& _task) override;
	void DispatchManualCompletion(oHandle _handle, oIOCPOp* _pIOCPOp) override;

	void CallBackUser(oIOCPOp* _pOP);
	void CallBackUserTask(oIOCPOp* _pOP);
	 
	void GetDesc(DESC* _pDesc){*_pDesc = Desc;}

private:
	DESC Desc;
	oIOCPContext* pContext;
	oTASK ParentDestructionTask;
};

oAPI int oIOCPThreadCount()
{
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	// fixme: Too many IOCP threads choke the system, need a better way to determine the correct number
	return sysInfo.dwNumberOfProcessors;
}

// oIOCP_Singleton spawns and maintains the worker threads.
struct oIOCP_Singleton : public oProcessSingleton<oIOCP_Singleton>
{
	oIOCP_Singleton()
		: OutstandingContextCount(0)
	{
		oReportingReference();

		unsigned int NumThreads = oIOCPThreadCount();

		hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, NumThreads);

		if(!hIOCP)
		{
			oErrorSetLast(oERROR_INVALID_PARAMETER, "Could not create I/O Completion Port with NumThreads=%i", NumThreads);
			return;
		}

		oCountdownLatch InitLatch("IOCP Init Latch", NumThreads);  // Create a latch to ensure by the time we return from the constructor all threads have initialized properly.
		WorkerThreads.resize(NumThreads);
		for(unsigned int i = 0; i < NumThreads; i++)
		{
			WorkerThreads[i] = oStd::thread(&IOCPThread, hIOCP, i, &InitLatch);
		}
		InitLatch.Wait();
	}

	~oIOCP_Singleton()
	{
		oFOR(oStd::thread& Thread, WorkerThreads)
		{
			// Post a shutdown message for each worker to unblock and disable it.
			PostQueuedCompletionStatus(hIOCP, 0, IOCPKEY_SHUTDOWN, nullptr);
		}

		oFOR(oStd::thread& Thread, WorkerThreads)
			Thread.join();

		if(INVALID_HANDLE_VALUE != hIOCP)
			CloseHandle(hIOCP);

		oFOR(oIOCPOrphan& orphan, OrphanedContexts)
		{
			delete orphan.pContext;
		}

		oReportingRelease();
	}

	void Flush()
	{
		oBackoff bo;
		while(OutstandingContextCount > 0)
		{ 
			bo.Pause(); 
		}

		oLockGuard<oMutex> lock(Mutex);
		CheckForOrphans(true);
	}

	oIOCPContext* NewIOCPContext(oIOCP_Impl* _pIOCP)
	{
#ifdef DEBUG_IOCP_ALLOCATIONS
		return new(oDebuggerGuardedAlloc(sizeof(oIOCPContext))) oIOCPContext(_pIOCP);
#else
		return new oIOCPContext(_pIOCP);
#endif
	}

	oIOCPContext* RegisterIOCP(oHandle& _Handle, oIOCP_Impl* _pIOCP)
	{
		oLockGuard<oMutex> lock(Mutex);
		++OutstandingContextCount;
		CheckForOrphans();

		if(_Handle == INVALID_HANDLE_VALUE)
			return NewIOCPContext(_pIOCP);

		ULONG_PTR key = reinterpret_cast<ULONG_PTR>(_Handle);
		//HACK: if a socket is a recycled socket, then this can fail with ERROR_INVALID_PARAMETER. HAve not currently found a way
		//	to check with windows if a handle is already associated with iocp. So for now just assume that if we get this error back
		//	it is because we are recycling a socket, and therefore everything is ok.
		if(hIOCP != CreateIoCompletionPort(_Handle, hIOCP, key, oUInt(WorkerThreads.size()) ) && GetLastError() != ERROR_INVALID_PARAMETER)
		{
			oWinSetLastError();
			return nullptr;
		}
		return NewIOCPContext(_pIOCP);
	}

	void UnregisterIOCP(oIOCPContext* _pContext)
	{
		oLockGuard<oMutex> lock(Mutex);
		oIOCPOrphan Context;
		Context.pContext = _pContext;
		Context.TimeReleased = oTimer();
		OrphanedContexts.push_back(Context);
		do
		{
			CheckForOrphans();
		}while(OrphanedContexts.size() == OrphanedContexts.capacity());
	
		--OutstandingContextCount;
	}

	void ActiveWait(oInterprocessEvent* _pEvent, unsigned int _TimeoutMS)
	{
		oScopedPartialTimeout Timeout(&_TimeoutMS);
		while(_TimeoutMS && !_pEvent->Wait(0))
		{
			if(!IOCPCompletionRoutine(hIOCP, 0) ) // Re-post the shutdown key if we pulled one out of the IOCP system as ActiveWait is done outside of the normal worker thread routine
				PostQueuedCompletionStatus(hIOCP, 0, IOCPKEY_SHUTDOWN, nullptr);

			Timeout.UpdateTimeout();
		}
	}

	void PostUserTask(oIOCPOp* _IOCP)
	{
		PostQueuedCompletionStatus(hIOCP, 0, IOCPKEY_USER_TASK, _IOCP);
	}

	void PostManualCompletion(oHandle _handle, oIOCPOp* _pIOCPOp)
	{
		PostQueuedCompletionStatus(hIOCP, 0, reinterpret_cast<ULONG_PTR>(_handle), _pIOCPOp);
	}

	static const oGUID GUID;

private:

	void CheckForOrphans(bool _Force = false)
	{
		double Time = oTimer();
		for( tOrphanList::iterator o = OrphanedContexts.begin(); o != OrphanedContexts.end();)
		{
			double ReleaseTime = o->TimeReleased;
			if( _Force || Time - o->TimeReleased > DEAD_SOCKET_OP_TIMEOUT_SECONDS )
			{
#ifdef DEBUG_IOCP_ALLOCATIONS
				o->pContext->~oIOCPContext();
				oDebuggerGuardedFree(o->pContext);
#else
				delete o->pContext;
#endif
				o = OrphanedContexts.erase(o);
				continue;
			}
			++o;
		}

	}

	const static unsigned int DEAD_SOCKET_OP_TIMEOUT_SECONDS = 300; 

	struct oIOCPOrphan
	{
		double TimeReleased;
		oIOCPContext* pContext;
	};

	typedef oArray<oIOCPOrphan,oKB(16)> tOrphanList;
	typedef std::vector<oStd::thread> tThreadList;			

	HANDLE			hIOCP;
	tOrphanList		OrphanedContexts;
	tThreadList		WorkerThreads;
	oMutex			Mutex;
	volatile int	OutstandingContextCount;
};

// {3DF7A5F8-BD85-4BC0-B295-DF86144C34A5}
const oGUID oIOCP_Singleton::GUID = { 0x3df7a5f8, 0xbd85, 0x4bc0, { 0xb2, 0x95, 0xdf, 0x86, 0x14, 0x4c, 0x34, 0xa5 } };

void oIOCP_Impl::Release() threadsafe
{
	if (pContext->LendRefCount().Release())
	{
		auto pContextCopy = pContext;
		thread_cast<oIOCP_Impl*>(this)->ParentDestructionTask(); // same because we're not longer a valid object
		delete this;
		oIOCP_Singleton::Singleton()->UnregisterIOCP(pContextCopy);
	}
}

bool oIOCPCreate(const oIOCP::DESC& _Desc, oTASK _ParentDestructionTask, oIOCP** _ppIOCP)
{
	bool success = false;
	oCONSTRUCT(_ppIOCP, oIOCP_Impl(_Desc, _ParentDestructionTask, &success));
	return success;
}

oIOCPOp* oIOCPContext::GetOp()
{
	unsigned int AllocIndex = pSocketAllocator->Allocate();
	if( AllocIndex == oIndexAllocatorBase::InvalidIndex )
		return nullptr;

	return &pSocketOps[AllocIndex];
}

void oIOCPContext::ReturnOp( oIOCPOp* _pOP )
{
	unsigned int Index = static_cast<unsigned int>( _pOP - pSocketOps );
	_pOP->Reset();
	pSocketAllocator->Deallocate( Index );
}

oIOCPContext::oIOCPContext( struct oIOCP_Impl* _pParent)
	: pParent(_pParent)
{
	oIOCP::DESC Desc;
	pParent->GetDesc(&Desc);

#ifdef DEBUG_IOCP_ALLOCATIONS
	pSocketOps = (oIOCPOp*)oDebuggerGuardedAlloc(sizeof(oIOCPOp) * Desc.MaxOperations );
	for(unsigned int i = 0; i < Desc.MaxOperations; ++i)
		new(pSocketOps + i) oIOCPOp();

	pSocketIndices = (unsigned int*)oDebuggerGuardedAlloc(sizeof(unsigned int) * Desc.MaxOperations);
	pSocketAllocator = new(oDebuggerGuardedAlloc(sizeof(oConcurrentIndexAllocator)))  oConcurrentIndexAllocator(pSocketIndices, Desc.MaxOperations * sizeof(unsigned int));
#else
	pSocketOps = new oIOCPOp[Desc.MaxOperations];
	pSocketIndices = new unsigned int[Desc.MaxOperations];
	pSocketAllocator = new oConcurrentIndexAllocator(pSocketIndices, Desc.MaxOperations * sizeof(unsigned int) );
#endif


	// Bind the context and construct the private data
	for(size_t i = 0; i < Desc.MaxOperations; ++i)
	{
		pSocketOps[i].pContext = this;
		// stealing this memory for user tasks. so make sure there is enough for that.
		size_t PrivateSize = std::max(Desc.PrivateDataSize, sizeof(oTASK));

#ifdef DEBUG_IOCP_ALLOCATIONS
		pSocketOps[i].pPrivateData = oDebuggerGuardedAlloc(PrivateSize);
#else
		pSocketOps[i].pPrivateData = new unsigned char[PrivateSize];
#endif
	}
}

oIOCPContext::~oIOCPContext()
{
	pSocketAllocator->Reset();
	size_t OpCount = pSocketAllocator->GetCapacity();
	for(size_t i = 0; i < OpCount; ++i )
	{
#ifdef DEBUG_IOCP_ALLOCATIONS
		oDebuggerGuardedFree(pSocketOps[i].pPrivateData);
#else
		delete pSocketOps[i].pPrivateData;
#endif
	}

#ifdef DEBUG_IOCP_ALLOCATIONS

	pSocketAllocator->~oConcurrentIndexAllocator();
	oDebuggerGuardedFree(pSocketAllocator);
	oDebuggerGuardedFree(pSocketIndices);
	for(unsigned int i = 0; i < OpCount; ++i)
		(pSocketOps + i)->~oIOCPOp();

	oDebuggerGuardedFree(pSocketOps);

#else
	delete pSocketAllocator;
	delete pSocketIndices;
	delete pSocketOps;
#endif
}

template<typename FUNC>
void oIOCPContext::Callback(oIOCPOp* _pOP, FUNC _func)
{
	// If we successfully reference the IOCP, it is still valid and 
	// we can callback the user releasing the reference when done.
	// The reference counting is done in a very specific manner since
	// the IOCPContext holds the refcount of the parent IOCP object.  This
	// involves explicitly refcounting on the raw refcount, validating the 
	// refcount and then releasing implicitly via the interface to ensure that
	// if we are releasing the final ref we allow the object to destruct properly.
	int NewCount = ParentRefCount.Reference();
	if ( ParentRefCount.Valid() )
	{
		if ( NewCount > 1 )
			(pParent->*_func)(_pOP);
		else
			ReturnOp(_pOP);

		pParent->Release();
	}
}

void oIOCPContext::CallBackUser( oIOCPOp* _pOP )
{
	_pOP->pContext->Callback(_pOP, &oIOCP_Impl::CallBackUser);
}

void oIOCPContext::CallBackUserTask( oIOCPOp* _pOP )
{
	_pOP->pContext->Callback(_pOP, &oIOCP_Impl::CallBackUserTask);
}

oIOCP_Impl::oIOCP_Impl(const DESC& _Desc, oTASK _ParentDestructionTask, bool* _pSuccess)
	: Desc(_Desc)
	, ParentDestructionTask(_ParentDestructionTask)
{
	if( !Desc.IOCompletionRoutine && Desc.Handle != INVALID_HANDLE_VALUE )
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER, "No IOCompletionRoutine specified, only supported if only issuing oTask's to iocp");
		return;
	}

	pContext = oIOCP_Singleton::Singleton()->RegisterIOCP(Desc.Handle, this );
	if( nullptr == pContext)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER, "Failed to create IOCP context");
		return;
	}
	*_pSuccess = true;
}

oIOCPOp* oIOCP_Impl::AcquireSocketOp()
{
	return pContext->GetOp();
}

void oIOCP_Impl::ReturnOp( oIOCPOp* _pIOCPOp )
{
	pContext->ReturnOp(_pIOCPOp);
}

void oIOCP_Impl::DispatchIOTask(oTASK&& _task)
{
	oIOCPOp* pIOCPOp = AcquireSocketOp();
	if( !pIOCPOp )
	{
		oErrorSetLast(oERROR_AT_CAPACITY, "IOCPOpPool is empty, you're sending too fast.");
		// need someway to let user know. 
		oASSERT(false, "IOCP will never call user task, probably very bad");
		return;
	}

	oTASK* task;
	pIOCPOp->ConstructPrivateData(&task);
	(*task) = std::move(_task);
	oIOCP_Singleton::Singleton()->PostUserTask(pIOCPOp);
}

void oIOCP_Impl::DispatchManualCompletion(oHandle _handle, oIOCPOp* _pIOCPOp)
{
	oIOCP_Singleton::Singleton()->PostManualCompletion(_handle, _pIOCPOp);
}

void oIOCP_Impl::CallBackUser( oIOCPOp* _pOP )
{
	Desc.IOCompletionRoutine(_pOP);
}

void oIOCP_Impl::CallBackUserTask( oIOCPOp* _pOP )
{
	oTASK* task;
	_pOP->GetPrivateData(&task);
	(*task)();

	_pOP->DestructPrivateData<oTASK>();
	ReturnOp(_pOP);
}

#include "oCRTLeakTracker.h"
void InitializeIOCP()
{
	oCRTLeakTracker::Singleton();

	oIOCP_Singleton::Singleton();
}

// obug_1763: We need to forcefully flushIOCP to ensure it doesn't report memory leaks.
void FlushIOCP()
{
	oIOCP_Singleton::Singleton()->Flush();
}

const oGUID& oGetGUID( threadsafe const oIOCP* threadsafe const * )
{
	// {5574C1B0-7F26-4A32-9A9A-93C17201060D}
	static const oGUID guid = { 0x5574c1b0, 0x7f26, 0x4a32, { 0x9a, 0x9a, 0x93, 0xc1, 0x72, 0x1, 0x6, 0xd } };
	return guid;
}

oAPI void oIOCPActiveWait(oInterprocessEvent* _pEvent, unsigned int _TimeoutMS)
{
	oIOCP_Singleton::Singleton()->ActiveWait(_pEvent, _TimeoutMS);
}

