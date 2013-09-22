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
// An interface for hooking a malloc routine to report allocations unmatched by
// deallocation and report the file line and callstack of where the allocation
// occurred.
#pragma once
#ifndef oLeakTracker_h
#define oLeakTracker_h

#include <oConcurrency/countdown_latch.h>
#include <oConcurrency/mutex.h>
#include <oCore/debugger.h>
#include <oBase/fixed_string.h>
#include <oBasis/oStdLinearAllocator.h>
#include <oBase/unordered_map.h>
#include <oBase/fnv1a.h>
#include <oBase/function.h>

class oLeakTracker
{
public:
	static const size_t STACK_TRACE_DEPTH = 32;
	static const size_t STACK_TRACE_OFFSET = 8;

	// Function returns the number of callstack symbols retrieved. _StartingOffset
	// allows ignoring a number of entries just before the call location since
	// typically a higher-level malloc call might have a known number of 
	// sub-functions it calls. _pSymbols receives the values up to _MaxNumSymbols.
	typedef std::function<size_t(ouro::debugger::symbol* _pSymbols, size_t _MaxNumSymbols, size_t _StartingOffset)> GetCallstackFn;

	// Function that behaves like snprintf but converts the specified symbol into
	// a string fit for the PrintFn below. The function should concatenate the
	// specified string after the specified _PrefixString so indentation can 
	// occur. The function should be able to identify std::bind internal 
	// symbols and fill the specified bool with true if it matches or false if it
	// doesn't and the function should noop if the value is true going in, that
	// way long callstacks of std::bind internals can be shortened.
	typedef std::function<int(char* _Buffer, size_t _SizeofBuffer, ouro::debugger::symbol _Symbol, const char* _PrefixString, bool* _pIsStdBind)> GetCallstackSymbolStringFn;

	// Print a fixed string to some underlying destination. This makes no 
	// assumptions about the string itself, and should add nothing to the string,
	// such as an automatic newline or the like.
	typedef std::function<void(const char* _String)> PrintFn;

	struct DESC
	{
		// The cross-platform logic in this class requires several platform calls to 
		// effectively track memory allocations, so rather than pollute too much of
		// this code with only a little platform dependencies, encapsulate those 
		// requirements in these calls here.

		bool ReportAllocationIDAsHex;
		bool CaptureCallstack; // this can be modified later with CaptureCallstack()
		GetCallstackFn GetCallstack;
		GetCallstackSymbolStringFn GetCallstackSymbolString;
		PrintFn Print;
	};

	struct ALLOCATION_DESC
	{
		uintptr_t AllocationID;
		size_t Size;
		ouro::debugger::symbol StackTrace[STACK_TRACE_DEPTH];
		unsigned int NumStackEntries;
		unsigned int Line;
		unsigned int Context;
		ouro::path_string	Path;
		bool Tracked; // true if the allocation occurred when tracking wasn't enabled
		inline bool operator==(const ALLOCATION_DESC& _Other) { return AllocationID == _Other.AllocationID; }
	};

	// Type of container exposed so we can pass in an allocator.
	static struct HashAllocation { size_t operator()(uintptr_t _AllocationID) const { return ouro::fnv1a<size_t>(&_AllocationID, sizeof(_AllocationID)); } };
	
	typedef std::pair<const uintptr_t, ALLOCATION_DESC> pair_type;
	typedef oStdLinearAllocator<pair_type> allocator_type;
	typedef ouro::unordered_map<uintptr_t, ALLOCATION_DESC, HashAllocation, std::equal_to<uintptr_t>, std::less<uintptr_t>, allocator_type> allocations_t;

	oLeakTracker(const DESC& _Desc, allocations_t::allocator_type _Allocator /*= allocations_t::allocator_type()*/);
	oLeakTracker(GetCallstackFn _GetCallstack, GetCallstackSymbolStringFn _GetCallstackSymbolString, PrintFn _Print, bool _ReportHexAllocationID, bool _CaptureCallstack, allocations_t::allocator_type _Allocator /*= allocations_t::allocator_type()*/);
	~oLeakTracker();

