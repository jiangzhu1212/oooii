// $(header)
#include <oBasis/oBuffer.h>
#include <oBasis/oAssert.h>
#include <oBasis/oLockedPointer.h>
#include <oBasis/oMutex.h>
#include <oBasis/oRef.h>
#include <oBasis/oRefCount.h>
#include <oBasis/oURI.h>

const oGUID& oGetGUID( threadsafe const oBuffer* threadsafe const * )
{
	// {714C9432-EBF6-4232-9E2E-90692C294B8B}
	static const oGUID oIIDBuffer = { 0x714c9432, 0xebf6, 0x4232, { 0x9e, 0x2e, 0x90, 0x69, 0x2c, 0x29, 0x4b, 0x8b } };
	return oIIDBuffer;
}

struct oBuffer_Impl : public oBuffer
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oBuffer);
	oDEFINE_LOCKABLE_INTERFACE(RWMutex);

	oBuffer_Impl(const char* _Name, void* _Allocation, size_t _Size, DeallocateFn _DeallocateFn, bool* _pSuccess);
	~oBuffer_Impl();

	void* GetData() override;
	const void* GetData() const override;

	size_t GetSize() const override;
	const char* GetName() const threadsafe override;

	void* Allocation;
	char Name[_MAX_PATH];
	size_t Size;
	DeallocateFn Deallocate;
	mutable oSharedMutex RWMutex;
	oRefCount RefCount;
};

template<typename void_t, typename ptr_t>
bool oBufferCreateImpl(const char* _Name, void_t _Allocation, size_t _Size, oBuffer::DeallocateFn _DeallocateFn, ptr_t _ppBuffer)
{
	bool success = false;
	oCONSTRUCT(thread_cast<oBuffer**>(_ppBuffer), oBuffer_Impl(_Name, const_cast<void*>(_Allocation), _Size, _DeallocateFn, &success));  // casts are safe because the higher level oBufferCreate calls ensure thread and const safety
	return !!*_ppBuffer;
}

bool oBufferCreate(const char* _Name, void* _Allocation, size_t _Size, oBuffer::DeallocateFn _DeallocateFn, oBuffer** _ppBuffer)
{
	return oBufferCreateImpl(_Name,  _Allocation, _Size, _DeallocateFn, _ppBuffer);
}
bool oBufferCreate(const char* _Name, const void* _Allocation, size_t _Size, oBuffer::DeallocateFn _DeallocateFn, const oBuffer** _ppBuffer)
{
	return oBufferCreateImpl(_Name,  _Allocation, _Size, _DeallocateFn, _ppBuffer);
}

bool oBufferCreate(const char* _Name, void* _Allocation, size_t _Size, oBuffer::DeallocateFn _DeallocateFn, threadsafe oBuffer** _ppBuffer)
{
	return oBufferCreateImpl(_Name,  _Allocation, _Size, _DeallocateFn, _ppBuffer);
}

bool oBufferCreate( const char* _Name, const void* _Allocation, size_t _Size, oBuffer::DeallocateFn _DeallocateFn, threadsafe const oBuffer** _ppBuffer )
{
	return oBufferCreateImpl(_Name,  _Allocation, _Size, _DeallocateFn, _ppBuffer);
}

oBuffer_Impl::oBuffer_Impl(const char* _Name, void* _Allocation, size_t _Size, DeallocateFn _DeallocateFn, bool* _pSuccess)
	: Allocation(_Allocation)
	, Size(_Size)
	, Deallocate(_DeallocateFn)
{
	if (!Allocation || !Size)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER);
	}

	*Name = 0;
	if (_Name)
		strcpy_s(Name, _Name);

	*_pSuccess = true;
}

oBuffer_Impl::~oBuffer_Impl()
{
	if (Deallocate)
		Deallocate(Allocation);
}

void* oBuffer_Impl::GetData()
{
	return Allocation;
}

const void* oBuffer_Impl::GetData() const
{
	return Allocation;
}

size_t oBuffer_Impl::GetSize() const
{
	return Size;
}

const char* oBuffer_Impl::GetName() const threadsafe
{
	return thread_cast<const char*>(Name);
}
