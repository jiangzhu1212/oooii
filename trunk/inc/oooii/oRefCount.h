// $(header)
#ifndef oRefCount_h
#define oRefCount_h

#include <oooii/oAssert.h>
#include <oooii/oAtomic.h>

#ifndef threadsafe
	#define threadsafe volatile
#endif

class oRefCount
{
	int r;
public:
	oRefCount(int _InitialRefCount = 1) { Set(_InitialRefCount); }
	inline bool Valid() const threadsafe { return r > 0; }
	inline operator int() threadsafe const { return r; }
	inline int Set(int _RefCount) threadsafe { return oSWAP(&r, _RefCount); }
	inline void Reference() threadsafe { oASSERT(Valid(), "Using an invalid refcounted object"); oINC(&r); }
	inline bool Release() threadsafe { oASSERT(Valid(), "Using an invalid refcounted object"); return oREF_RELEASE(&r); }
};

#endif
