// $(header)
// Interface for a multi-threaded sync event. This also has supported for multi-
// process synchronization through the use of named events. Prefer the class 
// version over the C-functions as its more RAII. This exposure was choosen to 
// keep oEvent lifetime simple as well as avoid double-indirects of a virtual 
// call + handle indirect for handle-based systems like Windows and Posix.
#pragma once
#ifndef oEvent_h
#define oEvent_h

#include <oPlatform/oStddef.h>

oDECLARE_HANDLE(oHEVENT);
oAPI oHEVENT oEventCreate(bool _AutoReset = false, const char* _InterprocessName = nullptr);
oAPI void oEventDestroy(oHEVENT _hEvent);
oAPI void oEventSet(oHEVENT _hEvent);
oAPI void oEventReset(oHEVENT _hEvent);
// Returns false if timed out (oErrorGetLast() will return oERROR_TIMEOUT)
oAPI bool oEventWait(oHEVENT _hEvent, unsigned int _TimeoutMS = oINFINITE_WAIT);
oAPI bool oEventWaitMultiple(oHEVENT* _hEvents, size_t _NumEvents, size_t* _pWaitBreakingEventIndex = nullptr, unsigned int _TimeoutMS = oINFINITE_WAIT);

class oEvent : oNoncopyable
{
public:
	typedef oHEVENT native_handle_type;

	struct AutoReset_t {};
	static const AutoReset_t AutoReset;

	oEvent(const char* _InterprocessName = nullptr) : hEvent(oEventCreate(false, _InterprocessName)) {}
	oEvent(AutoReset_t, const char* _InterprocessName = nullptr) : hEvent(oEventCreate(true, _InterprocessName)) {}
	~oEvent() { oEventDestroy(hEvent); }
	inline native_handle_type GetNativeHandle() threadsafe { return hEvent; }
	inline void Set() threadsafe { oEventSet(hEvent); }
	inline void Reset() threadsafe { oEventReset(hEvent); }
	inline bool Wait(unsigned int _TimeoutMS = oINFINITE_WAIT) threadsafe { return oEventWait(hEvent, _TimeoutMS); }

	static inline bool WaitMultiple(threadsafe oEvent** _ppEvents, size_t _NumEvents, size_t* _pWaitBreakingEventIndex = nullptr, unsigned int _TimeoutMS = oINFINITE_WAIT)
	{
		native_handle_type handles[64];
		for (size_t i = 0; i < _NumEvents; i++)
			handles[i] = _ppEvents[i]->GetNativeHandle();
		return oEventWaitMultiple(handles, _NumEvents, _pWaitBreakingEventIndex, _TimeoutMS);
	}

	template<size_t size> static inline bool WaitMultiple(threadsafe oEvent* (&_ppEvents)[size], size_t* _pWaitBreakingEventIndex = nullptr, unsigned int _TimeoutMS = oINFINITE_WAIT) { return WaitMultiple(_ppEvents, size, _pWaitBreakingEventIndex, _TimeoutMS); }
private:
	native_handle_type hEvent;
};

#endif
