/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
#include <oStd/thread.h>
#include <oStd/atomic.h>
#include <process.h> // _beginthread/_endthread
#include "oWinHeaders.h"

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
{}

static void __cdecl ostd_thread_proc(void* lpdwThreadParam)
{
	oStd::detail::thread_context* ctx = (oStd::detail::thread_context*)lpdwThreadParam;
	ctx->Callable();
	ctx->Callable = nullptr;
	_endthread();
}

void oStd::thread::initialize(const oCALLABLE& _ThreadProc)
{
	Context = std::unique_ptr<oStd::detail::thread_context>(new (_malloc_dbg(sizeof(oStd::detail::thread_context), _CRT_BLOCK, __FILE__, __LINE__)) oStd::detail::thread_context());
	Context->Callable = _ThreadProc;
	Context->hThread = (void*)_beginthread(ostd_thread_proc, 64*1024, Context.get());
}

oStd::thread::~thread()
{
	if (joinable())
	{
		oCRTASSERT(false, "Calling std::terminate because a joinable thread was destroyed");
		std::terminate();
	}
}

oStd::thread& oStd::thread::operator=(thread&& _That)
{
	if (this != &(_That))
	{
		Context = std::move(_That.Context);
	}
	return *this;
}

void std::swap(oStd::thread& _this, oStd::thread& _That)
{
	_this.swap(_That);
}

void oStd::thread::swap(thread& _That)
{
	std::swap(Context, _That.Context);
}

void oStd::thread::detach()
{
	Context = nullptr;
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
	std::unique_ptr<detail::thread_context> LocalContext = std::move(Context);

	if (!!LocalContext)
	{
		// There are cases where a thread is out of the debugger and has exited its
		// Windows-level thread proc, but GetExitCodeThread keeps reporting 
		// STILL_ACTIVE. It seems IOCP can keep a thread on life support, and 
		// perhaps a timing or corruption in other cases. It seems though that 
		// otherwise there are no ill effects as long as the thread is really 
		// finished (perhaps some system-heap memory leakage?) to just exit out.
		int traceCount = 0;
		while (oThreadIsJoinable(LocalContext->hThread))
		{
			static const DWORD dwTimeoutMS = 5000;//INFINITE;
			oStd::thread* pThis = this; // @oooii-tony: This is a weird one...
			DWORD dwResult = WaitForSingleObject(LocalContext->hThread, dwTimeoutMS);
			// @oooii-tony: Wow. WaitForSingleObject can stomp on the this pointer! 
			// This has been going on elsewhere for a while, but hasn't affected much.
			// What am I doing wrong here? I tried pThis->hThread = 0 below, but that 
			// causes other corruption. This might be some weird cross-DLL issue?
			if (pThis != this)
				oCRTTRACE("WaitForSingleObject corrupted this pointer! This happened once when calling Thread.join() directly from a dtor from an object on the stack. Not sure why, but basically move the Thread.join() call to another function and call that from the dtor to work around this issue.");

			if (dwResult == WAIT_TIMEOUT)
			{
				oStd::thread::id ID = get_id();
				if (!LocalContext->Callable)
				{
					oCRTTRACE("Thread '0x%x' exit timed out after %ums, ignoring.", *(unsigned int*)&ID, dwTimeoutMS);
					break;
				}
				oCRTTRACE("Thread '0x%x' still alive after joining for %us.", *(unsigned int*)&ID, (++traceCount) * dwTimeoutMS/1000);
			}
		}
	}
}

bool oStd::thread::joinable() const
{
	return !!Context;
}

oStd::thread::id oStd::thread::get_id() const
{
	oStd::thread::id ID;
	unsigned int intID = Context ? ::GetThreadId(Context->hThread) : 0;
	if (intID)
		*(unsigned int*)&ID = intID;
	return ID;
}

oStd::thread::native_handle_type oStd::thread::native_handle()
{
	return Context ? Context->hThread : 0;
}
