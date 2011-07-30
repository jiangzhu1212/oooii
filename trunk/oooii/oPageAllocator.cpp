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
#include <oooii/oPageAllocator.h>
#include <oooii/oAssert.h>
#include <oooii/oRefCount.h>
#include <oooii/oWindows.h>

static inline DWORD GetAccess(bool _ReadWrite) { return _ReadWrite ? PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ; }

static inline oPageAllocator::PAGE_STATUS GetStatus(DWORD _State)
{
	switch (_State)
	{
		case MEM_COMMIT: return oPageAllocator::COMMITTED;
		case MEM_FREE: return oPageAllocator::FREE;
		case MEM_RESERVE: return oPageAllocator::RESERVED;
		default: oASSUME(0);
	}
}

size_t oPageAllocator::GetPageSize()
{
	SYSTEM_INFO sysInfo = {0};
	GetSystemInfo(&sysInfo);
	return static_cast<size_t>(sysInfo.dwPageSize);
}

void oPageAllocator::GetRangeDesc(void* _BaseAddress, RANGE_DESC* _pRangeDesc)
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

void* oPageAllocator::Reserve(void* _DesiredPointer, size_t _Size, bool _ReadWrite)
{
	DWORD flAllocationType = MEM_RESERVE;
	if (_ReadWrite)
		flAllocationType |= MEM_WRITE_WATCH;

	void* p = VirtualAllocEx(GetCurrentProcess(), _DesiredPointer, _Size, flAllocationType, GetAccess(_ReadWrite));
	if (_DesiredPointer && p != _DesiredPointer)
	{
		oSetLastError(ENOMEM, "VirtualAllocEx return a pointer (0x%p) that is not the same as the requested pointer (0x%p).", p, _DesiredPointer);
		if (p) Unreserve(p);
		p = 0;
	}

	return p;
}

void oPageAllocator::Unreserve(void* _Pointer)
{
	oVB(VirtualFreeEx(GetCurrentProcess(), _Pointer, 0, MEM_RELEASE));
}

bool oPageAllocator::Commit(void* _BaseAddress, size_t _Size, bool _ReadWrite)
{
	void* p = VirtualAllocEx(GetCurrentProcess(), _BaseAddress, _Size, MEM_COMMIT, GetAccess(_ReadWrite));
	oVB_RETURN(p == _BaseAddress);
	return true;
}

bool oPageAllocator::Decommit(void* _Pointer)
{
	oVB_RETURN(VirtualFreeEx(GetCurrentProcess(), _Pointer, 0, MEM_DECOMMIT));
	return true;
}

bool oPageAllocator::SetReadWrite(void* _BaseAddress, size_t _Size, bool _ReadWrite)
{
	DWORD oldPermissions = 0;
	oVB_RETURN(VirtualProtect(_BaseAddress, _Size, GetAccess(_ReadWrite), &oldPermissions));
	return true;
}

bool oPageAllocator::SetPageablity(void* _BaseAddress, size_t _Size, bool _Pageable)
{
	oVB_RETURN(_Pageable ? VirtualUnlock(_BaseAddress, _Size) : VirtualLock(_BaseAddress, _Size));
	return true;
}
