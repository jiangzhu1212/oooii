// $(header)
#include "oIOCP.h"
#include <oooii/oThread.h>
#include <oooii/oGUID.h>
#include <oooii/oRef.h>
#include <oooii/oRefCount.h>
#include <oooii/oEvent.h>
#include <oooii/oSingleton.h>
#include <oooii/oMutex.h>
#include <oooii/oAssert.h>

#define IOCPKEY_SHUTDOWN 1

// An oIOCP_WorkerThread is spawned for each thread in the IOCP pool. There
// should be, arguably, 1 or 2 threads per CPU Core.
struct oIOCP_WorkerThread : public oThread::Proc
{
	typedef oIOCP::callback_t callback_t;

	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<oIOCP_WorkerThread>());

	oRefCount RefCount;
	oRef<threadsafe oThread> Thread;
	oEvent threadInitialized;

	enum eState
	{
		Running,
		ShuttingDown,
	};

	oIOCP_WorkerThread(HANDLE _hIOCP, callback_t _Callback);
	~oIOCP_WorkerThread();

	void Wait() threadsafe;

	virtual void RunIteration() override;
	virtual bool OnBegin() override;
	virtual void OnEnd() override;

	void Shutdown() threadsafe;

	eState		State;
	HANDLE		hIOCP;
	callback_t	Callback;
};

const oGUID& oGetGUID( threadsafe const oIOCP_WorkerThread* threadsafe const * )
{
	// {5F7AC921-CBCA-442a-87D3-0E5828B4F777}
	static const oGUID guid = { 0x5f7ac921, 0xcbca, 0x442a, { 0x87, 0xd3, 0xe, 0x58, 0x28, 0xb4, 0xf7, 0x77 } };
	return guid;
}

oIOCP_WorkerThread::oIOCP_WorkerThread(HANDLE _hIOCP, callback_t _Callback)
	: State(Running)
	, hIOCP(_hIOCP)
	, Callback(_Callback)
{
	if (oThread::Create("IOCP Worker Thread", 64*1024, false, this, &Thread))
	{
		Release(); // prevent circular ref
		threadInitialized.Wait();
	}
}

oIOCP_WorkerThread::~oIOCP_WorkerThread()
{

}

void oIOCP_WorkerThread::Wait() threadsafe
{
	if(Thread)
	{
		Thread->Exit();
		Thread->Wait();
		Thread = 0;
	}
}

void oIOCP_WorkerThread::RunIteration()
{
	if(State == ShuttingDown)
	{
		//Thread->Exit();
		return;
	}

	DWORD numberOfBytes;
	ULONG_PTR key;
	WSAOVERLAPPED* pOverlapped;
	if(GetQueuedCompletionStatus(hIOCP, &numberOfBytes, &key, &pOverlapped, INFINITE))
	{
		// oTRACE("Processing completion: Key=%i, Op=%i", key, pOverlapped ? pOverlapped->Op : -1);

		// Ignore all input if the Socket is trying to shut down. The callbacks may no longer exist.
		if(State == ShuttingDown || key == IOCPKEY_SHUTDOWN)
		{
			State = ShuttingDown;
			return;
		}

		oHandle handle = reinterpret_cast<oHandle>(key);
		Callback(handle, pOverlapped);
	}
}

bool oIOCP_WorkerThread::OnBegin()
{
	bool success = true;

	if (success)
		threadInitialized.Set();
	else
		oASSERT(false, "Error: %s", oGetLastErrorDesc());

	return success;
}

void oIOCP_WorkerThread::OnEnd()
{

}

void oIOCP_WorkerThread::Shutdown() threadsafe
{
	//Thread->Exit();

	State = ShuttingDown;
	PostQueuedCompletionStatus(hIOCP, 0, IOCPKEY_SHUTDOWN, NULL);
}

// oIOCP_Singleton spawns and maintains the worker threads.
struct oIOCP_Singleton : public oProcessSingleton<oIOCP_Singleton>
{
	oIOCP_Singleton()
		: Initialized	(false)
	{
		oAssert::Reference();

		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		NumThreads = sysInfo.dwNumberOfProcessors;

		hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, NumThreads);

