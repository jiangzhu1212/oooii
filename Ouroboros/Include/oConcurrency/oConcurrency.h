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
// Interfaces for concurrent programming. NOTE: These declarations and this 
// header exist at a very low level. Generic code and algorithms can leverage
// concurrent techniques for performance and that idea is something that should
// be encouraged. Alas efficient and robust concurrency handling remains very
// platform-specific, so declare a minimal API and allow usage in generic 
// libraries and leave the implementation to be downstream somewhere, so expect
// link errors unless there is middleware or other platform implementation to 
// implement these interfaces.
#pragma once
#ifndef oConcurrencyRequirements_h
#define oConcurrencyRequirements_h

#include <oBase/callable.h>
#include <oConcurrency/thread_safe.h>
#include <memory>
#include <system_error>

typedef std::function<void()> oTASK;
typedef std::function<void(size_t _Index)> oINDEXED_TASK;

namespace oConcurrency {

// _____________________________________________________________________________
// Basic utilities

// Runs the specified task (a drop-in debug replacement for oConcurrency::dispatch)
inline void dispatch_serial(const std::function<void()>& _Task) { _Task(); }

inline void serial_for(size_t _Begin, size_t _End, const std::function<void(size_t _Index)>& _Task)
{
	for (size_t i = _Begin; i < _End; i++)
		_Task(i);
}

} // namespace oConcurrency

#endif
