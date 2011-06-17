// $(header)
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
