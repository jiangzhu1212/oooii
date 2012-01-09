/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
// One of the key values of Intel's TBB is its ability to avoid expensive 
// threading situations like false sharing, oversubscription and context 
// switches. One of the key detriments to TBB is that they don't expose enough
// of their awesomeness for public consumption or support enough platforms to
// truly be considered cross-platform (where's my Sony PS3 version?)

#pragma once
#ifndef oBackoff_h
#define oBackoff_h

#include <oBasis/oStdThread.h>

class oBackoff
{
	static const size_t SpinThreshold = 16;
	size_t SpinCount;

	#pragma optimize("", off)
	inline void spin(size_t _Count)
	{
		for (size_t i = 0; i < _Count; i++) {}
	}
	#pragma optimize("", on)

public:
	oBackoff()
		: SpinCount(1)
	{}

	inline void Pause()
	{
		if (SpinCount <= SpinThreshold)
		{
				spin(SpinCount);
				SpinCount *= 2;
		}

		else
			oStd::this_thread::yield();
	}

	inline bool TryPause()
	{
		if (SpinCount <= SpinThreshold)
		{
			spin(SpinCount);
			SpinCount *= 2;
			return true;
		}

		return false;
	}

	inline void Reset() { SpinCount = 1; }
};

#endif
