// $(header)
#include <oooii/oCountdownLatch.h>
#include <oooii/oWindows.h>

oCountdownLatch::oCountdownLatch(const char* _DebugName, int _InitialCount)
	: Event(_DebugName)
	, RefCount(_InitialCount)
{
	if (_InitialCount < 0)
		Event.Set();
}

void* oCountdownLatch::GetNativeHandle() threadsafe
{
	return Event.GetNativeHandle();
}

const char* oCountdownLatch::GetDebugName() const threadsafe
{
	return Event.GetDebugName();
}

int oCountdownLatch::GetNumOutstanding() threadsafe
{
	int count = RefCount.Reference();
	Release();
	return count;
}

void oCountdownLatch::Reset(int _InitialCount)
{
	RefCount.Set(_InitialCount);
	Event.Reset();
}

void oCountdownLatch::Reference() threadsafe
{
	#ifdef oENABLE_ASSERTS
		int count = 
	#endif
	RefCount.Reference();
	oASSERT(count > 0, "oCountdownLatch::Reference() called on an invalid refcount, meaning this reference is too late to keep any waiting threads blocked. This is a race condition, but really it is a poorly-behaved system because references are being added after the countdown had finished. If possible, use Reset or the ctor to set an initial count beforehand and don't use Reference() at all (that's to what a semaphore limits the options), or ensure that a separate reference is kept by the system to protect against the event firing and only release that reference when it is known that new reference will not be made.");
};

void oCountdownLatch::Release() threadsafe
{
	if (RefCount.Release())
		Event.Set();
}

bool oCountdownLatch::Wait(unsigned int _TimeoutMS) threadsafe
{
	return Event.Wait(_TimeoutMS);
}

bool oCountdownLatch::WaitMultiple(threadsafe oCountdownLatch** _ppCountdownLatches, size_t _NumCountdownLatches, size_t* _pWaitBreakingCountdownLatchIndex, unsigned int _TimeoutMS)
{
	return oTWaitMultiple(_ppCountdownLatches, _NumCountdownLatches, _pWaitBreakingCountdownLatchIndex, _TimeoutMS);
}
