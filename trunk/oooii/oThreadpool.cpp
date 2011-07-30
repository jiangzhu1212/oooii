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
#include <oooii/oThreadpool.h>
#include <oooii/oConcurrentQueueMS.h>
#include <oooii/oConcurrentQueueOptimisticFIFO.h>
#include <oooii/oCPU.h>
#include <oooii/oErrno.h>
#include <oooii/oEvent.h>
#include <oooii/oPooledAllocator.h>
#include <oooii/oRef.h>
#include <oooii/oRefCount.h>
#include <oooii/oStdio.h>
#include <oooii/oThread.h>
#include <oooii/oThreading.h>
#include <oooii/oWindows.h>
#include <vector>

struct oQueuedTask
{
	oDECLARE_NEW_DELETE();

	oThreadpool::TaskProc Task;
	void* pData;

	oQueuedTask(oThreadpool::TaskProc _Task, void* _pData) : Task(_Task), pData(_pData) {}
	inline void Run() { Task(pData); delete this; }
	static inline void Run(void* _pQueuedTask) { static_cast<oQueuedTask*>(_pQueuedTask)->Run(); }
};

// @oooii-tony: Move this into an oThreadpool's context, but try to avoid adding another
// pointer to oQueuedTask because there could be a lot (100,000+) of tasks, so that could
// be 800k on 64-bit systems.
oDEFINE_CONCURRENT_POOLED_NEW_DELETE(oQueuedTask, sQueuedTaskPool, 100000);

const oGUID& oGetGUID( threadsafe const oThreadpool* threadsafe const * )
{
	// {E4CEF1FC-6482-4746-A1D8-4145C58A1DBC}
	static const oGUID oIIDThreadPoolBase = { 0xe4cef1fc, 0x6482, 0x4746, { 0xa1, 0xd8, 0x41, 0x45, 0xc5, 0x8a, 0x1d, 0xbc } };
	return oIIDThreadPoolBase;
}

struct Threadpool_ImplBase : public oThreadpool
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<oThreadpool>());

	Threadpool_ImplBase(const DESC* _pDesc, bool *_pSuccess);
	virtual ~Threadpool_ImplBase();

	void RegisterShutdownEvent(oEvent* _pShutdownEvent) threadsafe override;
	oQueuedTask* AllocateTask(TaskProc _Task, void* _pData) threadsafe;

	oEvent* pShutdownEvent;
	DESC Desc;
	oRefCount RefCount;
};

Threadpool_ImplBase::Threadpool_ImplBase(const DESC* _pDesc, bool *_pSuccess)
	: Desc(*_pDesc)
	, pShutdownEvent(0)
{
}

Threadpool_ImplBase::~Threadpool_ImplBase()
{
	if (pShutdownEvent)
		pShutdownEvent->Set();
}

void Threadpool_ImplBase::RegisterShutdownEvent(oEvent* _pShutdownEvent) threadsafe
{
	oEvent* pOldEvent = pShutdownEvent;
	while (oCAS(&pShutdownEvent, _pShutdownEvent, pOldEvent) != pOldEvent);
}

oQueuedTask* Threadpool_ImplBase::AllocateTask(TaskProc _Task, void* _pData) threadsafe
{
	oQueuedTask* t = 0;
	if (Desc.NumThreads < 2)
		_Task(_pData);

	else while (true) // spin until we get a valid allocation
	{
		t = sQueuedTaskPool.Construct(_Task, _pData);
		if (t) break;
		oYield();
	}

	return t;
}

struct Threadpool_Impl_Windows : public Threadpool_ImplBase
{
	Threadpool_Impl_Windows(const DESC* _pDesc, bool *_pSuccess);
	~Threadpool_Impl_Windows();

	void ScheduleTask(TaskProc _Task, void* _pData) threadsafe;

	PTP_CLEANUP_GROUP TPCleanupGroup;
	PTP_POOL hPool;
	TP_CALLBACK_ENVIRON TPEnvironment;

