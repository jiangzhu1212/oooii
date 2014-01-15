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
#include <process.h> // _beginthread/_endthread
#include "crt_assert.h"
#include <stdexcept>

namespace ouro {

thread::id this_thread::get_id()
{
	thread::id ID;
	*(unsigned int*)&ID = ::GetCurrentThreadId();
	return ID;
}

void this_thread::yield()
{
	#if defined(_WIN32) || defined(_WIN64)
		::SwitchToThread();
	#else
		#error Unsupported platform
	#endif
}

void detail::thread_context::sleep_for(unsigned int _Milliseconds)
{
	::Sleep(_Milliseconds);
}

thread::id::id()
	: ID(UINT_MAX)
{}

unsigned int thread::hardware_concurrency()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
}

thread::thread()
{}

static void __cdecl ostd_thread_proc(void* lpdwThreadParam)
{
	detail::thread_context* ctx = (detail::thread_context*)lpdwThreadParam;
	ctx->Callable();
	ctx->Callable = nullptr;
	_endthread();
}

void thread::initialize(const std::function<void()>& _ThreadProc)
{
	Context = std::unique_ptr<detail::thread_context>(new (_malloc_dbg(sizeof(detail::thread_context), _CRT_BLOCK, __FILE__, __LINE__)) detail::thread_context());
	Context->Callable = _ThreadProc;
	Context->hThread = (void*)_beginthread(ostd_thread_proc, 64*1024, Context.get());
}

thread::~thread()
{
	if (joinable())
	{
		oCRTASSERT(false, "Calling std::terminate because a joinable thread was destroyed");
		std::terminate();
	}
}

thread& thread::operator=(thread&& _That)
{
	if (this != &(_That))
	{
		Context = std::move(_That.Context);
	}
	return *this;
}

void thread::swap(thread& _That)
{
	std::swap(Context, _That.Context);
}

void thread::detach()
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

void thread::join()
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
			thread* pThis = this; // @tony: This is a weird one...
			DWORD dwResult = WaitForSingleObject(LocalContext->hThread, dwTimeoutMS);
			// @tony: Wow. WaitForSingleObject can stomp on the this pointer! 
			// This has been going on elsewhere for a while, but hasn't affected much.
			// What am I doing wrong here? I tried pThis->hThread = 0 below, but that 
			// causes other corruption. This might be some weird cross-DLL issue?
			if (pThis != this)
				oCRTTRACE("WaitForSingleObject corrupted this pointer! This happened once when calling Thread.join() directly from a dtor from an object on the stack. Not sure why, but basically move the Thread.join() call to another function and call that from the dtor to work around this issue.");

			if (dwResult == WAIT_TIMEOUT)
			{
				thread::id ID = get_id();
				if (!LocalContext->Callable)
				{
					oCRTTRACE("Thread '0x%x' exit timed out after %ums, ignoring.", *(unsigned int*)&ID, dwTimeoutMS);
					break;
				}
				oCRTTRACE("Thread '0x%x' still alive after joining for %us.", *(unsigned int*)&ID, (++traceCount) * dwTimeoutMS/1000);
			}
		}
	}
	else
		throw std::invalid_argument("invalid argument (join called on a non-joinable thread)"); // this is what VS2012 does if join is called on a non-joinable thread
}

bool thread::joinable() const
{
	return !!Context;
}

thread::id thread::get_id() const
{
	thread::id ID;
	unsigned int intID = Context ? ::GetThreadId(Context->hThread) : 0;
	if (intID)
		*(unsigned int*)&ID = intID;
	return ID;
}

thread::native_handle_type thread::native_handle()
{
	return Context ? Context->hThread : 0;
}

} // namespace ouro

void std::swap(ouro::thread& _This, ouro::thread& _That)
{
	_This.swap(_That);
}
