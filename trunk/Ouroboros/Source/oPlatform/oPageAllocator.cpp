/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
#include <oPlatform/oPageAllocator.h>
#include <oBasis/oError.h>
#include <oBasis/oRefCount.h>
#include <oPlatform/Windows/oWindows.h>

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

size_t oPageGetLargePageSize()
{
	return GetLargePageMinimum();
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

// @oooii-tony: Consider exposing this as the API...

enum oPAGE_ALLOCATION_TYPE
{
	oPAGE_RESERVE,
	oPAGE_RESERVE_READ_WRITE,
	oPAGE_COMMIT,
	oPAGE_COMMIT_READ_WRITE,
	oPAGE_RESERVE_AND_COMMIT,
	oPAGE_RESERVE_AND_COMMIT_READ_WRITE,
};

bool IsReadWrite(oPAGE_ALLOCATION_TYPE _Type)
{
	return !!(_Type & 0x1);
}

static inline DWORD GetAccess(bool _ReadWrite) { return _ReadWrite ? PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ; }
static inline DWORD GetAccess(oPAGE_ALLOCATION_TYPE _Type) { return GetAccess(IsReadWrite(_Type)); }

// This will populate the flags correctly, and adjust size to be aligned if 
// large page sizes are to be used.
static bool GetAllocationType(oPAGE_ALLOCATION_TYPE _AllocationType, void* _BaseAddress, bool _UseLargePageSize, size_t* _pSize, DWORD* _pflAllocationType, DWORD* _pdwFreeType)
{
	*_pflAllocationType = 0;
	*_pdwFreeType = 0;

	if (_UseLargePageSize)
	{
		if (_BaseAddress && (_AllocationType == oPAGE_RESERVE || _AllocationType == oPAGE_RESERVE_READ_WRITE))
			return oErrorSetLast(std::errc::invalid_argument, "Large page memory cannot be reserved.");

		*_pflAllocationType |= MEM_LARGE_PAGES;
		*_pSize = oStd::byte_align(*_pSize, oPageGetLargePageSize());
	}

	switch (_AllocationType)
	{
		case oPAGE_RESERVE_READ_WRITE: *_pflAllocationType |= MEM_WRITE_WATCH; // pass thru to oPAGE_RESERVE
		case oPAGE_RESERVE: *_pflAllocationType |= MEM_RESERVE; *_pdwFreeType = MEM_RELEASE; break;
		case oPAGE_COMMIT_READ_WRITE: // MEM_WRITE_WATCH is not a valid option for committing
		case oPAGE_COMMIT: *_pflAllocationType |= MEM_COMMIT; *_pdwFreeType = MEM_DECOMMIT; break;
		case oPAGE_RESERVE_AND_COMMIT_READ_WRITE: *_pflAllocationType |= MEM_WRITE_WATCH; // pass thru to oPAGE_RESERVE_AND_COMMIT
		case oPAGE_RESERVE_AND_COMMIT: *_pflAllocationType |= MEM_RESERVE | MEM_COMMIT; *_pdwFreeType = MEM_RELEASE | MEM_DECOMMIT; break;
		oNODEFAULT;
	}

	return true;
}

void oPageDeallocate(void* _BaseAddress, bool _UncommitAndUnreserve)
{
	DWORD flAllocationType, dwFreeType;
	size_t size = 0;
	oVERIFY(GetAllocationType(_UncommitAndUnreserve ? oPAGE_RESERVE_AND_COMMIT : oPAGE_COMMIT, _BaseAddress, false, &size, &flAllocationType, &dwFreeType));
	oVB(VirtualFreeEx(GetCurrentProcess(), _BaseAddress, 0, dwFreeType));
}

void* oPageAllocate(oPAGE_ALLOCATION_TYPE _AllocationType, void* _BaseAddress, size_t _Size, bool _UseLargePageSize)
{
	DWORD flAllocationType, dwFreeType;
	if (!GetAllocationType(_AllocationType, _BaseAddress, _UseLargePageSize, &_Size, &flAllocationType, &dwFreeType))
		return false; // pass through error

	DWORD flProtect = GetAccess(IsReadWrite(_AllocationType));

	void* p = VirtualAllocEx(GetCurrentProcess(), _BaseAddress, _Size, flAllocationType, flProtect);

	if (_BaseAddress && p != _BaseAddress)
	{
		// save error past cleanup, which might fail too..
		oWinSetLastError();
		errno_t err = oErrorGetLast();
		oStd::lstring errString = oErrorGetLastString();
		if (strstr(errString, "too small"))
			err = std::errc::no_buffer_space;

		if (p)
			oPageDeallocate(p, _AllocationType >= oPAGE_RESERVE_AND_COMMIT);

		oErrorSetLast(err, errString);
		return nullptr;
	}

	return p;
}

void* oPageReserve(void* _DesiredPointer, size_t _Size, bool _ReadWrite)
{
	return oPageAllocate(_ReadWrite ? oPAGE_RESERVE_READ_WRITE : oPAGE_RESERVE, _DesiredPointer, _Size, false);
}

void oPageUnreserve(void* _Pointer)
{
	oVB(VirtualFreeEx(GetCurrentProcess(), _Pointer, 0, MEM_RELEASE));
}

void* oPageCommit(void* _BaseAddress, size_t _Size, bool _ReadWrite, bool _UseLargePageSize)
{
	return oPageAllocate(_ReadWrite ? oPAGE_COMMIT_READ_WRITE : oPAGE_COMMIT, _BaseAddress, _Size, _UseLargePageSize);
}

void* oPageReserveAndCommit(void* _BaseAddress, size_t _Size, bool _ReadWrite, bool _UseLargePageSize)
{
	return oPageAllocate(_ReadWrite ? oPAGE_RESERVE_AND_COMMIT_READ_WRITE : oPAGE_RESERVE_AND_COMMIT, _BaseAddress, _Size, _UseLargePageSize);
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

bool oPageSetNoAccess( void* _BaseAddress, size_t _Size )
{
	DWORD oldPermissions = 0;
	oVB_RETURN(VirtualProtect(_BaseAddress, _Size, PAGE_NOACCESS, &oldPermissions));
	return true;
}

bool oPageSetPageablity(void* _BaseAddress, size_t _Size, bool _Pageable)
{
	oVB_RETURN(_Pageable ? VirtualUnlock(_BaseAddress, _Size) : VirtualLock(_BaseAddress, _Size));
	return true;
}
