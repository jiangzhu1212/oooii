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
#include "pch.h"
#include <oooii/oThreading.h>
#include <oooii/oAssert.h>
#include <oooii/oConcurrentQueueOptimisticFIFO.h>
#include <oooii/oErrno.h>
#include <oooii/oEvent.h>
#include <oooii/oRef.h>
#include <oooii/oRefCount.h>
#include <oooii/oSingleton.h>
#include <oooii/oStdio.h>
#include <oooii/oThread.h>
#include <oooii/oWindows.h>

oSTATICASSERT(sizeof(oMutex) == sizeof(CRITICAL_SECTION));
oMutex::oMutex() { InitializeCriticalSection((LPCRITICAL_SECTION)Footprint); }
oMutex::~oMutex() { DeleteCriticalSection((LPCRITICAL_SECTION)Footprint); }
void* oMutex::GetNativeHandle() threadsafe { return (LPCRITICAL_SECTION)Footprint; }
void oMutex::Lock() threadsafe { EnterCriticalSection((LPCRITICAL_SECTION)Footprint); }
bool oMutex::TryLock() threadsafe { return !!TryEnterCriticalSection((LPCRITICAL_SECTION)Footprint); }
void oMutex::Unlock() threadsafe { LeaveCriticalSection((LPCRITICAL_SECTION)Footprint); }

#ifndef _DEBUG
	oSTATICASSERT(sizeof(oRWMutex) == sizeof(SRWLOCK));
#endif

oRWMutex::oRWMutex()
{
	InitializeSRWLock((PSRWLOCK)&Footprint);
	#ifdef _DEBUG
		ThreadID = 0;
	#endif
}

oRWMutex::~oRWMutex()
{
}

void* oRWMutex::GetNativeHandle() threadsafe
{
	return (PSRWLOCK)&Footprint;
}

void oRWMutex::Lock() threadsafe
{
	#ifdef _DEBUG
		oCheckUnlocked(*this);
	#endif

	AcquireSRWLockExclusive((PSRWLOCK)&Footprint);

	#ifdef _DEBUG
		ThreadID = oThread::GetCurrentThreadID();
	#endif
}

void oRWMutex::LockRead() const threadsafe
{
	#ifdef _DEBUG
		// @oooii-tony: Compiler bug? This type-cast should *really* not be necessary.
		// Without the type-cast, VS2008 SP1 is confused as to whether *this is a 
		// threadsafe oRWMutex or a threadsafe oMutex, but those are unrelated types.
		// And we're here, in this itself, so why the confusion? Even if there is a 
		// reason, why here and NOT above in ::Lock()?
		oCheckUnlocked((threadsafe oRWMutex&)*this);
	#endif
	
	AcquireSRWLockShared((PSRWLOCK)&Footprint);

	#ifdef _DEBUG
		ThreadID = oThread::GetCurrentThreadID();
	#endif
}

void oRWMutex::Unlock() threadsafe
{
	#ifdef _DEBUG
		ThreadID = 0;
	#endif
	ReleaseSRWLockExclusive((PSRWLOCK)&Footprint);
}

void oRWMutex::UnlockRead() const threadsafe
{
	#ifdef _DEBUG
		ThreadID = 0;
	#endif
	ReleaseSRWLockShared((PSRWLOCK)&Footprint);
}

void oCheckUnlocked(threadsafe oMutex& _Mutex)
{
	if (!_Mutex.TryLock())
	{
		oASSERT(false, "Mutex is read-write locked. This could result in a deadlock.");
		_Mutex.Unlock();
	}
}

#ifdef _DEBUG
struct INTERNAL_RWMUTEX
{
	oRWMUTEX_FOOTPRINT();
};
#endif

void oCheckUnlocked(threadsafe oRWMutex& _RWMutex)
{
	// Why are the Try's only in VS2010? MS should retro-patch for VS2008.
	#ifdef oWINDOWS_FEATURE_LEVEL_VISUAL_STUDIO_2010
		PSRWLOCK pLock = (PSRWLOCK)&_RWMutex;

		if (!TryAcquireSRWLockShared(pLock))
		{
			oASSERT(false, "RWMutex is non-recursive and already read/shared locked. This could result in a deadlock.");
			_RWMutex.UnlockRead();
		}

		if (!TryAcquireSRWLockExclusive(pLock))
		{
			oASSERT(false, "RWMutex is non-recursive and already read-write/exclusively locked. This could result in a deadlock.");
			_RWMutex.Unlock();
		}
	#else
		#ifdef _DEBUG
			// @oooii-tony: Based on what I've observed, the low bit of the word that is
			// the slim RW lock is 1 when locked and 0 when not locked, so test for that...
			threadsafe INTERNAL_RWMUTEX& internalRWMutex = (threadsafe INTERNAL_RWMUTEX&)_RWMutex;
			oASSERT(!internalRWMutex.Footprint || internalRWMutex.ThreadID != oThread::GetCurrentThreadID(), "RWMutex is non-recursive and already read/shared locked. This could result in a deadlock.");
		#endif
	#endif
}

void oYield()
{
#if defined(_WIN32) || defined(_WIN64)
	SwitchToThread();
#else
	#error Unsupported platform
#endif
}

void oSleep(unsigned int _Milliseconds)
{
	Sleep((DWORD)_Milliseconds);
}

