// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// An interface for hooking a malloc routine to report allocations unmatched by
// deallocation and report the file line and callstack of where the allocation
// occurred. This is not threadsafe since usage of std::mutex at this level
// can itself trigger C++ runtime calls that could deadlock on malloc.
#pragma once
#include <oBase/algorithm.h>
#include <oConcurrency/countdown_latch.h>
#include <oMemory/std_allocator.h>
#include <oString/fixed_string.h>
#include <oBase/macros.h>
#include <atomic>
#include <cstdint>

namespace ouro {

class leak_tracker
{
public:
	static const size_t stack_trace_max_depth = 32; // how many entries to save
	static const size_t stack_trace_offset = 8; // start at nth entry (bypass common infrastructure code)
	static const size_t std_bind_internal_offset = 5; // number of symbols internal to std::bind to skip

	typedef size_t symbol_t;

	// Used to allocate and deallocate tracking entries. These allocations them-
	// selves will not be tracked.
	typedef void* (*allocate_fn)(size_t size);
	typedef void (*deallocate_fn)(void* ptr);

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
	typedef size_t (*callstack_fn)(symbol_t* symbols, size_t num_symbols, size_t offset);

	// snprintf to the specified destination the decoded symbol_t with optional
	// prefix. This should also be able to detect if the symbol_t is a std::bind
	// detail and set the optional bool accordingly, optionally skipping the 
	// noisy inner details of std::bind.
	typedef int (*format_fn)(char* dst, size_t dst_size, symbol_t symbol
		, const char* prefix, bool* out_is_std_bind);

	// Print a fixed string to some destination
	typedef void (*print_fn)(const char* str);

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

		uint32_t expected_delay_ms;
		uint32_t unexpected_delay_ms;
		bool use_hex_for_alloc_id;
		bool capture_callstack;
	};

	struct entry
	{
		uint32_t id;
		size_t size;
		symbol_t stack[stack_trace_max_depth];
		uint8_t num_stack_entries;
		bool tracked; // true if the allocation occurred when tracking wasn't enabled
		uint16_t line;
		uint16_t context;
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

	void thread_local_tracking(bool enabled);
	bool thread_local_tracking() const { return Info.thread_local_tracking_enabled(); }

	// Slow! but will pinpoint exactly where a leak was allocated.
	void capture_callstack(bool enabled) { Info.capture_callstack = enabled; }
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
	void new_context() { CurrentContext++; }

	// Clears all allocation tracking.
	void reset() { Allocations.clear(); }

	// Reports all allocations currently tracked to the debugger print function.
	// If _CurrentContextOnly is true, then only the allocations since the last 
	// call to new_context() will be reported. If _CurrentContextOnly is false, 
	// all allocations since the start of tracking will be reported. This returns 
	// the number of leaks reported.
	size_t report(bool current_context_only = true);

	// Call this when an allocation occurs. If a realloc, pass the id of the 
	// original pointer to old_alloc_id.
	void on_allocate(uint32_t alloc_id, size_t size, const char* path, uint32_t line, uint32_t old_alloc_id = 0);

	// Call this when a deallocation occurs
	void on_deallocate(uint32_t alloc_id);

private:

	static struct hasher { size_t operator()(uint32_t alloc_id) const { return alloc_id; } };
	typedef std::pair<const uint32_t, entry> pair_t;
	typedef std_user_allocator<pair_t> allocator_t;
	typedef ouro::unordered_map<uint32_t, entry, hasher
		, std::equal_to<uint32_t>, std::less<uint32_t>, allocator_t> allocations_t;

	info Info;
	allocations_t Allocations;
	countdown_latch DelayLatch;
	std::atomic<uint16_t> CurrentContext;
	bool Internal;

	size_t num_outstanding_allocations(bool current_context_only);
};

}
