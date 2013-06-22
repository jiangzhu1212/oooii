/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
// A dispatch queue that executes all enqueued tasks in order on a specific 
// thread. This is most useful when there are platform requirements such as 
// Window's window handling that must occur all in the same thread.
#ifndef oDispatchQueuePrivate_h
#define oDispatchQueuePrivate_h

#include <oBasis/oDispatchQueue.h>

// {FF7615D2-C7C4-486A-927A-343EBCEA7363}
oDEFINE_GUID_I(oDispatchQueuePrivate, 0xff7615d2, 0xc7c4, 0x486a, 0x92, 0x7a, 0x34, 0x3e, 0xbc, 0xea, 0x73, 0x63);
interface oDispatchQueuePrivate : oDispatchQueue
{
};

bool oDispatchQueueCreatePrivate(const char* _DebugName, size_t _InitialTaskCapacity, threadsafe oDispatchQueuePrivate** _ppDispatchQueue);

#endif
