// $(header)
#include <oooii/oBuffer.h>
#include <oooii/oAssert.h>
#include <oooii/oFile.h>
#include <oooii/oRefCount.h>
#include <oooii/oMutex.h>
#include <oooii/oLockedPointer.h>
#include <oooii/oRef.h>

const oGUID& oGetGUID( threadsafe const oBuffer* threadsafe const * )
{
	// {714C9432-EBF6-4232-9E2E-90692C294B8B}
	static const oGUID oIIDBuffer = { 0x714c9432, 0xebf6, 0x4232, { 0x9e, 0x2e, 0x90, 0x69, 0x2c, 0x29, 0x4b, 0x8b } };
	return oIIDBuffer;
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
	bool success = oFile::LoadBuffer(&b, &size, malloc, _Path, _IsText);
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
