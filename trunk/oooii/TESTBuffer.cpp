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
#include <oooii/oRef.h>
#include <oooii/oBuffer.h>
#include <oooii/oBufferPool.h>
#include <oooii/oTest.h>

struct TESTBuffer : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		static const size_t SIZE = 512;
		static const size_t ALIGNMENT = 256;
		unsigned char buf[SIZE];

		for (int i = 0; i < SIZE / sizeof(int); i++)
			((int*)buf)[i] = rand();

		oRef<threadsafe oBuffer> buffer;
		oTESTB( oBuffer::Create("TestBuffer", new unsigned char[SIZE], SIZE, oBuffer::Delete, &buffer), "Failed to create buffer");
		buffer->Update(buf, SIZE);

		unsigned char readBack[SIZE*2];
		oTESTB(buffer->Read((void**)&readBack, SIZE*2) == SIZE, "Unexpected buffer size");

		oTESTB(memcmp(buf, readBack, SIZE) == 0, "Buffer is invalid");
		memset(buf, 0xfe, SIZE);
		buffer->Update(buf, SIZE);

		oTESTB(buffer->Read((void**)&readBack, SIZE) == SIZE, "Unexpected buffer size");
		oTESTB(memcmp(buf, readBack, SIZE) == 0, "Buffer is invalid");

		oRef<threadsafe oBufferPool> Pool;
		oTESTB( oBufferPool::Create( "BufferPool", new unsigned char[SIZE], SIZE, SIZE / 16, oBuffer::Delete, &Pool ), "Failed to create buffer pool" );

		// Test some basic liftime stuff on the buffer
		Pool->GetFreeBuffer( &buffer );
		oRef<threadsafe oBuffer> other;
		Pool->GetFreeBuffer( &other );

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTBuffer);
