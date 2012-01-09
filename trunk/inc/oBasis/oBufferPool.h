// $(header)

// The buffer pool breaks a larger allocation into several smaller 
// buffers and recycles the memory as buffers go out of scope. There 
// is no explicit return as this is handled purely through the 
// oBuffer's refcount.
#pragma once
#ifndef oBufferPool_h
#define oBufferPool_h

#include <oBasis/oBuffer.h>

interface oBufferPool : oInterface
{
	virtual bool GetFreeBuffer(threadsafe oBuffer** _ppBuffer) threadsafe = 0;
};

oAPI bool oBufferPoolCreate(const char* _Name, void* _Allocation, size_t _AllocationSize, size_t _IndividualBufferSize, oBuffer::DeallocateFn _DeallocateFn, threadsafe oBufferPool** _ppBuffer);

#endif
