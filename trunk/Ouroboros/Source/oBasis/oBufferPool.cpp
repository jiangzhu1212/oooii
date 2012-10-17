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
#include <oBasis/oBufferPool.h>
#include <oBasis/oFixedString.h>
#include <oBasis/oInitOnce.h>
#include <oBasis/oLockFreeQueue.h>
#include <oBasis/oRef.h>
#include <oBasis/oRefCount.h>

const oGUID& oGetGUID( threadsafe const oBufferPool* threadsafe const * )
{
	// {B33BD1BF-EA1C-43d6-9762-38B81BC53717}
	static const oGUID oIIDBufferPool = { 0xb33bd1bf, 0xea1c, 0x43d6, { 0x97, 0x62, 0x38, 0xb8, 0x1b, 0xc5, 0x37, 0x17 } };
	return oIIDBufferPool;
}

struct oBufferPoolImpl : public oBufferPool
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oBufferPool);

	oBufferPoolImpl(const char* _Name, void* _Allocation, size_t _AllocationSize, size_t _IndividualBufferSize, oBuffer::DeallocateFn _DeallocateFn, bool* bSuccess);
	~oBufferPoolImpl();
	bool GetFreeBuffer(threadsafe oBuffer** _ppBuffer) threadsafe override;
	void DestroyBuffer(void* _pBuffer) threadsafe;

	oInitOnce<oStringPath> BufferName;
	oInitOnce<oStringPath> Name;
	oRefCount RefCount;
	oBuffer::DeallocateFn Dealloc;
	oLockFreeQueue<oRef<oBuffer>> FreeBuffers;
	unsigned int BufferCount;
	threadsafe size_t IndividualBufferSize;
	void* pPoolBase;
	bool Open;
};

oBufferPoolImpl::oBufferPoolImpl(const char* _Name, void* _pAllocation, size_t _AllocationSize, size_t _IndividualBufferSize, oBuffer::DeallocateFn _DeallocateFn, bool* bSuccess)
	: pPoolBase(_pAllocation)
	, IndividualBufferSize(_IndividualBufferSize)
	, Open(true)
	, BufferCount(0)
	, Dealloc(_DeallocateFn)
{
	*bSuccess = false;
	Name.Initialize(oSAFESTR(_Name));
	unsigned char* pNextAlloc = (unsigned char*)pPoolBase;

	oPrintf( BufferName.Initialize(), "%s_Buffer", Name );
	const unsigned char* pPoolEnd = (unsigned char*)pPoolBase + _AllocationSize;
	while( pNextAlloc + IndividualBufferSize < pPoolEnd )
	{
		oRef<oBuffer> FreeBuffer;

		if (!oBufferCreate(*BufferName, pNextAlloc, IndividualBufferSize, oBIND(&oBufferPoolImpl::DestroyBuffer, this, oBIND1), &FreeBuffer))
			return;

		FreeBuffers.push( FreeBuffer );

		++BufferCount;
		pNextAlloc += IndividualBufferSize;
	}

	*bSuccess = true;
}

bool oBufferPoolCreate( const char* _Name, void* _Allocation, size_t _AllocationSize, size_t _IndividualBufferSize, oBuffer::DeallocateFn _DeallocateFn, threadsafe oBufferPool** _ppBuffer )
{
	bool success = false;
	oCONSTRUCT(_ppBuffer, oBufferPoolImpl( _Name, _Allocation, _AllocationSize, _IndividualBufferSize, _DeallocateFn, &success ) );
	return success;
}

oBufferPoolImpl::~oBufferPoolImpl()
{
	// Close the pool so destruction can occur (we won't try to recycle the buffer)
	Open = false;
	oASSERT( FreeBuffers.unsafe_size() == BufferCount, "Buffers have not been returned to the pool!" );
	
	// Pop all the references off so they go out of scope
	oRef<oBuffer> FreeBuffer;
	while( FreeBuffers.try_pop(FreeBuffer) )
	{

	}
	FreeBuffer = nullptr;

	// Finally clean up the memory
	Dealloc(pPoolBase);
}

void oBufferPoolImpl::DestroyBuffer(void* _pBufer) threadsafe
{
	// If we're still open recycle the buffer
	if( Open )
	{
		// We Release because we have an additional implicit Reference from GetFreeBuffer
		// This is to ensure proper teardown order (we can't destruct the BufferPool until
		// all outstanding buffers have gone out of scope).  This is why we manually destroy
		// here as well
		if( RefCount.Release() )
		{
			oStd::atomic_decrement(&BufferCount);
			delete this;
			return;
		}

		// Pool is still alive so recycle this memory back into the pool
		oRef<oBuffer> FreeBuffer;
		// Threadcasts safe because size and name don't change
		if (!oBufferCreate(*BufferName, _pBufer, thread_cast<size_t&>(IndividualBufferSize), oBIND(&oBufferPoolImpl::DestroyBuffer, this, oBIND1), &FreeBuffer))
			return;

		FreeBuffers.push( FreeBuffer );
	}
}

bool oBufferPoolImpl::GetFreeBuffer(threadsafe oBuffer** _ppBuffer) threadsafe
{
	if(*_ppBuffer)
	{
		(*_ppBuffer)->Release();
		*_ppBuffer = nullptr;
	}
	oRef<oBuffer> FreeBuffer;
	if( !FreeBuffers.try_pop(FreeBuffer) )
		return oErrorSetLast(oERROR_NOT_FOUND, "There are no free buffers left in the oBufferPool");

	*_ppBuffer = FreeBuffer;
	FreeBuffer->Reference();

	// See DestroyBuffer for why we reference this here
	Reference();
	return true;
}
