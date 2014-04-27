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
#include <oCore/windows/win_crt_leak_tracker.h>
#include <oCore/debugger.h>
#include <oCore/mutex.h>
#include <oCore/process_heap.h>
#include <oCore/reporting.h>
#include <oCore/windows/win_crt_heap.h>

namespace ouro {
	namespace windows {
		namespace crt_leak_tracker {

static void* untracked_malloc(size_t _Size) { return process_heap::allocate(_Size); }
static void untracked_free(void* _Pointer) { process_heap::deallocate(_Pointer); }

const static size_t kTrackingInternalReserve = oMB(4);

static bool& thread_local_tracking_enabled()
{
	// has to be a pointer so for multi-module support (all instances of this from
	// a DLL perspective must point to the same bool value)
	oTHREAD_LOCAL static bool* pEnabled = nullptr;
	if (!pEnabled)
	{
		process_heap::find_or_allocate(
			"thread_local_tracking_enabled"
			, process_heap::per_thread
			, process_heap::none
			, [=](void* _pMemory) { *(bool*)_pMemory = true; }
			, nullptr
			, &pEnabled);
	}

	return *pEnabled;
}

class context
{
public:
	static context& singleton();

	void enable(bool _Enable);
	inline bool enabled() const { return Enabled; }

	inline void enable_report(bool _Enable) { ReportEnabled = _Enable; }
	inline bool enable_report() { return ReportEnabled; }

	inline void new_context() { lock_t lock(Mutex); pLeakTracker->new_context(); }

	inline void capture_callstack(bool _Capture) { lock_t lock(Mutex); pLeakTracker->capture_callstack(_Capture); }
	inline bool capture_callstack() const { lock_t lock(Mutex); return pLeakTracker->capture_callstack(); }

	void thread_local_tracking(bool _Enable) { lock_t lock(Mutex); pLeakTracker->thread_local_tracking(_Enable); }
	bool thread_local_tracking() { lock_t lock(Mutex); return pLeakTracker->thread_local_tracking(); }

	bool report(bool _CurrentContextOnly);
	inline void reset() { lock_t lock(Mutex); pLeakTracker->reset(); }
	inline void ignore(void* _Pointer) { lock_t lock(Mutex); pLeakTracker->on_deallocate(crt_heap::allocation_id(_Pointer)); }
	
	inline void add_delay() { lock_t lock(Mutex); pLeakTracker->add_delay(); }
	inline void release_delay() { lock_t lock(Mutex); pLeakTracker->release_delay(); }

	int on_malloc_event(int _AllocationType, void* _UserData, size_t _Size, int _BlockType, long _RequestNumber, const unsigned char* _Path, int _Line);

protected:
	context();
	~context();

	static int malloc_hook(int _AllocationType, void* _UserData, size_t _Size, int _BlockType, long _RequestNumber, const unsigned char* _Path, int _Line);
	
	// VS2012's std::mutex calls complex crt functions that grab a global mutex. If called
	// from other crt functions that then call malloc then deadlocks occur, so use an ouro
	// mutex to avoid that issue.
	typedef ouro::mutex mutex_t;
	typedef ouro::lock_guard<mutex_t> lock_t;

