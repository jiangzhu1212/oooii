// $(header)
#include <oBasis/oBuffer.h>
#include <oBasis/oBufferPool.h>
#include <oBasis/oRef.h>
#include <oBasis/oXML.h>
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

	oErrorSetLast(oERROR_NONE);
	return true;
}
