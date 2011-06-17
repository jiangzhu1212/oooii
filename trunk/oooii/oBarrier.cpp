// $(header)
#include <oooii/oBarrier.h>
#include <oooii/oAssert.h>
#include <oooii/oAtomic.h>
#include <oooii/oWindows.h>

unsigned int oBarrier::Reference() threadsafe
{
	unsigned int n = oINC(&r);
	if (n == 1) Event.Reset();
	return n;
}

unsigned int oBarrier::Release() threadsafe
{
	unsigned int n = oDEC(&r);
	if (n == 0)
		Event.Set();
	return n;
}

size_t oBarrier::WaitMultiple(threadsafe oBarrier** _ppBarriers, size_t _NumBarriers, bool _WaitAll, unsigned int _TimeoutMS)
{
	// Bypass using cross-platform oEvent API to avoid collapsing from a list of 
	// oBarriers to a list of oEvents, then again to a list of HANDLEs.

	HANDLE handles[64]; // 64 is a windows limit
	oASSERT(_NumBarriers < oCOUNTOF(handles), "Windows has a limit of 64 handles that can be waited on by WaitForMultipleObjects");
	for (size_t i = 0; i < _NumBarriers; i++)
		handles[i] = static_cast<HANDLE>(_ppBarriers[i]->Event.GetNativeHandle());
	DWORD result = WaitForMultipleObjects(static_cast<DWORD>(_NumBarriers), handles, _WaitAll, _TimeoutMS);
	return result == WAIT_TIMEOUT ? oTIMED_OUT : result;
}