namespace oAsyncFileIO
{
	struct QUEUED_READ
	{
		oDECLARE_NEW_DELETE();

		QUEUED_READ(const DESC* _pDesc, RESULT* _pResult)
			: Desc(*_pDesc)
			, pResult(_pResult)
		{
			strcpy_s(Path, _pDesc->Path);
			Desc.Path = Path;
		}

		RESULT* pResult;
		DESC Desc;
		char Path[_MAX_PATH];

		void Run()
		{
			void* pBuffer = 0;
			size_t size = 0;
			errno_t err = 0;
			
			if (!oLoadBuffer(&pBuffer, &size, Desc.Allocate ? Desc.Allocate : malloc, Desc.Path, oFile::IsText(Desc.Path)))
				err = oGetLastError();

			if (pResult)
			{
				pResult->pData = pBuffer;
				pResult->Size = size;
				pResult->Result = err;
			}

			Desc.Continuation(pResult);
		}
	};

	struct oIOThreadProc : public oThread::Proc
	{
		void Reference() threadsafe override {}
		void Release() threadsafe override
		{
			oASSERT(thread_cast<oIOThreadProc*>(this)->Queue.IsEmpty(), "There are outstanding files scheduled for read");
		}

		oIOThreadProc(size_t _MaxNumQueuedRequests)
			: Queue("oIOThread Queue", oPooledAllocatorBase::InitElementCount, _MaxNumQueuedRequests)
		{
			if (!oThread::Create("oIOThread", 32*1024, false, this, &Thread))
				oASSERT(false, "Failed to create oIOThread");
		}

		~oIOThreadProc()
		{
			Thread->Exit();
			WorkAvailable.Set();
			Thread->Wait();
		}

		void QueueFileRead(const DESC* _pDesc, RESULT* _pResult)
		{
			// Loop/blocking if we overrun the max number of requests
			QUEUED_READ* r = 0;
			while (1)
			{
				r = new QUEUED_READ(_pDesc, _pResult);
				if (r) break;
				oSleep(5);
			}

			Queue.Push(r);
			WorkAvailable.Set();
		}

		void RunIteration() override
		{
			WorkAvailable.Wait();
			WorkAvailable.Reset();
			QUEUED_READ* r = 0;
			while (Queue.TryPop(r))
			{
				r->Run();
				delete r;
			}
		}

		bool OnBegin() override { return true; }
		void OnEnd() override {}

		threadsafe oRef<oThread> Thread;
		oEvent WorkAvailable;
		oConcurrentQueueOptimisticFIFO<QUEUED_READ*> Queue;
	};

	struct oAsyncFileIOContext : oProcessSingleton<oAsyncFileIOContext>
	{
		oAsyncFileIOContext()
			: Pool("AsycFileIO QUEUED_READ Pool")
			, pIOThreadProc(0)
		{}

		~oAsyncFileIOContext()
		{
			Deinitialize();
		}

		bool Initialize(size_t _MaxNumQueuedRequests, bool _ExecuteOnMainThread)
		{
			if (pIOThreadProc)
			{
				oSetLastError(EINVAL, "IOThread already initialized");
				return false;
			}

			if (!_ExecuteOnMainThread)
			{
				Pool.Initialize(oPooledAllocatorBase::InitElementCount, _MaxNumQueuedRequests);
				oIOThreadProc* io = new oIOThreadProc(_MaxNumQueuedRequests);
				oSWAP(&pIOThreadProc, io);
				oSetLastError(0, "");
			}

			return true;
		}

		void Deinitialize()
		{
			oIOThreadProc* p = pIOThreadProc;
			if (p)
			{
				oSWAP<oIOThreadProc*>(&pIOThreadProc, 0);
				delete p;
				Pool.Deinitialize();
			}
		}

		oIOThreadProc* pIOThreadProc;
		oConcurrentPooledAllocator<QUEUED_READ> Pool;
	};

	void* QUEUED_READ::operator new(size_t _Size) { return oAsyncFileIOContext::Singleton()->Pool.Allocate(); }
	void QUEUED_READ::operator delete(void* _Pointer) { oAsyncFileIOContext::Singleton()->Pool.Deallocate(_Pointer); }
	void* QUEUED_READ::operator new[](size_t _Size) { return 0; }
	void QUEUED_READ::operator delete[](void* _Pointer) {}

	bool InitializeIOThread(size_t _MaxNumQueuedRequests, bool _ExecuteOnMainThread)
	{
		return oAsyncFileIOContext::Singleton()->Initialize(_MaxNumQueuedRequests, _ExecuteOnMainThread);
	}

	void DeinitializeIOThread()
	{
		oAsyncFileIOContext::Singleton()->Deinitialize();
	}

	void ScheduleFileRead(const DESC* _pDesc, RESULT* _pResult)
	{
		oIOThreadProc* io = oAsyncFileIOContext::Singleton()->pIOThreadProc;

		if (!io)
		{
			char buf[sizeof(QUEUED_READ)];
			QUEUED_READ* r = (QUEUED_READ*)buf;
			new (r) QUEUED_READ(_pDesc, _pResult);
			r->Run();
			r->~QUEUED_READ();
		}

		else
			io->QueueFileRead(_pDesc, _pResult);
	}

} // namespace oAsyncFileIO
