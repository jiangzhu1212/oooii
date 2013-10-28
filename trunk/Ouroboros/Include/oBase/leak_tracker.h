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
#ifndef oBase_leak_tracker_h
#define oBase_leak_tracker_h

#include <oStd/mutex.h>
#include <oBase/algorithm.h>
#include <oBase/countdown_latch.h>
#include <oBase/fixed_string.h>
#include <oBase/macros.h>

namespace ouro {

class leak_tracker
{
public:
	static const size_t stack_trace_max_depth = 32; // how many entries to savfe
	static const size_t stack_trace_offset = 8; // start at nth entry (bypass common infrastructure code)
	static const size_t std_bind_internal_offset = 5; // number of symbols internal to std::bind to skip

	typedef unsigned long long symbol_t;

	// Used to allocate and deallocate tracking entries. These allocations them-
	// selves will not be tracked.
	typedef void* (*allocate_fn)(size_t _Size);
	typedef void (*deallocate_fn)(void* _Pointer);

	// Tracking can come from 3rd party libraries with their own idea of timing
	// when freeing memory. For example TBB keeps its threads around in a way that 
	// cannot be determined from the outside, so provide a per-thread bool value
	// to allow per-thread enabling/disabling for such allocations. This is not 
	// just a thread_local value because DLLs can have their own copies and thus
	// create not only racy leaks, but also ones that only occur when called in a
	// certain order from different DLLs.
	typedef bool& (*thread_local_tracking_enabled_fn)();

	// Returns the number of symbols retreived into _pSymbols. _Offset allows
	// the capture to ignore internal details of the capture.
	typedef size_t (*callstack_fn)(symbol_t* _pSymbols, size_t _NumSymbols, size_t _Offset);

	// snprintf to the specified destination the decoded symbol_t with optional
	// prefix. This should also be able to detect if the symbol_t is a std::bind
	// detail and set the optional bool accordingly, optionally skipping the 
	// noisy inner details of std::bind.
	typedef int (*format_fn)(char* _StrDestination, size_t _SizeofStrDestination
		, symbol_t _Symbol, const char* _Prefix, bool* _pIsStdBind);

	// Print a fixed string to some destination
	typedef void (*print_fn)(const char* _String);

	struct info
	{
		info()
			: allocate(nullptr)
			, deallocate(nullptr)
			, thread_local_tracking_enabled(nullptr)
			, callstack(nullptr)
			, format(nullptr)
			, print(nullptr)
			, expected_delay_ms(5000)
			, unexpected_delay_ms(2000)
			, use_hex_for_alloc_id(false)
			, capture_callstack(false)
		{}

		allocate_fn allocate;
		deallocate_fn deallocate;
		thread_local_tracking_enabled_fn thread_local_tracking_enabled;
		callstack_fn callstack;
		format_fn format;
		print_fn print;

		unsigned int expected_delay_ms;
		unsigned int unexpected_delay_ms;
		bool use_hex_for_alloc_id;
		bool capture_callstack;
	};

	struct entry
	{
		unsigned int id;
		size_t size;
		symbol_t stack[stack_trace_max_depth];
		unsigned char num_stack_entries;
		bool tracked; // true if the allocation occurred when tracking wasn't enabled
		unsigned short line;
		unsigned short context;
		ouro::path_string source;
		bool operator==(const entry& _That) { return id == _That.id; }
		bool operator<(const entry& _That) { return id < _That.id; }
	};

	leak_tracker(const info& _Info)
		: Info(_Info)
		, Allocations(0, allocations_t::hasher(), allocations_t::key_equal(), allocations_t::key_less(), allocator_t(_Info.allocate, _Info.deallocate))
		, DelayLatch(1)
		, CurrentContext(0)
		, Internal(false)
	{}

	void thread_local_tracking(bool _Enabled);
	bool thread_local_tracking() const { return Info.thread_local_tracking_enabled(); }

	// Slow! but will pinpoint exactly where a leak was allocated.
	void capture_callstack(bool _Enabled) { Info.capture_callstack = _Enabled; }
	bool capture_callstack() const { return Info.capture_callstack; }

	// Some operations are asynchronous and allocate memory. If such operations 
	// occur near the end of the application's life, then there is an opportunity
	// to delay the report a bit waiting for the operation to complete. A memory 
	// user in such a case can call add_delay() and when its memory is freed then 
	// release_delay(). This only delays reporting - because the thread allocating 
	// memory could be terminated, this will only block for a time - not forever.
	void add_delay() { DelayLatch.reference(); }
	void release_delay() { DelayLatch.release(); }

	// Only allocations within a context are reported as leaks. This is to get 
	// around bootstrap allocations for localized reporting. For example in a unit 
	// test infrastructure, this allows only leak detection for each specific test.
	void new_context() { oStd::atomic_increment(&CurrentContext); }

	// Clears all allocation tracking.
	void reset() { lock_t Lock(Mutex); Allocations.clear(); }

	// Reports all allocations currently tracked to the debugger print function.
	// If _CurrentContextOnly is true, then only the allocations since the last 
	// call to new_context() will be reported. If _CurrentContextOnly is false, 
	// all allocations since the start of tracking will be reported. This returns 
	// the number of leaks reported.
	size_t report(bool _CurrentContextOnly = true);

	// Call this when an allocation occurs. If a realloc, pass the id of the 
	// original pointer to _OldAllocationID.
	void on_allocate(unsigned int _AllocationID, size_t _Size, const char* _Path, unsigned int _Line, unsigned int _OldAllocationID = 0);

	// Call this when a deallocation occurs
	void on_deallocate(unsigned int _AllocationID);

private:
	typedef oStd::recursive_mutex mutex_t;
	typedef oStd::lock_guard<mutex_t> lock_t;

	static struct hasher { size_t operator()(unsigned int _AllocationID) const { return _AllocationID; } };
	typedef std::pair<const unsigned int, entry> pair_t;
	typedef std_user_allocator<pair_t> allocator_t;
	typedef ouro::unordered_map<unsigned int, entry, hasher
		, std::equal_to<unsigned int>, std::less<unsigned int>, allocator_t> allocations_t;

	info Info;
	mutex_t Mutex;
	allocations_t Allocations;
	countdown_latch DelayLatch;
	unsigned short CurrentContext;
	bool Internal;

	size_t num_outstanding_allocations(bool _CurrentContextOnly);
};

} // namespace ouro

#endif
