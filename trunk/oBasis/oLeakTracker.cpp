// $(header)
#include <oBasis/oLeakTracker.h>
#include <oBasis/oAlgorithm.h>
#include <oBasis/oFor.h>
#include <oBasis/oGUID.h>
#include <oBasis/oStdAtomic.h>
#include <oBasis/oThread.h>
#include <assert.h>

// use lowest-level assert in case this is inside a more robust assert implementation
#define oLEAK_ASSERT(_Expression, _Message) assert((_Expression) && _Message);

oLeakTracker::oLeakTracker(GetCallstackFn _GetCallstack, GetCallstackSymbolStringFn _GetCallstackSymbolString, PrintFn _Print, bool _ReportHexAllocationID, bool _CaptureCallstack, allocations_t::allocator_type _Allocator)
	: InInternalProcesses(false)
	, Allocations(0, allocations_t::hasher(), allocations_t::key_equal(), allocations_t::key_less(), _Allocator)
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

static bool& GetThreadlocalTrackingEnabled()
{
	// has to be a pointer so for multi-module support (all instances of this from
	// a DLL perspective must point to the same bool value)
	thread_local static bool* pThreadlocalTrackingEnabled = nullptr;
	// {410D255E-F3B1-4A37-B511-521627F7341E}
	static const oGUID GUIDEnabled = { 0x410d255e, 0xf3b1, 0x4a37, { 0xb5, 0x11, 0x52, 0x16, 0x27, 0xf7, 0x34, 0x1e } };
	if (!pThreadlocalTrackingEnabled)
	{
		pThreadlocalTrackingEnabled = static_cast<bool*>(oThreadlocalMalloc(GUIDEnabled, sizeof(bool)));
		*pThreadlocalTrackingEnabled = true; // tracking is on by default
	}

	return *pThreadlocalTrackingEnabled;
}

void oLeakTracker::EnableThreadlocalTracking(bool _Enabled) threadsafe
{
	InInternalProcesses = true;
	GetThreadlocalTrackingEnabled() = _Enabled;
	InInternalProcesses = false;
};

void oLeakTracker::OnAllocation(uintptr_t _AllocationID, size_t _Size, const char* _Path, unsigned int _Line, uintptr_t _OldAllocationID) threadsafe
{
	oLockGuard<oRecursiveMutex> Lock(Mutex);
	if (!InInternalProcesses)
	{
		InInternalProcesses = true;
		oLeakTracker* pThis = thread_cast<oLeakTracker*>(this); // protected by mutex above



		std::map<int,int> test;

		test[10] = 10;

		oFindAndErase(test, 10);










		#if oENABLE_RELEASE_ASSERTS == 1 || oENABLE_ASSERTS == 1
			bool erased = 
		#endif
		oFindAndErase((allocations_t::map_type&)pThis->Allocations, _OldAllocationID);
		oASSERT(_OldAllocationID || !erased, "Address already tracked, and this event type is not a reallocation");

		ALLOCATION_DESC d;
		d.AllocationID = _AllocationID;
		d.Size = _Size;
		d.Path = _Path;
		d.Line = _Line;
		d.Context = CurrentContext;
		d.Tracked = GetThreadlocalTrackingEnabled();
		memset(d.StackTrace, 0, sizeof(d.StackTrace));
		if (Desc.CaptureCallstack && pThis->Desc.GetCallstack)
			d.NumStackEntries = static_cast<unsigned int>(pThis->Desc.GetCallstack(d.StackTrace, oCOUNTOF(d.StackTrace), STACK_TRACE_OFFSET));
		else
			d.NumStackEntries = 0;

		pThis->Allocations[_AllocationID] = d;
		InInternalProcesses = false;
	}
}

