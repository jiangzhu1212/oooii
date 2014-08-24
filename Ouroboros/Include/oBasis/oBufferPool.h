/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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

// The buffer pool breaks a larger allocation into several smaller 
// buffers and recycles the memory as buffers go out of scope. There 
// is no explicit return as this is handled purely through the 
// oBuffer's refcount.
#pragma once
#ifndef oBufferPool_h
#define oBufferPool_h

#include <oBasis/oBuffer.h>

// {B33BD1BF-EA1C-43d6-9762-38B81BC53717}
oDEFINE_GUID_I(oBufferPool, 0xb33bd1bf, 0xea1c, 0x43d6, 0x97, 0x62, 0x38, 0xb8, 0x1b, 0xc5, 0x37, 0x17);
interface oBufferPool : oInterface
{
	virtual bool GetFreeBuffer(threadsafe oBuffer** _ppBuffer) threadsafe = 0;
};

bool oBufferPoolCreate(const char* _Name, void* _Allocation, size_t _AllocationSize, size_t _IndividualBufferSize, oBuffer::DeallocateFn _DeallocateFn, threadsafe oBufferPool** _ppBuffer);

#endif
