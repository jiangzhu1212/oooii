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
#include "pch.h"
#include <oooii/oBuffer.h>
#include <oooii/oAssert.h>
#include <oooii/oRefCount.h>
#include <oooii/oStdio.h>
#include <oooii/oThreading.h>
#include <oooii/oLockedPointer.h>
#include <oooii/oLockFreeQueue.h>
#include <oooii/oRef.h>

const oGUID& oGetGUID( threadsafe const oBuffer* threadsafe const * )
{
	// {714C9432-EBF6-4232-9E2E-90692C294B8B}
	static const oGUID oIIDBuffer = { 0x714c9432, 0xebf6, 0x4232, { 0x9e, 0x2e, 0x90, 0x69, 0x2c, 0x29, 0x4b, 0x8b } };
	return oIIDBuffer;
}

const oGUID& oGetGUID( threadsafe const oBufferPool* threadsafe const * )
{
	// {B33BD1BF-EA1C-43d6-9762-38B81BC53717}
	static const oGUID oIIDBufferPool = { 0xb33bd1bf, 0xea1c, 0x43d6, { 0x97, 0x62, 0x38, 0xb8, 0x1b, 0xc5, 0x37, 0x17 } };
	return oIIDBufferPool;
}

struct oBuffer_Impl : public oBuffer
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<oBuffer>());

	oBuffer_Impl(const char* _Name, void* _Allocation, size_t _Size, DeallocateFn _DeallocateFn);
	~oBuffer_Impl();

	void Lock() threadsafe override;
	void LockRead() const threadsafe override;
	void Unlock() threadsafe override;
	void UnlockRead() const threadsafe override;

	void* GetData() override;
	const void* GetData() const override;

	size_t GetSize() const threadsafe override;
	const char* GetName() const threadsafe override;

	void* Allocation;
	char Name[_MAX_PATH];
	size_t Size;
	DeallocateFn Deallocate;
	mutable oRWMutex RWMutex;
	oRefCount RefCount;
};

bool oBuffer::Create(const char* _Name, void* _Allocation, size_t _Size, DeallocateFn _DeallocateFn, threadsafe oBuffer** _ppBuffer)
{
	if (!_Allocation || !_Size) return false;
	*_ppBuffer = new oBuffer_Impl(_Name, _Allocation, _Size, _DeallocateFn);
	return !!*_ppBuffer;
}

bool oBuffer::Create( const char* _Name, const void* _Allocation, size_t _Size, DeallocateFn _DeallocateFn, threadsafe const oBuffer** _ppBuffer )
{
	if (!_Allocation || !_Size) return false;
	// @oooii-kevin: const_cast is safe here because we guard const correctness by only returning a const pointer
	*_ppBuffer = new oBuffer_Impl(_Name, const_cast<void*>( _Allocation ), _Size, _DeallocateFn );
	return !!*_ppBuffer;
}


bool oBuffer::Create(const char* _Path, bool _IsText, threadsafe oBuffer** _ppBuffer)
{
	void* b = 0;
	size_t size = 0;
	bool success = oLoadBuffer(&b, &size, malloc, _Path, _IsText);
	return success ? Create(_Path, b, size, free, _ppBuffer) : success;
}

void oBuffer::Update(const void* _pData, size_t _SizeOfData) threadsafe
{
	oASSERT(_SizeOfData <= GetSize(), "oBuffer is too small for update");
	size_t copySize = __min(_SizeOfData, GetSize());
	oLockedPointer<oBuffer> pLockedThis(this);
	memcpy(pLockedThis->GetData(), _pData, copySize);
}

size_t oBuffer::Read(void** _ppDestination, size_t _SizeofDestination) const threadsafe
{
	oASSERT(_SizeofDestination >= GetSize(), "Destination is too small for update");
	size_t copySize = __min(_SizeofDestination, GetSize());
	oConstLockedPointer<oBuffer> pLockedThis(this);
	memcpy(_ppDestination, pLockedThis->GetData(), copySize);
	return copySize;
}

oBuffer_Impl::oBuffer_Impl(const char* _Name, void* _Allocation, size_t _Size, DeallocateFn _DeallocateFn)
	: Allocation(_Allocation)
	, Size(_Size)
	, Deallocate(_DeallocateFn)
{
	*Name = 0;
	if (_Name)
		strcpy_s(Name, _Name);
}

oBuffer_Impl::~oBuffer_Impl()
{
	oCheckUnlocked(RWMutex);
	if (Deallocate)
		Deallocate(Allocation);
}

void oBuffer_Impl::Lock() threadsafe
{
	RWMutex.Lock();
}

void oBuffer_Impl::LockRead() const threadsafe
{
	RWMutex.LockRead();
}

void oBuffer_Impl::Unlock() threadsafe
{
	RWMutex.Unlock();
}

