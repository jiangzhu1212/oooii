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
// Extension API to the oThread interface
#pragma once
#ifndef oThreading_h
#define oThreading_h

#include <oBasis/oThread.h>

// Returns the unique ID for the main thread that initialized the session for 
// the process.
oThread::id oThreadGetMainID();

// Return the native handle of the thread that calls this function (HANDLE on 
// Windows)
oThread::native_handle_type oThreadGetCurrentNativeHandle();

bool oThreadSetAffinity(oStd::thread& _Thread, size_t _AffinityMask);

enum oTHREAD_PRIORITY
{
	oTHREAD_PRIORITY_NOT_RUNNING,
	oTHREAD_PRIORITY_LOWEST,
	oTHREAD_PRIORITY_LOW,
	oTHREAD_PRIORITY_NORMAL,
	oTHREAD_PRIORITY_HIGH,
	oTHREAD_PRIORITY_HIGHEST,
};

bool oThreadSetPriority(threadsafe oThread& _Thread, oTHREAD_PRIORITY _Priority);
oTHREAD_PRIORITY oThreadGetPriority(oThread& _Thread);

#endif
