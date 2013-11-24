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
// Tracks CRT allocations. Mainly this is glue code linking oBase/leak_tracker
// to libcrt malloc.
#pragma once
#ifndef oCore_win_crt_leak_tracker_h
#define oCore_win_crt_leak_tracker_h

#include <oBase/leak_tracker.h>
#include <oStd/mutex.h>
#include <unordered_map>

namespace ouro {
	namespace windows {
		namespace crt_leak_tracker {

// This ensures the crt_leak_tracker has been initialized
void ensure_initialized();

void enable(bool _Enable);
bool enabled();

void enable_report(bool _Enable);
bool enable_report();

// All outstanding allocations are cleared as if they never happened. This is
// useful in unit test situations where even though a test leaked, leaks should 
// not be reported for subsequent tests.
void new_context();

// For each allocation the callstack leading to malloc is recorded. This is very
// slow, but can easily identify the source of memory leaks.
void capture_callstack(bool _Capture);
bool capture_callstack();

void thread_local_tracking(bool _Enable);
bool thread_local_tracking();

// Returns true if there were leaks or false if there were none.
bool report(bool _CurrentContextOnly = true);

// Reset all tracking bookkeeping
void reset();

// In rare low-level systems that need to persist after leaks have been 
// reported it is helpful not to report those allocations as a leak. For example 
// a log file that is going to retain the leak report itself should not be 
// reported as a leak.
void ignore(void* _Pointer);

// See oBase/leak_tracker for more details
void add_delay();
void release_delay();

		} // namespace crt_leak_tracker
	} // namespace windows
} // namespace ouro

#endif