void oBuffer_Impl::UnlockRead() const threadsafe
{
	RWMutex.UnlockRead();
}

void* oBuffer_Impl::GetData()
{
	return Allocation;
}

const void* oBuffer_Impl::GetData() const
{
	return Allocation;
}

size_t oBuffer_Impl::GetSize() const threadsafe
{
	return Size;
}

const char* oBuffer_Impl::GetName() const threadsafe
{
	return thread_cast<const char*>(Name);
}

struct oBufferPoolImpl : public oBufferPool
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<oBufferPool>());

	oBufferPoolImpl(const char* _Name, void* _Allocation, size_t _AllocationSize, size_t _IndividualBufferSize, oBuffer::DeallocateFn _DeallocateFn, bool* bSuccess);
	~oBufferPoolImpl();
	bool GetFreeBuffer(threadsafe oBuffer** _ppBuffer) threadsafe override;
	void DestroyBuffer(void* _pBuffer) threadsafe;

	char BufferName[_MAX_PATH];
	char Name[_MAX_PATH];
	oRefCount RefCount;
	oBuffer::DeallocateFn Dealloc;
	oLockFreeQueue<oRef<threadsafe oBuffer>> FreeBuffers;
	unsigned int BufferCount;
	threadsafe size_t IndividualBufferSize;
	void* pPoolBase;
	bool Open;
};


oBufferPoolImpl::oBufferPoolImpl(const char* _Name, void* _pAllocation, size_t _AllocationSize, size_t _IndividualBufferSize, oBuffer::DeallocateFn _DeallocateFn, bool* bSuccess) :
pPoolBase(_pAllocation),
IndividualBufferSize(_IndividualBufferSize),
Open(true),
BufferCount(0),
Dealloc(_DeallocateFn)
{
	strcpy_s(Name, oSAFESTR(_Name));
	*bSuccess = false;
	unsigned char* pNextAlloc = (unsigned char*)pPoolBase;

	sprintf_s( BufferName, "%s_Buffer", Name );
	const unsigned char* pPoolEnd = (unsigned char*)pPoolBase + _AllocationSize;
	while( pNextAlloc + IndividualBufferSize < pPoolEnd )
	{
		oRef<threadsafe oBuffer> FreeBuffer;

		if( !oBuffer::Create( BufferName, pNextAlloc, IndividualBufferSize, oBIND( &oBufferPoolImpl::DestroyBuffer, this, oBIND1), &FreeBuffer ) )
			return;

		if( !FreeBuffers.TryPush( FreeBuffer ) )
			return;

		++BufferCount;
		pNextAlloc += IndividualBufferSize;
	}

	*bSuccess = true;
}

bool oBufferPool::Create( const char* _Name, void* _Allocation, size_t _AllocationSize, size_t _IndividualBufferSize, oBuffer::DeallocateFn _DeallocateFn, threadsafe oBufferPool** _ppBuffer )
{
	bool success = false;
	*_ppBuffer = new oBufferPoolImpl( _Name, _Allocation, _AllocationSize, _IndividualBufferSize, _DeallocateFn, &success );
	if( !success )
	{
		*_ppBuffer = NULL;
		return false;
	}
	return true;
}

oBufferPoolImpl::~oBufferPoolImpl()
{
	// Close the pool so destruction can occur (we won't try to recycle the buffer)
	Open = false;
	oASSERT( FreeBuffers.Size() == BufferCount, "Buffers have not been returned to the pool!" );
	
	// Pop all the references off so they go out of scope
	oRef<threadsafe oBuffer> FreeBuffer;
	while( FreeBuffers.TryPop(FreeBuffer) )
	{

	}
	FreeBuffer = NULL;

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
			oDEC( &BufferCount );
			delete this;
			return;
		}

		// Pool is still alive so recycle this memory back into the pool
		oRef<threadsafe oBuffer> FreeBuffer;
		// Threadcasts safe because size and name don't change
		if( !oBuffer::Create( thread_cast<char*>( BufferName ), _pBufer, *thread_cast<size_t*>( &IndividualBufferSize ), oBIND( &oBufferPoolImpl::DestroyBuffer, this, oBIND1), &FreeBuffer ) )
			return;

		FreeBuffers.TryPush( FreeBuffer )	;
	}
}

bool oBufferPoolImpl::GetFreeBuffer(threadsafe oBuffer** _ppBuffer) threadsafe
{
	oRef<threadsafe oBuffer> FreeBuffer;
	if( !FreeBuffers.TryPop(FreeBuffer) )
	{
		oSetLastError(ENOENT, "There are no free buffers left in the oBufferPool");
		return false;
	}

	*_ppBuffer = FreeBuffer;
	FreeBuffer->Reference();

	// See DestroyBuffer for why we reference this here
	Reference();
	return true;
}


