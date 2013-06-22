/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#ifndef oRefCount_h
#define oRefCount_h

#include <oStd/oStdAtomic.h>
#include <oConcurrency/thread_safe.h>

class oRefCount
{
	int r;
public:
	oRefCount(int _InitialRefCount = 1) { Set(_InitialRefCount); }
	inline bool Valid() const threadsafe { return r > 0; }
	inline operator int() threadsafe const { return r; }
	inline int Set(int _RefCount) threadsafe { return oStd::atomic_exchange(&r, _RefCount); }
	inline int Reference() threadsafe { return oStd::atomic_increment(&r); }
	inline bool Release() threadsafe
	{
		// Start with classic atomic ref then test for 0, but then mark the count 
		// as garbage to prevent any quick inc/dec (ABA issue) in the body of the 
		// calling if() that is testing this release's result.
		static const int sFarFromZero = 0xC0000000;
		int newRef = oStd::atomic_decrement(&r);
		return (newRef == 0 && oStd::atomic_compare_exchange(&r, sFarFromZero, 0));
	}
};

#endif
