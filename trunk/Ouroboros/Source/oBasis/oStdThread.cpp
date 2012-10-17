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
#include <oBasis/oStdThread.h>
#include <oBasis/oStdAtomic.h>
#include <process.h>
#include "oWinHeaders.h"

typedef std::tr1::function<void()> oPROC;

oStd::thread::id oStd::this_thread::get_id()
{
	oStd::thread::id ID;
	*(unsigned int*)&ID = ::GetCurrentThreadId();
	return ID;
}

void oStd::this_thread::yield()
{
	#if defined(_WIN32) || defined(_WIN64)
		::SwitchToThread();
	#else
		#error Unsupported platform
	#endif
}

void oStd::this_thread::__sleep_for(unsigned int _Milliseconds)
{
	::Sleep(_Milliseconds);
}

oStd::thread::id::id()
	: ID(UINT_MAX)
{}

bool std::operator==(oStd::thread::id x, oStd::thread::id y) { return *(unsigned int*)&x == *(unsigned int*)&y; }
bool std::operator!=(oStd::thread::id x, oStd::thread::id y) { return *(unsigned int*)&x != *(unsigned int*)&y; }
bool std::operator<(oStd::thread::id x, oStd::thread::id y) { return *(unsigned int*)&x < *(unsigned int*)&y; }
bool std::operator<=(oStd::thread::id x, oStd::thread::id y) { return *(unsigned int*)&x <= *(unsigned int*)&y; }
bool std::operator>(oStd::thread::id x, oStd::thread::id y) { return *(unsigned int*)&x > *(unsigned int*)&y; }
bool std::operator>=(oStd::thread::id x, oStd::thread::id y) { return *(unsigned int*)&x >= *(unsigned int*)&y; }

unsigned int oStd::thread::hardware_concurrency()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
}

oStd::thread::thread()
	: hThread(nullptr)
{}

static void __cdecl ostd_thread_proc(void* lpdwThreadParam)
{
	oPROC* pFN = (oPROC*)lpdwThreadParam;
	(*pFN)();
	pFN->~oPROC();
	_free_dbg(pFN, _CRT_BLOCK);
	_endthread();
}

void oStd::thread::initialize(std::tr1::function<void()> _ThreadProc)
{
	// Because a move operator may null out or change ThreadProc, we
	// need to make a separate, persistent copy. Also allocate this in a way that
	// doesn't show up as a false positive on leak reports. Since this is 
	// std::thread, flag the alloc as a CRT allocation because I imagine when
	// MSVC implements this it will be.
	oPROC* pProc = new (_malloc_dbg(sizeof(oPROC), _CRT_BLOCK, __FILE__, __LINE__)) std::tr1::function<void()>();
	*pProc = _ThreadProc;
	hThread = (void*)_beginthread(ostd_thread_proc, 64*1024, pProc);
}

void oStd::thread::move(thread& _That)
{
	oMOVE1(*this, _That, hThread);
}

oStd::thread::~thread()
{
	if (joinable())
	{
		oASSERT(false, "Calling std::terminate because a joinable thread was destroyed");
		std::terminate();
	}
}

void std::swap(oStd::thread& _this, oStd::thread& _That)
{
	_this.swap(_That);
}

void oStd::thread::swap(thread& _That)
{
	std::swap(hThread, _That.hThread);
}

void oStd::thread::detach()
{
	oStd::atomic_exchange(&hThread, 0);
}

static bool oThreadIsJoinable(HANDLE _hThread)
{
	DWORD exitCode = 0;
	if (!GetExitCodeThread(_hThread, &exitCode))
		return false;
	return exitCode == STILL_ACTIVE;
}

void oStd::thread::join()
{
	if (joinable())
	{
		oStd::thread::id ID = get_id(); // can't get ID after join, so cache it now
		oTRACE("Thread 0x%x is being joined...", ID);

		// @oooii-tony: There was a case just before the "Saint Tropez" ship where
		// The above trace would print, THEN the system message "...exited with code 
		// 0 (0x0)." comes up. To me, that means the thread is dead, but we were
		// deadlocked in this wait here. So this was turned from a single INFINITE
		// wait to a check on a flag with a timeout. It seems this is somehow 
		// related to IOCP. There was code running that would self-issue another
		// IOCP callback. All this was initiated from the deadlocking thread, but 
		// didn't run on that thread. It felt as if IOCP callbacks would somehow
		// ref the thread to keep it alive AFTER it reports death in the debugger,
		// but before it unblocks itself. Anyway this complexity below made it easy
		// to see by putting a breakpoint on the oTRACE and seeing who was still
		// awake, so I'll leave it in for easier debugging.

		bool Joinable = oThreadIsJoinable(hThread);
		while (Joinable)
		{
			static const DWORD dwTimeoutMS = 1000;
			DWORD dwResult = ::WaitForSingleObject(hThread, dwTimeoutMS);
			Joinable = oThreadIsJoinable(hThread);
			if (Joinable)
				oTRACE("Thread 0x%x still believes it is joinable after %u milliseconds, so go back to waiting...", ID, dwTimeoutMS);
		}

		oTRACE("Thread 0x%x has been joined.", ID);
		hThread = 0;
	}
}

bool oStd::thread::joinable() const
{
	return !!hThread;
}

oStd::thread::id oStd::thread::get_id() const
{
	oStd::thread::id ID;
	unsigned int intID = ::GetThreadId(hThread);
	if (intID)
		*(unsigned int*)&ID = intID;
	return ID;
}

oStd::thread::native_handle_type oStd::thread::native_handle()
{
	return hThread;
}
