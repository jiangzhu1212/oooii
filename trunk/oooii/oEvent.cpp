/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
	return WaitForSingleObject(hEvent, _TimeoutMS) == WAIT_OBJECT_0;
}

size_t oEvent::WaitMultiple(threadsafe oEvent** _ppEvents, size_t _NumEvents, bool _WaitAll, unsigned int _TimeoutMS)
{
	HANDLE handles[64]; // 64 is a windows limit
	oASSERT(_NumEvents < oCOUNTOF(handles), "Windows has a limit of 64 handles that can be waited on by WaitForMultipleObjects");
	for (size_t i = 0; i < _NumEvents; i++)
		handles[i] = static_cast<HANDLE>(_ppEvents[i]->GetNativeHandle());
	unsigned long result = WaitForMultipleObjects(static_cast<unsigned long>(_NumEvents), handles, _WaitAll, _TimeoutMS);
	return result == WAIT_TIMEOUT ? oTIMED_OUT : result;
}
