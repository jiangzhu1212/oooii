// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBase/leak_tracker.h>
#include <oBase/assert.h>

using namespace std;

namespace ouro {

size_t leak_tracker::num_outstanding_allocations(bool current_context_only)
{
	size_t n = 0;
	for (const allocations_t::value_type& pair : Allocations)
	{
		const entry& e = pair.second;
		if (e.tracked && (!current_context_only || e.context == CurrentContext))
			n++;
	}
	return n;
}

size_t leak_tracker::report(bool current_context_only)
{
	xlstring buf;
	sstring memsize;

	release_delay();
	if (cv_status::timeout == DelayLatch.wait_for(chrono::milliseconds(Info.expected_delay_ms))) // some delayed frees might be in threads that get stomped on (thus no exit code runs) during static deinit, so don't wait forever
		oTRACE("WARNING: a delay on the leak report count was added, but has yet to be released. The timeout has been reached, so this report will include delayed releases that haven't (yet) occurred.");

	// Some 3rd party task systems (TBB) may not have good API for ensuring they
	// are in a steady-state going into a leak report. To limit the number of 
	// false-positives, check again if there are leaks.
	bool RecoveredFromAsyncLeaks = false;
	size_t CheckedNumLeaks = num_outstanding_allocations(current_context_only);
	if (CheckedNumLeaks > 0)
	{
		oTRACE("There are potentially %u leaks(s), sleeping and checking again to eliminate async false positives...", CheckedNumLeaks);
		this_thread::sleep_for(chrono::milliseconds(Info.unexpected_delay_ms));
		RecoveredFromAsyncLeaks = true;
	}

	size_t nLeaks = 0;
	bool headerPrinted = false;
	size_t totalLeakBytes = 0;
	for (const allocations_t::value_type& pair : Allocations)
	{
		const entry& e = pair.second;
		if (e.tracked && (!current_context_only || e.context == CurrentContext))
		{
			if (!headerPrinted)
			{
				mstring Header;
				snprintf(Header, "========== Leak Report%s ==========\n"
					, RecoveredFromAsyncLeaks ? " (recovered from async false positives)" : "");
				Info.print(Header);
				headerPrinted = true;
			}
			
			nLeaks++;
			totalLeakBytes += e.size;

			format_bytes(memsize, e.size, 2);

			if (Info.use_hex_for_alloc_id)
			{
				if (e.source.empty())
					snprintf(buf, "<no filename> : {0x%p} %s (probably a call to ::new(size_t))\n", e.id, memsize.c_str());
				else
					snprintf(buf, "%s(%u) : {0x%p} %s\n", e.source.c_str(), e.line, e.id, memsize.c_str());
			}

			else
			{
				if (e.source.empty())
					snprintf(buf, "<no filename> : {%d} %s (probably a call to ::new(size_t))\n", e.id, memsize.c_str());
				else
					snprintf(buf, "%s(%u) : {%d} %s\n", e.source.c_str(), e.line, e.id, memsize);
			}

			Info.print(buf);

			bool IsStdBind = false;
			for (size_t i = 0; i < e.num_stack_entries; i++)
			{
				bool WasStdBind = IsStdBind;
				Info.format(buf, buf.capacity(), e.stack[i], "  ", &IsStdBind);
				if (!WasStdBind && IsStdBind) // skip a number of the internal wrappers
					i += std_bind_internal_offset;
				Info.print(buf);
			}
		}
	}

	if (nLeaks)
	{
		sstring strTotalLeakBytes;
		mstring Footer;
		format_bytes(strTotalLeakBytes, totalLeakBytes, 2);
		snprintf(Footer, "========== Leak Report: %u Leak(s) %s%s ==========\n"
			, nLeaks
			, strTotalLeakBytes.c_str()
			, RecoveredFromAsyncLeaks ? " (recovered from async false positives)" : "");
		Info.print(Footer);
	}

	DelayLatch.reset(1);
	return nLeaks;
}

void leak_tracker::on_allocate(uint32_t alloc_id, size_t _Size, const char* _Path, uint32_t _Line, uint32_t _OldAllocationID)
{
	if (!Internal) // prevent infinite recursion
	{
		Internal = true;

		#if oENABLE_RELEASE_ASSERTS == 1 || oENABLE_ASSERTS == 1
			bool erased = 
		#endif
		find_and_erase(Allocations, _OldAllocationID);
		oASSERT(_OldAllocationID || !erased, "Address already tracked, and this event type is not a reallocation");

		entry e;
		e.id = alloc_id;
		e.size = _Size;
		e.source = _Path;
		e.line = static_cast<uint16_t>(_Line);
		e.context = CurrentContext;
		e.tracked = Info.thread_local_tracking_enabled();
		memset(e.stack, 0, sizeof(e.stack));
		e.num_stack_entries = Info.capture_callstack ? static_cast<uint8_t>(Info.callstack(e.stack, stack_trace_max_depth, stack_trace_offset)) : 0;
		Allocations[alloc_id] = e;
		Internal = false;
	}
}

void leak_tracker::on_deallocate(uint32_t alloc_id)
{
	if (!Internal)
	{
		Internal = true;
		// there may be existing allocs before tracking was enabled, so we're going 
		// to have to ignore those since they weren't captured
		find_and_erase(Allocations, alloc_id);
		Internal = false;
	}
}

void leak_tracker::thread_local_tracking(bool enabled)
{
	Internal = true;
	Info.thread_local_tracking_enabled() = enabled;
	Internal = false;
}

}
