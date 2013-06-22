/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#include <oBasis/oString.h>
#include <oPlatform/oDebugger.h>
#include <oPlatform/Windows/oCRTHeap.h>
#include <oPlatform/oProcessHeap.h>
#include <oPlatform/oReporting.h>

// {F253EA65-29FC-47D0-9E2E-400DAC41D861}
const oGUID oCRTLeakTracker::GUID = { 0xf253ea65, 0x29fc, 0x47d0, { 0x9e, 0x2e, 0x40, 0xd, 0xac, 0x41, 0xd8, 0x61 } };
oSINGLETON_REGISTER(oCRTLeakTracker);

static void* untracked_malloc(size_t _Size) { return oProcessHeapAllocate(_Size); }
static void untracked_free(void* _Pointer) { oProcessHeapDeallocate(_Pointer); }

const static size_t kTrackingInternalReserve = oMB(4);

oCRTLeakTracker::oCRTLeakTracker()
	: NonLinearBytes(0)
	, Enabled(false)
	, OriginalAllocHook(nullptr)
{
	oReportingReference(); // reporting keeps a log file... don't track that

	pLeakTracker = new oLeakTracker(oDebuggerGetCallstack, oDebuggerSymbolSPrintf, oDebuggerPrint, false, false
		, oStdLinearAllocator<oLeakTracker::allocations_t::value_type>(untracked_malloc(kTrackingInternalReserve)
			, kTrackingInternalReserve, &NonLinearBytes, untracked_malloc, untracked_free));

	sInstanceForDeferredRelease = this;
	Reference(); // keep an extra references to ourselves so that malloc is always hooked
	atexit(AtExit); // then free it at the very end
}

oCRTLeakTracker::~oCRTLeakTracker()
{
	ReportLeaks(false);
	_CrtSetAllocHook(OriginalAllocHook);

	oLeakTracker::allocations_t::allocator_type a = pLeakTracker->GetAllocator();
	delete pLeakTracker;
	untracked_free(a.pAllocator);

	if (NonLinearBytes)
	{
		char buf[128];
		oFormatMemorySize(buf, NonLinearBytes, 2);
		oTRACE("CRT Leak Tracker: Allocated %s beyond the internal reserve. Increase kTrackingInternalReserve to improve performance, especially on shutdown.", buf);
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
	unsigned int nLeaks = 0;
	if (ReportEnabled)
	{
		bool OldValue = IsEnabled();
		Enable(false);
		nLeaks = pLeakTracker->Report(_CurrentContextOnly);
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
				pLeakTracker->OnAllocation(static_cast<uintptr_t>(_RequestNumber), _Size, (const char*)_Path, _Line);
				break;
			case _HOOK_REALLOC:
				pLeakTracker->OnAllocation(static_cast<uintptr_t>(_RequestNumber), _Size, (const char*)_Path, _Line, oCRTHeapGetAllocationID(_UserData));
				break;
			case _HOOK_FREE:
				pLeakTracker->OnDeallocation(oCRTHeapGetAllocationID(_UserData));
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
	pLeakTracker->OnDeallocation(oCRTHeapGetAllocationID(_Pointer));
}

void oConcurrency::enable_leak_tracking_threadlocal(bool _Enabled)
{
	oCRTLeakTracker::Singleton()->EnableThreadlocalTracking(_Enabled);
}