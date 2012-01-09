// $(header)
// An interface for hooking a malloc routine to report allocations unmatched by
// deallocation and report the file line and callstack of where the allocation
// occurred.
#pragma once
#ifndef oLeakTracker_h
#define oLeakTracker_h

#include <oBasis/oFixedString.h>
#include <oBasis/oFunction.h>
#include <oBasis/oLinearAllocator.h>
#include <oBasis/oMutex.h>
#include <oBasis/oUnorderedMap.h>

class oLeakTracker
{
public:
	static const size_t STACK_TRACE_DEPTH = 32;
	static const size_t STACK_TRACE_OFFSET = 8;

	// Function returns the number of callstack symbols retrieved. _StartingOffset
	// allows ignoring a number of entries just before the call location since
	// typically a higher-level malloc call might have a known number of 
	// sub-functions it calls. _pSymbols receives the values up to _MaxNumSymbols.
	typedef oFUNCTION<size_t(unsigned long long* _pSymbols, size_t _MaxNumSymbols, size_t _StartingOffset)> GetCallstackFn;

	// Function that behaves like snprintf but converts the specified symbol into
	// a string fit for the PrintFn below. The function should concatenate the
	// specified string after the specified _PrefixString so indentation can 
	// occur. The function should be able to identify std::bind internal 
	// symbols and fill the specified bool with true if it matches or false if it
	// doesn't and the function should noop if the value is true going in, that
	// way long callstacks of std::bind internals can be shortened.
	typedef oFUNCTION<int(char* _Buffer, size_t _SizeofBuffer, unsigned long long _Symbol, const char* _PrefixString, bool* _pIsStdBind)> GetCallstackSymbolStringFn;

	// Print a fixed string to some underlying destination. This makes no 
	// assumptions about the string itself, and should add nothing to the string,
	// such as an automatic newline or the like.
	typedef oFUNCTION<void(const char* _String)> PrintFn;

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
		unsigned long long StackTrace[STACK_TRACE_DEPTH];
		unsigned int NumStackEntries;
		unsigned int Line;
		unsigned int Context;
		oStringPath	Path;
		bool Tracked; // true if the allocation occurred when tracking wasn't enabled
		inline bool operator==(const ALLOCATION_DESC& _Other) { return AllocationID == _Other.AllocationID; }
	};

	// Type of container exposed so we can pass in an allocator.
	static struct HashAllocation { size_t operator()(uintptr_t _AllocationID) const { return oHash_superfast(&_AllocationID, sizeof(_AllocationID)); } };
	
	typedef std::pair<const uintptr_t, ALLOCATION_DESC> pair_type;
	typedef oStdLinearAllocator<pair_type> allocator_type;
	typedef oUnorderedMap<uintptr_t, ALLOCATION_DESC, HashAllocation, std::equal_to<uintptr_t>, std::less<uintptr_t>, allocator_type> allocations_t;

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

	// TODO: Describe params
	bool Report(bool _CurrentContextOnly = true) threadsafe;

	// Enables (or disables) memory tracking for the calling thread. This can be
	// useful in shutting down any tracking done during calls to 3rd party APIs
	// (lookin' at you TBB!).
	void EnableThreadlocalTracking(bool _Enabled = true) threadsafe;

	// Useful in tracking leaks around known points in the code, such as between
	// unit tests or level loads.
	void NewContext() threadsafe;

private:
	allocations_t Allocations;

	bool InInternalProcesses; // don't track allocations this object itself makes just to do the tracking (trust that this won't leak too!)
	DESC Desc;
	oRecursiveMutex Mutex;

	unsigned int CurrentContext;
};

#endif
