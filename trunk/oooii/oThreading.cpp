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
#include <oooii/oThreading.h>
#include <oooii/oAssert.h>
#include <oooii/oErrno.h>
#include <oooii/oEvent.h>
#include <oooii/oFile.h>
#include <oooii/oRef.h>
#include <oooii/oRefCount.h>
#include <oooii/oSingleton.h>
#include <oooii/oThread.h>
#include <oooii/oWindows.h>

unsigned int oGetCurrentThreadID()
{
	return ::GetCurrentThreadId();
}

unsigned int oGetMainThreadID()
{
	return oWinGetMainThreadID();
}

void* oGetCurrentThreadNativeHandle()
{
	return ::GetCurrentThread();
}

unsigned int oGetCurrentProcessID()
{
	return ::GetCurrentProcessId();
}

void* oGetProcessNativeHandle(const char* _Name)
{
	return oWinGetProcessHandle(_Name);
}

void oYield()
{
	#if defined(_WIN32) || defined(_WIN64)
		SwitchToThread();
	#else
		#error Unsupported platform
	#endif
}

void oSleep(unsigned int _Milliseconds)
{
	Sleep((DWORD)_Milliseconds);
}

void oSerialFor(oFUNCTION<void(size_t _Index)> _Function, size_t _Begin, size_t _End)
{
	for (size_t i = _Begin; i < _End; i++)
		_Function(i);
}

void oRunSerialTasks(oFUNCTION<void()>* _pFunctions, size_t _NumFunctions)
{
	for (size_t i = 0; i < _NumFunctions; i++)
		_pFunctions[i]();
}
