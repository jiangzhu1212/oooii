// $(header)
#include <oPlatform/oEvent.h>
#include <oBasis/oAssert.h>
#include <oPlatform/oWindows.h>

const oEvent::AutoReset_t oEvent::AutoReset;

oHEVENT oEventCreate(bool _AutoReset, const char* _InterprocessName)
{
	char windowsInterProcessName[1024];
	sprintf_s(windowsInterProcessName, "Global\\%s", oSAFESTR(_InterprocessName));
	HANDLE hEvent = CreateEvent(0, _AutoReset ? FALSE : TRUE, FALSE, _InterprocessName ? windowsInterProcessName : nullptr);
	if (!hEvent) oWinSetLastError();
	return (oHEVENT)hEvent;
}

void oEventDestroy(oHEVENT _hEvent)
{
	CloseHandle(_hEvent);
}

void oEventSet(oHEVENT _hEvent)
{
	SetEvent(_hEvent);
}

void oEventReset(oHEVENT _hEvent)
{
	ResetEvent(_hEvent);
}

bool oEventWait(oHEVENT _hEvent, unsigned int _TimeoutMS)
{
	return oWaitSingle(_hEvent, _TimeoutMS);
}

bool oEventWaitMultiple(oHEVENT* _hEvents, size_t _NumEvents, size_t* _pWaitBreakingEventIndex, unsigned int _TimeoutMS)
{
	if (_NumEvents >= 64)
		return oErrorSetLast(oERROR_AT_CAPACITY, "A maximum of 64 events can be waited on at once");
	return oWaitMultiple((HANDLE*)_hEvents, _NumEvents, _pWaitBreakingEventIndex, _TimeoutMS);
}
