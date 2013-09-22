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
// A useful concept for minimizing locks from TBB source: this object 
// encapsulates a temporary spin lock.
#pragma once
#ifndef oBase_backoff_h
#define oBase_backoff_h

#include <oStd/thread.h>

namespace ouro {

class backoff
{
public:
	backoff();

	// A spin that degrades (spins longer) with each call until it makes more 
	// sense to yield.
	void pause();

	// A spin that degrades (spins longer) with each call (returning true) until 
	// it would yield, then it returns false without yielding.
	bool try_pause();

	// Resets the degradation of the spin loop.
	void reset();

private:
	static const size_t SpinThreshold = 16;
	size_t SpinCount;
	void spin(size_t _Count);
};

inline backoff::backoff()
	: SpinCount(1)
{}

#pragma optimize("", off)
inline void backoff::spin(size_t _Count)
{
	for (size_t i = 0; i < _Count; i++) {}
}
#pragma optimize("", on)

inline void backoff::pause()
{
	if (SpinCount <= SpinThreshold)
	{
		spin(SpinCount);
		SpinCount *= 2;
	}

	else
		oStd::this_thread::yield();
}

inline bool backoff::try_pause()
{
	if (SpinCount <= SpinThreshold)
	{
		spin(SpinCount);
		SpinCount *= 2;
		return true;
	}

	return false;
}

inline void backoff::reset()
{
	SpinCount = 1;
}

} // namespace ouro

#endif
