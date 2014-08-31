// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#ifndef oRefCount_h
#define oRefCount_h

#include <oBasis/thread_safe.h>
#include <atomic>

class oRefCount
{
	std::atomic<int> r;
public:
	oRefCount(int _InitialRefCount = 1) { Set(_InitialRefCount); }
	inline bool Valid() const threadsafe { return r > 0; }
	inline operator int() threadsafe const { return r; }
	inline int Set(int _RefCount) threadsafe { return r = _RefCount; }
	inline int Reference() threadsafe { return ++r; }
	inline bool Release() threadsafe
	{
		// Start with classic atomic ref then test for 0, but then mark the count 
		// as garbage to prevent any quick inc/dec (ABA issue) in the body of the 
		// calling if() that is testing this release's result.
		static const int sFarFromZero = 0xC0000000;
		int zero = 0;
		return (--r == 0 && r.compare_exchange_strong(zero, sFarFromZero));
	}
};

#endif
