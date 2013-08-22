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
#include <oBasis/oBuffer.h>
#include <oBasis/oBufferPool.h>
#include <oBasis/oRef.h>
#include "oBasisTestCommon.h"

bool oBasisTest_oBuffer()
{
	static const size_t SIZE = 512;

	oRef<threadsafe oBuffer> buffer;
	oRef<threadsafe oBufferPool> Pool;
	oTESTB(oBufferPoolCreate("BufferPool", new unsigned char[SIZE], SIZE, SIZE / 16, oBuffer::Delete, &Pool), "Failed to create buffer pool");

	// Test some basic lifetime stuff on the buffer
	Pool->GetFreeBuffer(&buffer);
	oRef<threadsafe oBuffer> other;
	Pool->GetFreeBuffer(&other);

	oErrorSetLast(0);
	return true;
}