	static VOID CALLBACK CleanupGroupCancelCallback(PVOID ObjectContext, PVOID CleanupContext) {}
	static VOID CALLBACK WorkCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work) { oQueuedTask::Run(Context); }
};

Threadpool_Impl_Windows::Threadpool_Impl_Windows(const DESC* _pDesc, bool *_pSuccess)
	: Threadpool_ImplBase(_pDesc, _pSuccess)
	,	TPCleanupGroup(CreateThreadpoolCleanupGroup())
	, hPool(CreateThreadpool(0))
{
	oASSERT(TPCleanupGroup, "CreateThreadpoolCleanupGroup failed");
	InitializeThreadpoolEnvironment(&TPEnvironment);
	SetThreadpoolCallbackPool(&TPEnvironment, hPool);
	SetThreadpoolCallbackCleanupGroup(&TPEnvironment, TPCleanupGroup, CleanupGroupCancelCallback);

	DWORD dwNumThreads = Desc.NumThreads;
	if (dwNumThreads == oINVALID)
	{
		oCPU::DESC cpu;
		oCPU::GetDesc(0, &cpu);
		dwNumThreads = cpu.NumHardwareThreads;
	}

	else if (dwNumThreads == 0)
		dwNumThreads = 1;

	if (!SetThreadpoolThreadMinimum(hPool, dwNumThreads))
		oASSERT(false, "SetThreadPoolThreadMinimum(hPool=%x, MinConcurrency=%u) failed", hPool, dwNumThreads);
	SetThreadpoolThreadMaximum(hPool, dwNumThreads);

	*_pSuccess = true;
}

Threadpool_Impl_Windows::~Threadpool_Impl_Windows()
{
	oTRACE("oThreadpool beginning dtor");
	CloseThreadpoolCleanupGroupMembers(TPCleanupGroup, TRUE, 0);
	CloseThreadpoolCleanupGroup(TPCleanupGroup);
	CloseThreadpool(hPool);
	DestroyThreadpoolEnvironment(&TPEnvironment);
}

void Threadpool_Impl_Windows::ScheduleTask(TaskProc _Task, void* _pData) threadsafe
{
	oQueuedTask* t = AllocateTask(_Task, _pData);
	if (t)
	{
		PTP_WORK TPWork = CreateThreadpoolWork(WorkCallback, t, (PTP_CALLBACK_ENVIRON)&TPEnvironment);
		SubmitThreadpoolWork(TPWork);
	}

	else
		oSetLastError(ENOMEM);
}

const oGUID& oGetGUID( threadsafe const Threadpool_ImplBase* threadsafe const * )
{
	// {F602912C-91EF-4f0c-8F23-88A1DD82964C}
	static const oGUID oIIDThreadpoolWorker = { 0xf602912c, 0x91ef, 0x4f0c, { 0x8f, 0x23, 0x88, 0xa1, 0xdd, 0x82, 0x96, 0x4c } };
	return oIIDThreadpoolWorker;
}

struct Threadpool_Impl_OOOii : public Threadpool_ImplBase
{
	struct Worker : public oThread::Proc
	{
		oDEFINE_REFCOUNT_INTERFACE(RefCount);
		oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<Threadpool_ImplBase>());

		Worker(threadsafe Threadpool_Impl_OOOii* _pThreadpool);

		void RunIteration() override;
		bool OnBegin() override { return true; }
		void OnEnd() override {}

		bool Start(const char* _DebugName) { bool success = oThread::Create(_DebugName, 64 * 1024, false, this, &Thread); Release(); return success; }
		void Stop() { if (Thread) { Reference(); Thread->Exit(); WorkAvailable.Set(); Thread->Wait(); Thread = 0; } }