	mutable mutex_t Mutex;
	ouro::leak_tracker* pLeakTracker;
	size_t NonLinearBytes;
	_CRT_ALLOC_HOOK OriginalAllocHook;
	bool Enabled;
	bool ReportEnabled;
};

context::context()
	: NonLinearBytes(0)
	, Enabled(false)
	, OriginalAllocHook(nullptr)
{
	ouro::reporting::ensure_initialized();

	// This touches conCRT to ensure the lib is loaded. If not it can grab the CRT mutex
	// and can do so from the leak tracker inside an alloc, thus causing a deadlock.
	//{
	//	recursive_mutex m;
	//	m.lock();
	//	m.unlock();
	//}

	leak_tracker::info lti;
	lti.allocate = untracked_malloc;
	lti.deallocate = untracked_free;
	lti.thread_local_tracking_enabled = thread_local_tracking_enabled;
	lti.callstack = debugger::callstack;
	lti.format = debugger::format;
	lti.print = debugger::print;
	pLeakTracker = new leak_tracker(lti);
}

context::~context()
{
	report(false);
	_CrtSetAllocHook(OriginalAllocHook);

	if (NonLinearBytes)
	{
		mstring buf;
		format_bytes(buf, NonLinearBytes, 2);
		oTRACE("CRT leak tracker: Allocated %s beyond the internal reserve. Increase kTrackingInternalReserve to improve performance, especially on shutdown.", buf.c_str());
	}
}

context& context::singleton()
{
	static context* sInstance = nullptr;
	if (!sInstance)
	{
		process_heap::find_or_allocate(
			"crt_leak_tracker context"
			, process_heap::per_process
			, process_heap::none
			, [=](void* _pMemory) { new (_pMemory) context(); }
			, [=](void* _pMemory) { ((context*)_pMemory)->~context(); }
			, &sInstance);
	}
	return *sInstance;
}

void context::enable(bool _Enable)
{
	if (_Enable && !Enabled)
		OriginalAllocHook = _CrtSetAllocHook(malloc_hook);
	else if (!_Enable && Enabled)
	{
		_CrtSetAllocHook(OriginalAllocHook);
		OriginalAllocHook = nullptr;
	}

	Enabled = _Enable;
}

bool context::report(bool _CurrentContextOnly)
{
	size_t nLeaks = 0;
	if (ReportEnabled)
	{
		lock_t lock(Mutex);
		bool OldValue = enabled();
		enable(false);
		nLeaks = pLeakTracker->report(_CurrentContextOnly);
		enable(OldValue);
	}

	return nLeaks > 0;
}

int context::on_malloc_event(int _AllocationType, void* _UserData, size_t _Size, int _BlockType, long _RequestNumber, const unsigned char* _Path, int _Line)
{
	int allowAllocationToProceed = 1;

	// Call any prior hook first
	if (OriginalAllocHook)
		allowAllocationToProceed = OriginalAllocHook(_AllocationType, _UserData, _Size, _BlockType, _RequestNumber, _Path, _Line);

	if (allowAllocationToProceed && _BlockType != _IGNORE_BLOCK && _BlockType != _CRT_BLOCK)
	{
		lock_t lock(Mutex);
		switch (_AllocationType)
		{
			case _HOOK_ALLOC: pLeakTracker->on_allocate(static_cast<unsigned int>(_RequestNumber), _Size, (const char*)_Path, _Line); break;
			case _HOOK_REALLOC: pLeakTracker->on_allocate(static_cast<unsigned int>(_RequestNumber), _Size, (const char*)_Path, _Line, windows::crt_heap::allocation_id(_UserData)); break;
			case _HOOK_FREE: pLeakTracker->on_deallocate(windows::crt_heap::allocation_id(_UserData)); break;
			default: __assume(0);
		}
	}

	return allowAllocationToProceed;
}

int context::malloc_hook(int _AllocationType, void* _UserData, size_t _Size, int _BlockType, long _RequestNumber, const unsigned char* _Path, int _Line)
{
	return context::singleton().on_malloc_event(_AllocationType, _UserData, _Size, _BlockType, _RequestNumber, _Path, _Line);
}

void ensure_initialized()
{
	context::singleton();
}
 
void enable(bool _Enable)
{
	context::singleton().enable(_Enable);
}
 
bool enabled()
{
	return context::singleton().enabled();
}

void enable_report(bool _Enable)
{
	context::singleton().enable_report(_Enable);
}

bool enable_report()
{
	return context::singleton().enable_report();
}

void new_context()
{
	context::singleton().new_context();
}

void capture_callstack(bool _Capture)
{
	context::singleton().capture_callstack(_Capture);
}

bool capture_callstack()
{
	return context::singleton().capture_callstack();
}

void thread_local_tracking(bool _Enable)
{
	context::singleton().thread_local_tracking(_Enable);
}

bool thread_local_tracking()
{
	return context::singleton().thread_local_tracking();
}

bool report(bool _CurrentContextOnly)
{
	return context::singleton().report(_CurrentContextOnly);
}

void reset()
{
	context::singleton().reset();
}

void ignore(void* _Pointer)
{
	context::singleton().ignore(_Pointer);
}

void add_delay()
{
	context::singleton().add_delay();
}

void release_delay()
{
	context::singleton().release_delay();
}

		} // namespace crt_leak_tracker
	} // namespace windows
} // namespace ouro
