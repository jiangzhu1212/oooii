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
#include <oBasis/oLeakTracker.h>
#include <oBase/algorithm.h>
#include <oBasis/oBasisRequirements.h>
#include <oBasis/oLockThis.h>
#include <oStd/for.h>
#include <oStd/atomic.h>
#include <oStd/chrono.h>
#include <cassert>

using namespace ouro;

// use lowest-level assert in case this is inside a more robust assert implementation
#define oLEAK_ASSERT(_Expression, _Message) assert((_Expression) && _Message);

oLeakTracker::oLeakTracker(GetCallstackFn _GetCallstack, GetCallstackSymbolStringFn _GetCallstackSymbolString, PrintFn _Print, bool _ReportHexAllocationID, bool _CaptureCallstack, allocations_t::allocator_type _Allocator)
	: InInternalProcesses(false)
	, Allocations(0, allocations_t::hasher(), allocations_t::key_equal(), allocations_t::key_less(), _Allocator)
	, DelayLatch(1)
	, CurrentContext(0)
{
	Desc.GetCallstack = _GetCallstack;
	Desc.GetCallstackSymbolString = _GetCallstackSymbolString;
	Desc.Print = _Print;
	Desc.ReportAllocationIDAsHex = _ReportHexAllocationID;
	Desc.CaptureCallstack = _CaptureCallstack;
}

oLeakTracker::oLeakTracker(const DESC& _Desc, allocations_t::allocator_type _Allocator)
	: Desc(_Desc)
	, InInternalProcesses(false)
	, Allocations(0, allocations_t::hasher(), allocations_t::key_equal(), allocations_t::key_less(), _Allocator)
	, DelayLatch(1)
	, CurrentContext(0)
{
}

oLeakTracker::~oLeakTracker()
{
}

void oLeakTracker::CaptureCallstack(bool _Capture) threadsafe
{
	oStd::atomic_exchange(&Desc.CaptureCallstack, _Capture);
}

void oLeakTracker::NewContext() threadsafe
{
	oStd::atomic_increment(&CurrentContext);
}

static int& GetThreadlocalTrackingEnabled()
{
	// has to be a pointer so for multi-module support (all instances of this from
	// a DLL perspective must point to the same bool value)
	thread_local static int* pThreadlocalTrackingEnabled = nullptr;
	// {410D255E-F3B1-4A37-B511-521627F7341E}
	static const guid GUIDEnabled = { 0x410d255e, 0xf3b1, 0x4a37, { 0xb5, 0x11, 0x52, 0x16, 0x27, 0xf7, 0x34, 0x1e } };
	if (!pThreadlocalTrackingEnabled)
	{
		oThreadlocalMalloc(GUIDEnabled
			, [&](void* _pMemory) { *(int*)_pMemory = 1; } // tracking is on by default
			, oLIFETIME_TASK()
			, &pThreadlocalTrackingEnabled);
	}

	return *pThreadlocalTrackingEnabled;
}

void oLeakTracker::EnableThreadlocalTracking(bool _Enabled) threadsafe
{
	oConcurrency::lock_guard<oConcurrency::recursive_mutex> Lock(Mutex);
	InInternalProcesses = true;
	int& enabled_counter = GetThreadlocalTrackingEnabled();
	if (_Enabled)
		enabled_counter += 1;
	else
		enabled_counter -= 1;
	oASSERT(enabled_counter <= 1, "EnableThreadlocalTracking(true) should only be called after calling EnableThreadlocalTracking(false)");
	InInternalProcesses = false;
};

void oLeakTracker::OnAllocation(uintptr_t _AllocationID, size_t _Size, const char* _Path, unsigned int _Line, uintptr_t _OldAllocationID) threadsafe
{
	oConcurrency::lock_guard<oConcurrency::recursive_mutex> Lock(Mutex);
	if (!InInternalProcesses)
	{
		InInternalProcesses = true;

		#if oENABLE_RELEASE_ASSERTS == 1 || oENABLE_ASSERTS == 1
			bool erased = 
		#endif
		find_and_erase((allocations_t::map_type&)This()->Allocations, _OldAllocationID);
		oASSERT(_OldAllocationID || !erased, "Address already tracked, and this event type is not a reallocation");

		ALLOCATION_DESC d;
		d.AllocationID = _AllocationID;
		d.Size = _Size;
		d.Path = _Path;
		d.Line = _Line;
		d.Context = CurrentContext;
		d.Tracked = GetThreadlocalTrackingEnabled() > 0;
		memset(d.StackTrace, 0, sizeof(d.StackTrace));
		if (Desc.CaptureCallstack && This()->Desc.GetCallstack)
			d.NumStackEntries = static_cast<unsigned int>(This()->Desc.GetCallstack(d.StackTrace, oCOUNTOF(d.StackTrace), STACK_TRACE_OFFSET));
		else
			d.NumStackEntries = 0;

		This()->Allocations[_AllocationID] = d;
		InInternalProcesses = false;
	}
}

void oLeakTracker::OnDeallocation(uintptr_t _AllocationID) threadsafe
{
	oConcurrency::lock_guard<oConcurrency::recursive_mutex> Lock(Mutex);
	if (!InInternalProcesses)
	{
		InInternalProcesses = true;
		// there may be existing allocs before tracking was enabled, so we're going 
		// to have to ignore those since they weren't captured
		find_and_erase(This()->Allocations, _AllocationID);
		InInternalProcesses = false;
	}
}

