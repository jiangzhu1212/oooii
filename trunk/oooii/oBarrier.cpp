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
