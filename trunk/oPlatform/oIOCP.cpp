// $(header)
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

struct oIOCPContext
{
public:
	oIOCPOp* GetOp();
	void ReturnOp(oIOCPOp* _pOP);
	static void CallBackUser(oIOCPOp* _pOP);

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

void IOCPThread(HANDLE	_hIOCP, unsigned int _Index, oCountdownLatch* _pLatch)
{
	oFixedString<char, 64> Name;
	sprintf_s(Name, "IOCP Thread: %d", _Index);
	oDebuggerSetThreadName(Name);
	_pLatch->Release();
	_pLatch = NULL; // Drop the latch as it's not valid after releasing

	while(1)
	{
		DWORD numberOfBytes;
		ULONG_PTR key;
		oIOCPOp* pSocketOp;

		if(GetQueuedCompletionStatus(_hIOCP, &numberOfBytes, &key, (WSAOVERLAPPED**)&pSocketOp, INFINITE))
		{
			// Ignore all input if the Socket is trying to shut down. The callbacks may no longer exist.
			if( key == IOCPKEY_SHUTDOWN)
				break;

			oIOCPContext::CallBackUser(pSocketOp);
		}
	} 

	oEndThread();
}

struct oIOCP_Impl : public oIOCP
{
public:
	oDEFINE_REFCOUNT_INTERFACE(pContext->LendRefCount());
	oDEFINE_TRIVIAL_QUERYINTERFACE(oIOCP);

	oIOCP_Impl(const DESC& _Desc, threadsafe oInterface* _pParent, bool* _pSuccess);
	~oIOCP_Impl();

	oIOCPOp* AcquireSocketOp() override;
	void ReturnOp(oIOCPOp* _pIOCPOp) override;
	void CallBackUser(oIOCPOp* _pOP);
	 
	void GetDesc(DESC* _pDesc){*_pDesc = Desc;}

private:
	DESC Desc;
	oIOCPContext* pContext;
	oRef<threadsafe oInterface> Parent;
};

// oIOCP_Singleton spawns and maintains the worker threads.
struct oIOCP_Singleton : public oProcessSingleton<oIOCP_Singleton>
{
	oIOCP_Singleton()
		: OustandingContextCount(0)
	{
		oReportingReference();

		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		// fixme: Too many IOCP threads choke the system, need a better way to determine the correct number
		unsigned int NumThreads = sysInfo.dwNumberOfProcessors;

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
			WorkerThreads[i] = oStd::thread(oBIND(&IOCPThread, hIOCP, i, &InitLatch));
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
		while(OustandingContextCount > 0)
		{ 
			bo.Pause(); 
		}

		CheckForOrphans(true);
	}

	oIOCPContext* RegisterIOCP(oHandle& _Handle, oIOCP_Impl* _pIOCP)
	{
		oLockGuard<oMutex> lock(Mutex);
		++OustandingContextCount;
		CheckForOrphans();

		ULONG_PTR key = reinterpret_cast<ULONG_PTR>(_Handle);
		if(hIOCP != CreateIoCompletionPort(_Handle, hIOCP, key, oSize32(WorkerThreads.size()) ))
		{
			oErrorSetLast(oERROR_INVALID_PARAMETER, "Could not associate handle with I/O Completion Port");
			return nullptr;
		}
		return new oIOCPContext(_pIOCP);
	}

	void UNRegisterIOCP(oIOCPContext* _pContext)
	{
		oLockGuard<oMutex> lock(Mutex);
		oIOCPOrphan Context;
		Context.pContext = _pContext;
		Context.TimeReleased = oTimer();
		OrphanedContexts.push_back(Context);
		CheckForOrphans();
		--OustandingContextCount;
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
				delete o->pContext;
				o = OrphanedContexts.erase(o);
				continue;
			}
			++o;
		}

	}

	const static unsigned int DEAD_SOCKET_OP_TIMEOUT_SECONDS = 15; 

	struct oIOCPOrphan
	{
		double TimeReleased;
		oIOCPContext* pContext;
	};

	typedef oArray<oIOCPOrphan,1024> tOrphanList;
	typedef std::vector<oStd::thread> tThreadList;			

	HANDLE			hIOCP;
	tOrphanList		OrphanedContexts;
	tThreadList		WorkerThreads;
	oMutex			Mutex;
	volatile int	OustandingContextCount;
};

// {3DF7A5F8-BD85-4BC0-B295-DF86144C34A5}
const oGUID oIOCP_Singleton::GUID = { 0x3df7a5f8, 0xbd85, 0x4bc0, { 0xb2, 0x95, 0xdf, 0x86, 0x14, 0x4c, 0x34, 0xa5 } };

bool oIOCPCreate(const oIOCP::DESC& _Desc, threadsafe oInterface* _pParent, oIOCP** _ppIOCP)
{
	if (!_pParent || !_ppIOCP)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	bool success = false;
	oCONSTRUCT(_ppIOCP, oIOCP_Impl(_Desc, _pParent, &success));
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

	pSocketOps = new oIOCPOp[Desc.MaxOperations];
	pSocketIndices = new unsigned int[Desc.MaxOperations];
	pSocketAllocator = new oConcurrentIndexAllocator(pSocketIndices, Desc.MaxOperations * sizeof(unsigned int) );


	// Bind the context and construct the private data
	for(size_t i = 0; i < Desc.MaxOperations; ++i)
	{
		pSocketOps[i].pContext = this;
		pSocketOps[i].pPrivateData = new unsigned char[Desc.PrivateDataSize];
	}
}

oIOCPContext::~oIOCPContext()
{
	pSocketAllocator->Reset();
	for(size_t i = 0; i < pSocketAllocator->GetCapacity(); ++i )
	{
		delete pSocketOps[i].pPrivateData;
	}
	delete pSocketAllocator;
	delete pSocketIndices;
	delete pSocketOps;
}

void oIOCPContext::CallBackUser( oIOCPOp* _pOP )
{
	oIOCPContext* pContext = _pOP->pContext;
	
	oRefCount& ParentRefCount = pContext->ParentRefCount;

	// If we successfully reference the IOCP, it is still valid and 
	// we can callback the user releasing the reference when done.
	// The reference counting is done in a very specific manner since
	// the IOCPContext holds the refcount of the parent IOCP object.  This
	// involves explicitly refcounting on the raw refcount, validating the 
	// refcount and then releasing implicitly via the interface to ensure that
	// if we are releasing the final ref we allow the object to destruct properly.
	ParentRefCount.Reference();
	if( ParentRefCount.Valid() )
	{
		pContext->pParent->CallBackUser(_pOP);
		pContext->pParent->Release();
	}
}


oIOCP_Impl::oIOCP_Impl( const DESC& _Desc, threadsafe oInterface* _pParent, bool* _pSuccess )
	: Desc(_Desc)
	, Parent(_pParent)
{
	if( !Parent )
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER, "Parent interface not specified");
	}
	if( !Desc.IOCompletionRoutine )
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER, "No IOCompletionRoutine specified");
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


oIOCP_Impl::~oIOCP_Impl()
{
	oIOCP_Singleton::Singleton()->UNRegisterIOCP(pContext);
}

oIOCPOp* oIOCP_Impl::AcquireSocketOp()
{
	return pContext->GetOp();
}

void oIOCP_Impl::ReturnOp( oIOCPOp* _pIOCPOp )
{
	pContext->ReturnOp(_pIOCPOp);
}


void oIOCP_Impl::CallBackUser( oIOCPOp* _pOP )
{
	Desc.IOCompletionRoutine(_pOP);
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

