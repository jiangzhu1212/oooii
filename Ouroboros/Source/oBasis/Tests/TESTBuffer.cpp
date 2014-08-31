// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBasis/oBuffer.h>
#include <oBasis/oBufferPool.h>
#include "oBasisTestCommon.h"

bool oBasisTest_oBuffer()
{
	static const size_t SIZE = 512;

	ouro::intrusive_ptr<threadsafe oBuffer> buffer;
	ouro::intrusive_ptr<threadsafe oBufferPool> Pool;
	oTESTB(oBufferPoolCreate("BufferPool", new unsigned char[SIZE], SIZE, SIZE / 16, oBuffer::Delete, &Pool), "Failed to create buffer pool");

	// Test some basic lifetime stuff on the buffer
	Pool->GetFreeBuffer(&buffer);
	ouro::intrusive_ptr<threadsafe oBuffer> other;
	Pool->GetFreeBuffer(&other);

	oErrorSetLast(0);
	return true;
}
