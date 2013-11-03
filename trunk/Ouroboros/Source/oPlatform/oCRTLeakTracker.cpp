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
#include "oCRTLeakTracker.h"
#include <oPlatform/Windows/oCRTHeap.h>
#include <oPlatform/oProcessHeap.h>
#include <oPlatform/oReporting.h>

using namespace ouro;

// {F253EA65-29FC-47D0-9E2E-400DAC41D861}
const oGUID oCRTLeakTracker::GUID = { 0xf253ea65, 0x29fc, 0x47d0, { 0x9e, 0x2e, 0x40, 0xd, 0xac, 0x41, 0xd8, 0x61 } };
oSINGLETON_REGISTER(oCRTLeakTracker);

static void* untracked_malloc(size_t _Size) { return process_heap::allocate(_Size); }
static void untracked_free(void* _Pointer) { process_heap::deallocate(_Pointer); }

const static size_t kTrackingInternalReserve = oMB(4);

static bool& GetThreadlocalTrackingEnabled()
{
	// has to be a pointer so for multi-module support (all instances of this from
	// a DLL perspective must point to the same bool value)
	thread_local static bool* pThreadlocalTrackingEnabled = nullptr;
	if (!pThreadlocalTrackingEnabled)
	{
		process_heap::find_or_allocate(
			"threadlocal_tracking_enabled"
			, process_heap::per_thread
			, process_heap::none
			, [=](void* _pMemory) { *(bool*)_pMemory = true; }
			, nullptr
			, &pThreadlocalTrackingEnabled);
	}

	return *pThreadlocalTrackingEnabled;
}

oCRTLeakTracker::oCRTLeakTracker()
	: NonLinearBytes(0)
	, Enabled(false)
	, OriginalAllocHook(nullptr)
{
	oReportingReference(); // reporting keeps a log file... don't track that

	leak_tracker::info lti;
	lti.allocate = untracked_malloc;
	lti.deallocate = untracked_free;
	lti.thread_local_tracking_enabled = GetThreadlocalTrackingEnabled;
	lti.callstack = debugger::callstack;
	lti.format = debugger::format;
	lti.print = debugger::print;
	pLeakTracker = new leak_tracker(lti);

	sInstanceForDeferredRelease = this;
	Reference(); // keep an extra references to ourselves so that malloc is always hooked
	atexit(AtExit); // then free it at the very end
}

oCRTLeakTracker::~oCRTLeakTracker()
{
	ReportLeaks(false);
	_CrtSetAllocHook(OriginalAllocHook);

	if (NonLinearBytes)
	{
		mstring buf;
		format_bytes(buf, NonLinearBytes, 2);
		oTRACE("CRT Leak Tracker: Allocated %s beyond the internal reserve. Increase kTrackingInternalReserve to improve performance, especially on shutdown.", buf.c_str());
	}

	oReportingRelease();
}

oCRTLeakTracker* oCRTLeakTracker::sInstanceForDeferredRelease = nullptr;
void oCRTLeakTracker::AtExit()
{
	if (sInstanceForDeferredRelease)
		sInstanceForDeferredRelease->Release();
}

void oCRTLeakTracker::Enable(bool _Enabled)
{
	if (_Enabled && !Enabled)
		OriginalAllocHook = _CrtSetAllocHook(MallocHook);
	else if (!_Enabled && Enabled)
	{
		_CrtSetAllocHook(OriginalAllocHook);
		OriginalAllocHook = nullptr;
	}

	Enabled = _Enabled;
}

bool oCRTLeakTracker::IsEnabled() const
{
	return Enabled;
}

void oCRTLeakTracker::Report(bool _Report)
{
	ReportEnabled = _Report;
}

bool oCRTLeakTracker::IsReportEnabled() const
{
	return ReportEnabled;
}

bool oCRTLeakTracker::ReportLeaks(bool _CurrentContextOnly)
{
	size_t nLeaks = 0;
	if (ReportEnabled)
	{
		bool OldValue = IsEnabled();
		Enable(false);
		nLeaks = pLeakTracker->report(_CurrentContextOnly);
		Enable(OldValue);
	}

	return nLeaks > 0;
}

int oCRTLeakTracker::OnMallocEvent(int _AllocationType, void* _UserData, size_t _Size, int _BlockType, long _RequestNumber, const unsigned char* _Path, int _Line)
{
	int allowAllocationToProceed = 1;

	// Call any prior hook first
	if (OriginalAllocHook)
		allowAllocationToProceed = OriginalAllocHook(_AllocationType, _UserData, _Size, _BlockType, _RequestNumber, _Path, _Line);

	if (allowAllocationToProceed && _BlockType != _IGNORE_BLOCK && _BlockType != _CRT_BLOCK)
	{
		switch (_AllocationType)
		{
			case _HOOK_ALLOC:
				pLeakTracker->on_allocate(static_cast<unsigned int>(_RequestNumber), _Size, (const char*)_Path, _Line);
				break;
			case _HOOK_REALLOC:
				pLeakTracker->on_allocate(static_cast<unsigned int>(_RequestNumber), _Size, (const char*)_Path, _Line, oCRTHeapGetAllocationID(_UserData));
				break;
			case _HOOK_FREE:
				pLeakTracker->on_deallocate(oCRTHeapGetAllocationID(_UserData));
				break;
			default:
				__assume(0);
		}
	}

	return allowAllocationToProceed;
}

int oCRTLeakTracker::MallocHook(int _AllocationType, void* _UserData, size_t _Size, int _BlockType, long _RequestNumber, const unsigned char* _Path, int _Line)
{
	// Use this deferred copy rather than Singleton() because that static pointer
	// can be destroyed.
	if (!sInstanceForDeferredRelease)
	{
		sInstanceForDeferredRelease = Singleton();
		sInstanceForDeferredRelease->Reference();
		atexit(AtExit);
	}

	return sInstanceForDeferredRelease->OnMallocEvent(_AllocationType, _UserData, _Size, _BlockType, _RequestNumber, _Path, _Line);
}

void oCRTLeakTracker::UntrackAllocation(void* _Pointer)
{
	pLeakTracker->on_deallocate(oCRTHeapGetAllocationID(_Pointer));
}

void oConcurrency::enable_leak_tracking_threadlocal(bool _Enabled)
{
	oCRTLeakTracker::Singleton()->EnableThreadlocalTracking(_Enabled);
}