		oRef<threadsafe oThread> Thread;
		threadsafe Threadpool_Impl_OOOii* pThreadpool; // raw to prevent circular refs
		threadsafe oEvent WorkAvailable;
		double WakeTime;
		oRefCount RefCount;
	};

	Threadpool_Impl_OOOii(const DESC* _pDesc, bool *_pSuccess);
	~Threadpool_Impl_OOOii();

	void ScheduleTask(TaskProc _Task, void* _pData) threadsafe;

	typedef std::vector<oRef<Worker> > workers_t;
	workers_t Workers;

	typedef oConcurrentQueueMS<oQueuedTask*> queue_t;
	//typedef oConcurrentQueueOptimisticFIFO<oQueuedTask*> queue_t;

	queue_t Tasks;
};

Threadpool_Impl_OOOii::Threadpool_Impl_OOOii(const DESC* _pDesc, bool *_pSuccess)
	: Threadpool_ImplBase(_pDesc, _pSuccess)
	, Tasks("oThreadPool Task List")
{
	*_pSuccess = true;

	size_t nThreads = _pDesc->NumThreads;

	if (nThreads == oINVALID)
	{
		oCPU::DESC cpu;
		oCPU::GetDesc(0, &cpu);
		nThreads = cpu.NumHardwareThreads;
	}

	Workers.reserve(nThreads);
	for (size_t i = 0; i < nThreads; i++)
	{
		char n[128];
		sprintf_s(n, "oThreadpool Worker %02u", i);

		oRef<Worker> w;
		w /= new Worker(this);
		Workers.push_back(w);
		w->Start(n);
	}
}

Threadpool_Impl_OOOii::~Threadpool_Impl_OOOii()
{
	// First kick all workers into active, but exiting mode
	for (workers_t::iterator it = Workers.begin(); it != Workers.end(); ++it)
		(*it)->Stop();
}

void Threadpool_Impl_OOOii::ScheduleTask(TaskProc _Task, void* _pData) threadsafe
{
	oQueuedTask* t = AllocateTask(_Task, _pData);
	if (t)
	{
		Tasks.Push(t);
		workers_t& workers = thread_cast<workers_t&>(Workers);
		for (workers_t::iterator it = workers.begin(); it != workers.end(); ++it)
			(*it)->WorkAvailable.Set();
	}
}

Threadpool_Impl_OOOii::Worker::Worker(threadsafe Threadpool_Impl_OOOii* _pThreadpool)
	: pThreadpool(_pThreadpool)
	, WakeTime(oTimer())
{
}

void Threadpool_Impl_OOOii::Worker::RunIteration()
{
	oQueuedTask* t = 0;
	if (pThreadpool->Tasks.TryPop(t))
	{
			t->Run();
			WakeTime = oTimer();
	}

	else if ((oTimer() - WakeTime) > 1.0)
	{
		WorkAvailable.Reset();
		WorkAvailable.Wait();
		WakeTime = oTimer();
	}

	else
		oYield();
}

bool oThreadpool::Create(const DESC* _pDesc, threadsafe oThreadpool** _ppThreadpool)
{
	if (!_pDesc || !_ppThreadpool)
	{
		oSetLastError(EINVAL);
		return false;
	}

	bool success = false;
	
	switch (_pDesc->Implementation)
	{
		case oThreadpool::WINDOWS_THREAD_POOL:
			*_ppThreadpool = new Threadpool_Impl_Windows(_pDesc, &success);
			break;

		case oThreadpool::OOOII:
			*_ppThreadpool = new Threadpool_Impl_OOOii(_pDesc, &success);
			break;
		
		default:
			oASSERT(false, "Only WINDOWS and OOOII implementations are currently implemented");
			oSetLastError(EINVAL);
			return false;
	}

	if (!*_ppThreadpool)
		oSetLastError(ENOMEM);
	else if (!success)
	{
		delete *_ppThreadpool;
		*_ppThreadpool = 0;
	}

	return !!*_ppThreadpool;
}