	allocations_t::allocator_type GetAllocator() { return Allocations.get_allocator(); }

	// Capturing the callstack for each alloc can be slow, so provide a way to
	// turn it on or off. An effective use would be to start an application up to 
	// a point where leaks are likely, bypassing an expensive capture for static 
	// init and asset loading. Then in the suspected area start logging more 
	// explicit data.
	void CaptureCallstack(bool _Capture = true) threadsafe;

	// Call this when an allocation occurs. If a realloc, pass the ID of the 
	// original pointer to _OldAllocationID.
	void OnAllocation(uintptr_t _AllocationID, size_t _Size, const char* _Path, unsigned int _Line, uintptr_t _OldAllocationID = 0) threadsafe;

	// Call this when a deallocation occurs
	void OnDeallocation(uintptr_t _AllocationID) threadsafe;

	// Returns the number of allocations at the time of the calls.
	unsigned int GetNumAllocations() const threadsafe;

	// Clears tracked allocations. This can be useful when doing targetted 
	// debugging of a leak known to be in a specific area. However this is not
	// context-based: this will erase all prior history.
	void Reset() threadsafe;

	// Returns true if _pDesc was filled with valid information, false if the 
	// allocation was not found as being recorded.
	bool FindAllocation(uintptr_t _AllocationID, ALLOCATION_DESC* _pDesc) threadsafe;

	// Use the specified function to iterate through all outstanding allocations
	// and print useful information about the leak. If _pTotalLeakedBytes is 
	// valid, it will be filled with the sum total of all bytes leaked. This 
	// returns true if there are any leaks, false if there are no leaks.

	// Iterates all allocations and returns the number of tracked leaks detected.
	// If _CurrentContextOnly is true, then only the allocations since the last 
	// call to NewContext() will be reported. If _CurrentContextOnly is false, all 
	// allocations since the start of tracking will be reported.
	unsigned int CalculateNumLeaks(bool _CurrentContextOnly = true) const threadsafe { return InternalReportLeaks(false, true, _CurrentContextOnly); }

	// Iterates all allocations and reports to the Print function each tracked 
	// allocation. If _CurrentContextOnly is true, then only the allocations since
	// the last call to NewContext() will be reported. If _CurrentContextOnly is 
	// false, all allocations since the start of tracking will be reported. This 
	// returns the number of leaks reported.
	unsigned int Report(bool _CurrentContextOnly = true) const threadsafe { return InternalReportLeaks(true, true, _CurrentContextOnly); }

	// Enables (or disables) memory tracking for the calling thread. This can be
	// useful in shutting down any tracking done during calls to 3rd party APIs
	// (lookin' at you TBB!).
	void EnableThreadlocalTracking(bool _Enabled = true) threadsafe;

	// Useful in tracking leaks around known points in the code, such as between
	// unit tests or level loads.
	void NewContext() threadsafe;

	void ReferenceDelay() const threadsafe { This()->DelayLatch.reference(); }
	void ReleaseDelay() const threadsafe { This()->DelayLatch.release(); }

private:
	allocations_t Allocations;

	bool InInternalProcesses; // don't track allocations this object itself makes just to do the tracking (trust that this won't leak too!)
	DESC Desc;
	oConcurrency::recursive_mutex Mutex;
	oConcurrency::countdown_latch DelayLatch;
	unsigned int CurrentContext;

	// Returns false for a blacklisted or out-of-context allocation 
	bool ShouldReport(const ALLOCATION_DESC& _Desc, bool _CurrentContextOnly = true) const;
	inline oLeakTracker* This() const threadsafe { return thread_cast<oLeakTracker*>(this); }

	unsigned int InternalReportLeaks(bool _TraceReport, bool _WaitForAsyncAllocs, bool _CurrentContextOnly) const threadsafe;
};

#endif
