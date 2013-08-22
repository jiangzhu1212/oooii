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
// Anything that might want to be consistently controlled by a unit test 
// infrastructure is separated out into this virtual interface.
#pragma once
#ifndef oConcurrencyTestRequirements_h
#define oConcurrencyTestRequirements_h

#include <oConcurrency/oConcurrency.h>
#include <oConcurrency/thread_safe.h>
#include <cstdarg>

namespace oConcurrency {
	namespace tests {

interface test_threadpool
{
public:

	// returns the name used to identify this threadpool for test's report.
	virtual const char* name() const threadsafe = 0;

	// dispatches a single task for execution on any thread. There is no execution
	// order guarantee.
	virtual void dispatch(const oTASK& _Task) threadsafe = 0;

	// parallel_for basically breaks up some dispatch calls to be executed on 
	// worker threads. If the underlying threadpool does not support parallel_for,
	// this should return false.
	virtual bool parallel_for(size_t _Begin, size_t _End, const oINDEXED_TASK& _Task) threadsafe = 0;

	// waits for the threadpool to be empty. The threadpool must be reusable after
	// this call (this is not join()).
	virtual void flush() threadsafe = 0;

	// Release the threadpool reference obtained by enumerate_threadpool below.
	virtual void release() threadsafe = 0;
};

// No requirements yet...
interface requirements
{
	virtual bool is_debugger_attached() const = 0;
	virtual void vreport(const char* _Format, va_list _Args) = 0;
	inline void report(const char* _Format, ...) { va_list a; va_start(a, _Format); vreport(_Format, a); va_end(a); }
};

	} // namespace tests
} // namespace oConcurrency

#endif