unsigned int oLeakTracker::GetNumAllocations() const threadsafe
{
	return static_cast<unsigned int>(This()->Allocations.size()); // cast is ok because this is an instantaneous sampling
}

void oLeakTracker::Reset() threadsafe
{
	oLockThis(Mutex)->Allocations.clear();
}

bool oLeakTracker::ShouldReport(const ALLOCATION_DESC& _Desc, bool _CurrentContextOnly) const
{
	return _Desc.Tracked && (!_CurrentContextOnly || _Desc.Context == CurrentContext);
}

bool oLeakTracker::FindAllocation(uintptr_t _AllocationID, ALLOCATION_DESC* _pDesc) threadsafe
{
	// @oooii-tony: Should this return false for blacklisted/out-of-context allocs?

	oConcurrency::lock_guard<oConcurrency::recursive_mutex> Lock(Mutex);
	allocations_t::const_iterator it = This()->Allocations.find(_AllocationID);
	if (it != This()->Allocations.end())
	{
		*_pDesc = it->second;
		return true;
	}

	return false;
}

unsigned int oLeakTracker::InternalReportLeaks(bool _TraceReport, bool _WaitForAsyncAllocs, bool _CurrentContextOnly) const threadsafe
{
	xlstring buf;
	sstring memsize;

	This()->DelayLatch.release(); // inherently threadsafe API
	if (oStd::cv_status::timeout == This()->DelayLatch.wait_for(oStd::chrono::milliseconds(5000))) // some delayed frees might be in threads that get stomped on (thus no exit code runs) during static deinit, so don't wait forever
		oTRACE("WARNING: a delay on the leak report count was added, but has yet to be released. The timeout has been reached, so this report will include delayed releases that haven't (yet) occurred.");

	bool RecoveredFromAsyncLeaks = false;
	// Some 3rd party task systems (TBB) may not have good API for ensuring they
	// are in a steady-state going into a leak report. To limit the number of 
	// false-positives, check again if there are leaks.
	if (_WaitForAsyncAllocs)
	{
		unsigned int CheckedNumLeaks = InternalReportLeaks(false, false, _CurrentContextOnly);
		if (CheckedNumLeaks > 0)
		{
			oTRACE("There are potentially %u leaks(s), sleeping and checking again to eliminate async false positives...", CheckedNumLeaks);
			oStd::this_thread::sleep_for(oStd::chrono::milliseconds(1000));
			RecoveredFromAsyncLeaks = true;
		}
	}

	oConcurrency::lock_guard<oConcurrency::recursive_mutex> Lock(This()->Mutex);
	unsigned int nLeaks = 0;

	if (This()->Desc.Print)
	{
		bool headerPrinted = false;
		size_t totalLeakBytes = 0;
		oFOR(const allocations_t::value_type& pair, This()->Allocations)
		{
			const ALLOCATION_DESC& d = pair.second;
			if (!This()->ShouldReport(d, _CurrentContextOnly))
				continue;

			if (!headerPrinted && _TraceReport)
			{
				mstring Header;
				snprintf(Header, "========== Leak Report%s ==========\n", RecoveredFromAsyncLeaks ? " (recovered from async false positives)" : "");
				This()->Desc.Print(Header);
				headerPrinted = true;
			}
			
			nLeaks++;
			totalLeakBytes += d.Size;

			if (_TraceReport)
			{
				format_bytes(memsize, d.Size, 2);

				if (Desc.ReportAllocationIDAsHex)
				{
					if (d.Path && *d.Path)
						snprintf(buf, "%s(%u) : {0x%p} %s\n", d.Path, d.Line, d.AllocationID, memsize.c_str());
					else
						snprintf(buf, "<no filename> : {0x%p} %s (probably a call to ::new(size_t))\n", d.AllocationID, memsize.c_str());
				}

				else
				{
					if (d.Path && *d.Path)
						snprintf(buf, "%s(%u) : {%d} %s\n", d.Path, d.Line, d.AllocationID, memsize);
					else
						snprintf(buf, "<no filename> : {%d} %s (probably a call to ::new(size_t))\n", d.AllocationID, memsize.c_str());
				}

				This()->Desc.Print(buf);

				const bool FilterStdBind = true;

				bool IsStdBind = false;
				for (size_t i = 0; i < d.NumStackEntries; i++)
				{
					bool WasStdBind = IsStdBind;
					This()->Desc.GetCallstackSymbolString(buf, buf.capacity(), d.StackTrace[i], "  ", &IsStdBind);
					if (!WasStdBind && IsStdBind) // skip a number of the internal wrappers
						i += 5;

					This()->Desc.Print(buf);
				}
			}
		}

		if (nLeaks && _TraceReport)
		{
			sstring strTotalLeakBytes;
			mstring Footer;
			format_bytes(strTotalLeakBytes, totalLeakBytes, 2);
			snprintf(Footer, "========== Leak Report: %u Leak(s) %s%s ==========\n", nLeaks, strTotalLeakBytes.c_str(), _WaitForAsyncAllocs ? " (recovered from async false positives)" : "");
			This()->Desc.Print(Footer);
		}
	}

	This()->DelayLatch.reset(1);
	return nLeaks;
}