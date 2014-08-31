// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

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
