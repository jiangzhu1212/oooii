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
#include <oooii/oThread.h>
#include <oooii/oAssert.h>
#include <oooii/oDebugger.h>
#include <oooii/oRef.h>
#include <oooii/oRefCount.h>
#include <oooii/oSingleton.h>
#include <oooii/oStdio.h>
#include <oooii/oWindows.h>
#include <process.h>

#define oTHREAD_TRACEX(ThreadDebugName, msg, ...) oTRACE("[%u] %s: " msg, ::GetCurrentThreadId(), ThreadDebugName, ## __VA_ARGS__)
#define oTHREAD_TRACE(msg, ...) oTHREAD_TRACEX((oThread::Current() ? oThread::Current()->GetDebugName() : "System Thread"), msg, ## __VA_ARGS__)

// Strange that beginthreadex has a different sig in different builds
#ifdef _M_X64
	#define CALLTYPE __cdecl
#else
	#define CALLTYPE __stdcall
#endif

const oGUID& oGetGUID( threadsafe const oThread* threadsafe const * )
{
	// {A13C6FAC-3F0B-486e-9685-560D3B8C2588}
	static const oGUID oIIDThread = { 0xa13c6fac, 0x3f0b, 0x486e, { 0x96, 0x85, 0x56, 0xd, 0x3b, 0x8c, 0x25, 0x88 } };
	return oIIDThread;
}
struct Thread_Impl : public oThread
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<oThread>());

	Thread_Impl(const char* _DebugName, size_t _StackSize, bool _CreateSuspended, Thread_Impl::Proc* _Proc, bool* _pSuccess);
	~Thread_Impl();
	static unsigned int CALLTYPE WinThreadProc(void* lpdwThreadParam);

	void* GetNativeHandle() threadsafe override { return hThread; }
	size_t GetID() const threadsafe override { return Id; }
	bool IsRunning() const threadsafe override { return !bShouldExit; }
	bool IsSuspended() const threadsafe override { return !!bIsSuspended; }
	const char* GetDebugName() const threadsafe override { return thread_cast<const char*>(DebugName); }

	void SetAffinity(size_t _AffinityMask) threadsafe override { AffinityMask = _AffinityMask; SetThreadAffinityMask(hThread, _AffinityMask); }
	size_t GetAffinity() const override { return AffinityMask; }
	void SetPriority(PRIORITY _Priority) threadsafe override { SetThreadPriority(hThread, ConvertPriority(_Priority)); }
	PRIORITY GetPriority() threadsafe const override { return ConvertPriority(GetThreadPriority(hThread)); }
	void Suspend() threadsafe override { bShouldSuspend = true; }
	void Resume() threadsafe override;
	void Exit() threadsafe override;

	// Returns 0 on success or WAIT_TIMEOUT
	bool Wait(unsigned int _TimeoutMS = oINFINITE_WAIT) threadsafe override;

	oRef<oThread::Proc> ThreadProc;
	size_t AffinityMask;
	HANDLE hThread;
	unsigned int Id;
	int bIsSuspended; // bool padded for atomic operation
	oRefCount RefCount;
	char DebugName[64];
	bool bShouldSuspend;
	bool bShouldExit;
	oTHREADLOCAL static threadsafe oThread* sCurrent;
};

oTHREADLOCAL threadsafe oThread* Thread_Impl::sCurrent = 0;

threadsafe oThread* oThread::Current()
{
	return Thread_Impl::sCurrent;
}

bool oThread::Create(const char* _DebugName, size_t _StackSize, bool _StartSuspended, Proc* _Proc, threadsafe oThread** _ppThread)
{
	if (!_StackSize || !_Proc || !_ppThread)
	{
		oSetLastError(EINVAL);
		return false;
	}

	bool success = false;
	oCONSTRUCT(_ppThread, Thread_Impl(_DebugName, _StackSize, _StartSuspended, _Proc, &success))
	return success;
}

Thread_Impl::Thread_Impl(const char* _DebugName, size_t _StackSize, bool _CreateSuspended, Thread_Impl::Proc* _Proc, bool *_pSuccess)
	: ThreadProc(_Proc)
	, AffinityMask(~0u)
	, hThread(0)
	, Id(0)
	, bIsSuspended(false)
	, bShouldSuspend(false)
	, bShouldExit(false)
{
	*_pSuccess = false;
	strcpy_s(DebugName, _DebugName ? _DebugName : "OOOii Thread");
	hThread = (HANDLE)_beginthreadex(0, static_cast<unsigned int>(_StackSize), WinThreadProc, this, CREATE_SUSPENDED, &Id);
	if (!hThread)
		return;
	
	oTHREAD_TRACEX(DebugName, "Created%s", _CreateSuspended ? " (suspended)" : ""); // use TRACEX because ::GetCurrent() isn't up until thread starts.
	if (!_CreateSuspended)
		Resume();

	*_pSuccess = true;
}

Thread_Impl::~Thread_Impl()
{
	Exit();

	bool done = true;
	if (::GetCurrentThreadId() != GetID())
	{
		oTHREAD_TRACE("[%u] %s: waiting for exit...", Id, DebugName);
		done = Wait(5000);
	}
	oTHREAD_TRACE("[%u] %s: exited %s", Id, DebugName, done ? "successfully." : "after timeout.");
}

unsigned int CALLTYPE Thread_Impl::WinThreadProc(void* lpdwThreadParam)
{
	Thread_Impl* t = static_cast<Thread_Impl*>(lpdwThreadParam);
	oDebugger::SetThreadName(t->DebugName, (void*)t->hThread);
	sCurrent = t;

	oTHREAD_TRACE("Running OnBegin...");
	if (t->ThreadProc->OnBegin())
	{
		oTHREAD_TRACE("OnBegin successful, running main loop...");
		while (true)
		{
			if (t->bShouldSuspend)
			{
				oSWAP(&t->bIsSuspended, true);
				oTRACE("Suspending...");
				if (-1 == SuspendThread(t->hThread))
					oASSERT(false, "Suspend failed");
			}

			if (t->bShouldExit)
				break;

			t->ThreadProc->RunIteration();
		}
	}

	oTHREAD_TRACE("Main loop exited, Running OnEnd...");
	t->ThreadProc->OnEnd();
	sCurrent = 0;
	oTHREAD_TRACE("OnEnd complete, thread user code is complete.");
	//detail_DEPRECATED::ReleaseThreadLocalSingletons();
	oReleaseAllProcessThreadlocalSingletons();
	return 0;
}

void Thread_Impl::Resume() threadsafe
{
	bShouldSuspend = false;
	oSWAP(&bIsSuspended, false);
	ResumeThread(hThread);
}

void Thread_Impl::Exit() threadsafe
{
	bShouldExit = true;
	// If we're on this thread and we need to be resumed, then we wouldn't be
	// executing code now would we?
	if (::GetCurrentThreadId() != this->GetID())
		Resume();
}

bool Thread_Impl::Wait(unsigned int _TimeoutMS) threadsafe
{
	DWORD result = WaitForSingleObject(hThread, _TimeoutMS);

	oASSERT(result == WAIT_TIMEOUT || result == WAIT_OBJECT_0, "Wait failed: %s", oGetLastErrorDesc());
	if (result == WAIT_TIMEOUT)
		oSetLastError(ETIMEDOUT, "Thread %s timed out after %u ms", GetDebugName(), _TimeoutMS);

	return result != WAIT_TIMEOUT;
}