void oLeakTracker::OnDeallocation(uintptr_t _AllocationID) threadsafe
{
	oLockGuard<oRecursiveMutex> Lock(Mutex);
	if (!InInternalProcesses)
	{
		InInternalProcesses = true;
		oLeakTracker* pThis = thread_cast<oLeakTracker*>(this); // protected by mutex above
		// there may be existing allocs before tracking was enabled, so we're going 
		// to have to ignore those since they weren't captured
		oFindAndErase(pThis->Allocations, _AllocationID);
		InInternalProcesses = false;
	}
}

unsigned int oLeakTracker::GetNumAllocations() const threadsafe
{
	return static_cast<unsigned int>(thread_cast<allocations_t&>(Allocations).size()); // cast is ok because this is an instantaneous sampling
}

void oLeakTracker::Reset() threadsafe
{
	oLockGuard<oRecursiveMutex> Lock(Mutex);
	thread_cast<allocations_t&>(Allocations).clear(); // protected by mutex above
}

bool oLeakTracker::FindAllocation(uintptr_t _AllocationID, ALLOCATION_DESC* _pDesc) threadsafe
{
	oLockGuard<oRecursiveMutex> Lock(Mutex);
	oLeakTracker* pThis = thread_cast<oLeakTracker*>(this); // protected by mutex above
	allocations_t::const_iterator it = pThis->Allocations.find(_AllocationID);
	if (it != pThis->Allocations.end())
	{
		*_pDesc = it->second;
		return true;
	}

	return false;
}

bool oLeakTracker::Report(bool _AllContexts) threadsafe
{
	oStringL buf;
	oStringS memsize;

	oLockGuard<oRecursiveMutex> Lock(Mutex);
	oLeakTracker* pThis = thread_cast<oLeakTracker*>(this); // protected by mutex above

	size_t nLeaks = 0;

	if (pThis->Desc.Print)
	{
		bool headerPrinted = false;
		size_t totalLeakBytes = 0;
		oFOR(const allocations_t::value_type& pair, pThis->Allocations)
		{
			const ALLOCATION_DESC& d = pair.second;
			if (!d.Tracked)
				continue;

			if (!_AllContexts && d.Context != CurrentContext)
				continue;
			
			if (!headerPrinted)
			{
				pThis->Desc.Print("========== Leak Report ==========\n");
				headerPrinted = true;
			}
			
			nLeaks++;
			totalLeakBytes += d.Size;

			oFormatMemorySize(memsize.c_str(), d.Size, 2);

			if (Desc.ReportAllocationIDAsHex)
			{
				if (d.Path && *d.Path)
					sprintf_s(buf, "%s(%u) : {0x%p} %s\n", d.Path, d.Line, d.AllocationID, memsize);
				else
					sprintf_s(buf, "<no filename> : {0x%p} %s (probably a call to ::new(size_t))\n", d.AllocationID, memsize);
			}

			else
			{
				if (d.Path && *d.Path)
					sprintf_s(buf, "%s(%u) : {%d} %s\n", d.Path, d.Line, d.AllocationID, memsize);
				else
					sprintf_s(buf, "<no filename> : {%d} %s (probably a call to ::new(size_t))\n", d.AllocationID, memsize);
			}

			pThis->Desc.Print(buf);

			const bool FilterStdBind = true;

			bool IsStdBind = false;
			for (size_t i = 0; i < d.NumStackEntries; i++)
			{
				bool WasStdBind = IsStdBind;
				pThis->Desc.GetCallstackSymbolString(buf, buf.capacity(), d.StackTrace[i], "  ", &IsStdBind);
				if (!WasStdBind && IsStdBind) // skip a number of the internal wrappers
					i += 5;

				pThis->Desc.Print(buf);
			}
		}

		if (nLeaks)
		{
			oStringS strTotalLeakBytes;
			oFormatMemorySize(strTotalLeakBytes.c_str(), totalLeakBytes, 2);
			oStringM Footer;
			sprintf_s(Footer, "========== Leak Report: %u Leaks %s ==========\n", nLeaks, strTotalLeakBytes);
			pThis->Desc.Print(Footer);
		}
	}

	return nLeaks > 0;
}
