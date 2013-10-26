/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
#include <oPlatform/oInterprocessEvent.h>
#include <oBase/assert.h>
#include <oPlatform/Windows/oWindows.h>

const oInterprocessEvent::AutoReset_t oInterprocessEvent::AutoReset;

oHEVENT oInterprocessEventCreate(bool _AutoReset, const char* _InterprocessName)
{
	char windowsInterProcessName[1024];
	snprintf(windowsInterProcessName, "Global\\%s", oSAFESTR(_InterprocessName));
	HANDLE hEvent = CreateEvent(0, _AutoReset ? FALSE : TRUE, FALSE, _InterprocessName ? windowsInterProcessName : nullptr);
	if (!hEvent) oWinSetLastError();
	return (oHEVENT)hEvent;
}

void oInterprocessEventDestroy(oHEVENT _hEvent)
{
	CloseHandle(_hEvent);
}

void oInterprocessEventSet(oHEVENT _hEvent)
{
	SetEvent(_hEvent);
}

void oInterprocessEventReset(oHEVENT _hEvent)
{
	ResetEvent(_hEvent);
}

bool oInterprocessEventWait(oHEVENT _hEvent, unsigned int _TimeoutMS)
{
	return WAIT_OBJECT_0 == ::WaitForSingleObject(_hEvent, _TimeoutMS == oInfiniteWait ? INFINITE : _TimeoutMS);
}

bool oInterprocessEventWaitMultiple(oHEVENT* _hEvents, size_t _NumEvents, size_t* _pWaitBreakingEventIndex, unsigned int _TimeoutMS)
{
	if (_NumEvents >= 64)
		oTHROW(no_buffer_space, "A maximum of 64 events can be waited on at once");
	
	DWORD result = ::WaitForMultipleObjects(static_cast<DWORD>(_NumEvents), (const HANDLE*)_hEvents, !_pWaitBreakingEventIndex, _TimeoutMS == oInfiniteWait ? INFINITE : _TimeoutMS);

	if (result == WAIT_FAILED)
	{
		oWinSetLastError();
		return false;
	}

	else if (result == WAIT_TIMEOUT)
	{
		oErrorSetLast(std::errc::timed_out);
		return false;
	}

	else if (_pWaitBreakingEventIndex)
	{
		if (result >= WAIT_ABANDONED_0) 
			*_pWaitBreakingEventIndex = result - WAIT_ABANDONED_0;
		else
			*_pWaitBreakingEventIndex = result - WAIT_OBJECT_0;
	}

	return true;
}
