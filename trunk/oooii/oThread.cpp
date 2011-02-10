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
#include <oooii/oThread.h>
#include <oooii/oAssert.h>
#include <oooii/oDebugger.h>
#include <oooii/oRef.h>
#include <oooii/oRefCount.h>
#include <oooii/oSingleton.h>
#include <oooii/oStdio.h>
#include <oooii/oWindows.h>

#define oTHREAD_TRACEX(ThreadDebugName, msg, ...) oTRACE("[%u] %s: " msg, oThread::GetCurrentThreadID(), ThreadDebugName, ## __VA_ARGS__)
#define oTHREAD_TRACE(msg, ...) oTHREAD_TRACEX((oThread::Current() ? oThread::Current()->GetDebugName() : "System Thread"), msg, ## __VA_ARGS__)

struct oMainThreadIdContext : public oSingleton<oMainThreadIdContext>
{
	struct Run
	{
		Run() { oMainThreadIdContext::Singleton(); }
	};

	oMainThreadIdContext()
		: Id(GetCurrentThreadId())
	{}

	size_t Id;
};

static oMainThreadIdContext::Run oMainThreadIdContext; // @oooii-tony: ok static (we want to run code at static init time)

oTHREADLOCAL HANDLE ghThread = 0;
oTHREADLOCAL DWORD gThreadId = 0;

struct Thread_Impl : public oThread
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);

	Thread_Impl(const char* _DebugName, size_t _StackSize, bool _CreateSuspended, Thread_Impl::Proc* _Proc);
	~Thread_Impl();
	static DWORD WINAPI WinThreadProc(LPVOID lpdwThreadParam);

	void* GetNativeHandle() threadsafe override { return hThread; }
	size_t GetID() const threadsafe override { return Id; }
	bool IsRunning() const threadsafe override { return !bShouldExit; }
	bool IsSuspended() const threadsafe override { return !!bIsSuspended; }
	const char* GetDebugName() const threadsafe override { return thread_cast<const char*>(DebugName); }

	void SetAffinity(size_t _AffinityMask) threadsafe override { AffinityMask = _AffinityMask; SetThreadAffinityMask(hThread, _AffinityMask); }
	size_t GetAffinity() const override { return AffinityMask; }
	void SetPriority(PRIORITY _Priority) override { SetThreadPriority(hThread, ConvertPriority(_Priority)); }
	PRIORITY GetPriority() const override { return ConvertPriority(GetThreadPriority(hThread)); }
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
	if (!_Proc || !_ppThread) return false;
	*_ppThread = new Thread_Impl(_DebugName, _StackSize, _StartSuspended, _Proc);
	return !!*_ppThread;
}

size_t oThread::GetCurrentThreadID()
{
	return ::GetCurrentThreadId();
}

void* oThread::GetCurrentThreadNativeHandle()
{
	return ::GetCurrentThread();
}

bool oThread::CurrentThreadIsMain()
{
	return GetCurrentThreadID() == oMainThreadIdContext::Singleton()->Id;
}

Thread_Impl::Thread_Impl(const char* _DebugName, size_t _StackSize, bool _CreateSuspended, Thread_Impl::Proc* _Proc)
	: ThreadProc(_Proc)
	, AffinityMask(~0u)
	, hThread(0)
	, Id(0)
	, bIsSuspended(false)
	, bShouldSuspend(false)
	, bShouldExit(false)
{
	oASSERT(oThread::CurrentThreadIsMain(), "Threads should be created only from the main thread");
	hThread = CreateThread(0, static_cast<DWORD>(_StackSize), WinThreadProc, this, CREATE_SUSPENDED, (DWORD*)&Id);
	oASSERT(hThread, "CreateThread failed");
	strcpy_s(DebugName, _DebugName ? _DebugName : "OOOii Thread");
	oTHREAD_TRACEX(DebugName, "Created%s", _CreateSuspended ? " (suspended)" : ""); // use TRACEX because ::GetCurrent() isn't up until thread starts.
	if (!_CreateSuspended)
		Resume();
}

Thread_Impl::~Thread_Impl()
{
	Exit();

	bool done = true;
	if (oThread::GetCurrentThreadID() != GetID())
	{
		oTRACE("[%u] %s: waiting for exit...", Id, DebugName);
		done = Wait(5000);
	}
	oTRACE("[%u] %s: exited %s", Id, DebugName, done ? "successfully." : "after timeout.");
	CloseHandle(hThread);
}

DWORD Thread_Impl::WinThreadProc(LPVOID lpdwThreadParam)
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

	oTRACE("Main loop exited, Running OnEnd...");
	t->ThreadProc->OnEnd();
	sCurrent = 0;
	oTRACE("OnEnd complete, thread user code is complete.");
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
	if (oThread::GetCurrentThreadID() != this->GetID())
		Resume();
}

bool Thread_Impl::Wait(unsigned int _TimeoutMS) threadsafe
{
	DWORD result = WaitForSingleObject(hThread, _TimeoutMS);
	#ifdef _DEBUG
		char errDesc[512];
		oGetNativeErrorDesc(errDesc, _countof(errDesc), ::GetLastError());
		oASSERT(result == WAIT_TIMEOUT || result == WAIT_OBJECT_0, "Wait failed: %s", errDesc);
	#endif
	return result != WAIT_TIMEOUT;
}
