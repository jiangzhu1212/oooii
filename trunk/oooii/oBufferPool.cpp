// $(header)
#include <oooii/oBufferPool.h>
#include <oooii/oLockFreeQueue.h>
#include <oooii/oRef.h>
#include <oooii/oRefCount.h>

const oGUID& oGetGUID( threadsafe const oBufferPool* threadsafe const * )
{
	// {B33BD1BF-EA1C-43d6-9762-38B81BC53717}
	static const oGUID oIIDBufferPool = { 0xb33bd1bf, 0xea1c, 0x43d6, { 0x97, 0x62, 0x38, 0xb8, 0x1b, 0xc5, 0x37, 0x17 } };
	return oIIDBufferPool;
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

oBufferPoolImpl::oBufferPoolImpl(const char* _Name, void* _pAllocation, size_t _AllocationSize, size_t _IndividualBufferSize, oBuffer::DeallocateFn _DeallocateFn, bool* bSuccess)
	: pPoolBase(_pAllocation)
	, IndividualBufferSize(_IndividualBufferSize)
	, Open(true)
	, BufferCount(0)
	, Dealloc(_DeallocateFn)
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
