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
// A wrapper for oBase::countdown_latch that adds only threadsafe keywording. 
// For primary documentation, see oBase/countdown_latch.h.
#pragma once
#ifndef oConcurrency_countdown_latch_h
#define oConcurrency_countdown_latch_h

#include <oConcurrency/thread_safe.h>
#include <oBase/countdown_latch.h>
#include <stdexcept>

namespace oConcurrency {

class countdown_latch
{
public:
	countdown_latch(int _InitialCount) : l(_InitialCount) {}
	int outstanding() const threadsafe { return L().outstanding(); }
	void reset(int _InitialCount) threadsafe { L().reset(_InitialCount); }
	void reference() threadsafe { L().reference(); }
	void release() threadsafe { L().release(); }
	void wait() threadsafe { L().wait(); }
	template<typename Rep, typename Period>
	oStd::cv_status::value wait_for(const oStd::chrono::duration<Rep, Period>& _RelativeTime) threadsafe { return L().wait_for(_RelativeTime); }
private:
	ouro::countdown_latch l;
	ouro::countdown_latch& L() const threadsafe { return thread_cast<ouro::countdown_latch&>(l); }
	countdown_latch(const countdown_latch&); /* = delete */
	const countdown_latch& operator=(const countdown_latch&); /* = delete */
};

} // namespace oConcurrency

#endif
