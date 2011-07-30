// $(header)
#include <oooii/oEvent.h>
#include <oooii/oAssert.h>
#include <oooii/oWindows.h>

oEvent::oEvent(bool _AutoReset, const char* _DebugName, const char* _InterProcessName)
	: hEvent(0)
{
	CommonConstructor(_AutoReset, _DebugName, _InterProcessName);
}

oEvent::oEvent(const char* _DebugName, const char* _InterProcessName)
	: hEvent(0)
{
	CommonConstructor(false, _DebugName, _InterProcessName);
}

void oEvent::CommonConstructor(bool _AutoReset, const char* _DebugName, const char* _InterProcessName)
{
#ifdef _DEBUG
	strcpy_s(DebugName, _DebugName ? _DebugName : "");
#endif

	char windowsInterProcessName[1024];
	sprintf_s(windowsInterProcessName, "Global\\%s", oSAFESTR(_InterProcessName));
	hEvent = CreateEvent(0, _AutoReset ? FALSE : TRUE, FALSE, _InterProcessName ? windowsInterProcessName : 0);
}

oEvent::~oEvent()
{
	CloseHandle(hEvent);
}
void oEvent::Set() threadsafe
{
	SetEvent(hEvent);
}

void oEvent::Reset() threadsafe
{
	ResetEvent(hEvent);
}

bool oEvent::Wait(unsigned int _TimeoutMS) threadsafe
{
	return oWaitSingle(hEvent, _TimeoutMS);
}

bool oEvent::WaitMultiple(threadsafe oEvent** _ppEvents, size_t _NumEvents, size_t* _pWaitBreakingEventIndex, unsigned int _TimeoutMS)
{
	return oTWaitMultiple(_ppEvents, _NumEvents, _pWaitBreakingEventIndex, _TimeoutMS);
}
