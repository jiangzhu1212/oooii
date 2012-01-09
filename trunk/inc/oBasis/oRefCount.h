// $(header)
#ifndef oRefCount_h
#define oRefCount_h

#include <oBasis/oStdAtomic.h>
#include <oBasis/oThreadsafe.h>

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