		if(!hIOCP)
		{
			oSetLastError(EINVAL, "Could not create I/O Completion Port with NumThreads=%i", NumThreads);
			// *_pSuccess = false;
			return;
		}
	}

	~oIOCP_Singleton()
	{
		// Post a shutdown message for each worker to unblock and disable it.
		for(size_t i = 0; i < WorkerThreads.size(); i++)
			WorkerThreads[i]->Shutdown();

		for(size_t i = 0; i < WorkerThreads.size(); i++)
		{
			WorkerThreads[i]->Wait();
			WorkerThreads[i] = 0;
		}

		WorkerThreads.clear();

		if(INVALID_HANDLE_VALUE != hIOCP)
			CloseHandle(hIOCP);

		oAssert::Release();
	}

	void Init()
	{
		if(!Initialized)
		{
			WorkerThreads.resize(NumThreads);
			for(unsigned int i = 0; i < NumThreads; i++)
				WorkerThreads[i] = new oIOCP_WorkerThread(hIOCP, oBIND(&oIOCP_Singleton::IOCPCallback, this, oBIND1, oBIND2));

			Initialized = true;
		}
	}

	bool RegisterHandle(oHandle& _Handle, oIOCP::callback_t _Callback)
	{
		oRWMutex::ScopedLock lock(Mutex);

		ULONG_PTR key = reinterpret_cast<ULONG_PTR>(_Handle);
		if(hIOCP != CreateIoCompletionPort(_Handle, hIOCP, key, NumThreads))
		{
			oSetLastError(EINVAL, "Could not associate handle with I/O Completion Port");
			return false;
		}

		HandleMap[_Handle] = _Callback;

		return true;
	}

	bool UnregisterHandle(oHandle& _Handle)
	{
		HandleMap.erase(_Handle);
		return true;
	}

	void IOCPCallback(oHandle& _Handle, void* _pOverlapped)
	{
		oRWMutex::ScopedLockRead lock(Mutex);
		oIOCP::callback_t& callback = HandleMap[_Handle];
		if(callback)
			callback(_Handle, _pOverlapped);
	}

private:
	typedef std::map<oHandle, oIOCP::callback_t> tHandleMap;
	typedef std::vector< oRef<threadsafe oIOCP_WorkerThread> > tThreadList;

	HANDLE			hIOCP;
	unsigned int	NumThreads;
	tHandleMap		HandleMap;
	tThreadList		WorkerThreads;
	oRWMutex		Mutex;
	bool			Initialized;
};

void InitializeIOCP()
{
	oIOCP_Singleton::Singleton()->Init();
}

// An instance of oIOCP_Impl will automatically unregister all handles
// registered with it.
struct oIOCP_Impl : public oIOCP
{
	char		DebugName[64];
	oRefCount	RefCount;

	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<oIOCP>());

	oIOCP_Impl(const char* _DebugName, bool* _pSuccess);
	~oIOCP_Impl();

	virtual bool RegisterHandle(oHandle& _Handle, callback_t _Callback);
	virtual bool UnregisterHandle(oHandle& _Handle);

	typedef std::vector<oHandle> tHandleList;
	tHandleList HandleList;
};

const oGUID& oGetGUID( threadsafe const oIOCP* threadsafe const * )
{
	// {5574C1B0-7F26-4A32-9A9A-93C17201060D}
	static const oGUID guid = { 0x5574c1b0, 0x7f26, 0x4a32, { 0x9a, 0x9a, 0x93, 0xc1, 0x72, 0x1, 0x6, 0xd } };
	return guid;
}

bool oIOCP::Create(const char* _DebugName, oIOCP** _ppIOCP)
{
	oIOCP_Singleton::Singleton()->Init();
	bool success = false;
	oCONSTRUCT( _ppIOCP, oIOCP_Impl(_DebugName, &success));
	return success;
}

oIOCP_Impl::oIOCP_Impl(const char* _DebugName, bool* _pSuccess)
{
	*_pSuccess = false;
	*DebugName = 0;
	if (_DebugName)
		strcpy_s(DebugName, _DebugName);

	*_pSuccess = true;
}

oIOCP_Impl::~oIOCP_Impl()
{
	while(!HandleList.empty())
	{
		UnregisterHandle(HandleList.back());
	}
}

bool oIOCP_Impl::RegisterHandle(oHandle& _Handle, callback_t _Callback)
{
	if(!oIOCP_Singleton::Singleton()->RegisterHandle(_Handle, _Callback))
		return false;

	HandleList.push_back(_Handle);
	return true;
}

bool oIOCP_Impl::UnregisterHandle(oHandle& _Handle)
{
	if(!oIOCP_Singleton::Singleton()->UnregisterHandle(_Handle))
		return false;

	tHandleList::iterator it = std::find(HandleList.begin(), HandleList.end(), _Handle);
	oASSERT(it != HandleList.end(), "Trying to Unregister a handle not registered with this IOCP.");
	HandleList.erase(it);
	
	return true;
}

