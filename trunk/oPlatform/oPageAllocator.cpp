// $(header)
#include <oPlatform/oPageAllocator.h>
#include <oBasis/oError.h>
#include <oBasis/oRefCount.h>
#include <oPlatform/oWindows.h>

static inline DWORD GetAccess(bool _ReadWrite) { return _ReadWrite ? PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ; }

static inline oPAGE_STATUS GetStatus(DWORD _State)
{
	switch (_State)
	{
		case MEM_COMMIT: return oPAGE_COMMITTED;
		case MEM_FREE: return oPAGE_FREE;
		case MEM_RESERVE: return oPAGE_RESERVED;
		oNODEFAULT;
	}
}

size_t oPageGetPageSize()
{
	SYSTEM_INFO sysInfo = {0};
	GetSystemInfo(&sysInfo);
	return static_cast<size_t>(sysInfo.dwPageSize);
}

void oPageGetRangeDesc(void* _BaseAddress, oPAGE_RANGE_DESC* _pRangeDesc)
{
	MEMORY_BASIC_INFORMATION mbi;
	
	#ifdef oENABLE_ASSERTS
		size_t returnSize = 
	#endif
	VirtualQuery(_BaseAddress, &mbi, sizeof(mbi));
	oASSERT(sizeof(mbi) == returnSize, "");
	_pRangeDesc->BaseAddress = mbi.BaseAddress;
	_pRangeDesc->SizeInBytes = mbi.RegionSize;
	_pRangeDesc->Status = GetStatus(mbi.State);
	_pRangeDesc->ReadWrite = (mbi.AllocationProtect & PAGE_EXECUTE_READWRITE) || (mbi.AllocationProtect & PAGE_READWRITE);
	_pRangeDesc->IsPrivate = mbi.Type == MEM_PRIVATE;
}

void* oPageReserve(void* _DesiredPointer, size_t _Size, bool _ReadWrite)
{
	DWORD flAllocationType = MEM_RESERVE;
	if (_ReadWrite)
		flAllocationType |= MEM_WRITE_WATCH;

	void* p = VirtualAllocEx(GetCurrentProcess(), _DesiredPointer, _Size, flAllocationType, GetAccess(_ReadWrite));
	if (_DesiredPointer && p != _DesiredPointer)
	{
		oErrorSetLast(oERROR_AT_CAPACITY, "VirtualAllocEx return a pointer (0x%p) that is not the same as the requested pointer (0x%p).", p, _DesiredPointer);
		if (p) oPageUnreserve(p);
		p = 0;
	}

	return p;
}

void oPageUnreserve(void* _Pointer)
{
	oVB(VirtualFreeEx(GetCurrentProcess(), _Pointer, 0, MEM_RELEASE));
}

bool oPageCommit(void* _BaseAddress, size_t _Size, bool _ReadWrite)
{
	void* p = VirtualAllocEx(GetCurrentProcess(), _BaseAddress, _Size, MEM_COMMIT, GetAccess(_ReadWrite));
	oVB_RETURN(p == _BaseAddress);
	return true;
}

bool oPageDecommit(void* _Pointer)
{
	oVB_RETURN(VirtualFreeEx(GetCurrentProcess(), _Pointer, 0, MEM_DECOMMIT));
	return true;
}

bool oPageSetReadWrite(void* _BaseAddress, size_t _Size, bool _ReadWrite)
{
	DWORD oldPermissions = 0;
	oVB_RETURN(VirtualProtect(_BaseAddress, _Size, GetAccess(_ReadWrite), &oldPermissions));
	return true;
}

bool oPageSetPageablity(void* _BaseAddress, size_t _Size, bool _Pageable)
{
	oVB_RETURN(_Pageable ? VirtualUnlock(_BaseAddress, _Size) : VirtualLock(_BaseAddress, _Size));
	return true;
}
