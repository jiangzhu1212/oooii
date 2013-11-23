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
#include <oCore/page_allocator.h>
#include <oCore/windows/win_error.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

using namespace oStd;

namespace ouro {
	namespace page_allocator {

static status::value get_status(DWORD _State)
{
	switch (_State)
	{
		case MEM_COMMIT: return status::committed;
		case MEM_FREE: return status::free;
		case MEM_RESERVE: return status::reserved;
		oNODEFAULT;
	}
}

static DWORD get_access(access::value _Access)
{
	switch (_Access)
	{
		case access::none: return PAGE_NOACCESS;
		case access::read_only: return PAGE_EXECUTE_READ;
		case access::read_write: return PAGE_EXECUTE_READWRITE;
		oNODEFAULT;
	}
}

size_t page_size()
{
	SYSTEM_INFO sysInfo = {0};
	GetSystemInfo(&sysInfo);
	return static_cast<size_t>(sysInfo.dwPageSize);
}

size_t large_page_size()
{
	return GetLargePageMinimum();
}

range get_range(void* _Base)
{
	MEMORY_BASIC_INFORMATION mbi;
	#ifdef oENABLE_ASSERTS
		size_t returnSize = 
	#endif
	VirtualQuery(_Base, &mbi, sizeof(mbi));
	oASSERT(sizeof(mbi) == returnSize, "");
	range r;
	r.base = mbi.BaseAddress;
	r.size = mbi.RegionSize;
	r.status = get_status(mbi.State);
	r.read_write = (mbi.AllocationProtect & PAGE_EXECUTE_READWRITE) || (mbi.AllocationProtect & PAGE_READWRITE);
	r.is_private = mbi.Type == MEM_PRIVATE;
	return r;
}

/* enum class */ namespace allocation_type
{	enum value {

	reserve,
	reserve_read_write,
	commit,
	commit_read_write,
	reserve_and_commit,
	reserve_and_commit_read_write,

};}

// This will populate the flags correctly, and adjust size to be aligned if 
// large page sizes are to be used.
static void get_allocation_type(allocation_type::value _AllocationType, void* _BaseAddress, bool _UseLargePageSize, size_t* _pSize, DWORD* _pflAllocationType, DWORD* _pdwFreeType)
{
	*_pflAllocationType = 0;
	*_pdwFreeType = 0;

	if (_UseLargePageSize)
	{
		if (_BaseAddress && (_AllocationType == allocation_type::reserve || _AllocationType == allocation_type::reserve_read_write))
			throw std::invalid_argument("large page memory cannot be reserved");

		*_pflAllocationType |= MEM_LARGE_PAGES;
		*_pSize = byte_align(*_pSize, large_page_size());
	}

	switch (_AllocationType)
	{
		case allocation_type::reserve_read_write: *_pflAllocationType |= MEM_WRITE_WATCH; // pass thru to allocation_type::reserve
		case allocation_type::reserve: *_pflAllocationType |= MEM_RESERVE; *_pdwFreeType = MEM_RELEASE; break;
		case allocation_type::commit_read_write: // MEM_WRITE_WATCH is not a valid option for committing
		case allocation_type::commit: *_pflAllocationType |= MEM_COMMIT; *_pdwFreeType = MEM_DECOMMIT; break;
		case allocation_type::reserve_and_commit_read_write: *_pflAllocationType |= MEM_WRITE_WATCH; // pass thru to allocation_type::reserve_and_commit
		case allocation_type::reserve_and_commit: *_pflAllocationType |= MEM_RESERVE | MEM_COMMIT; *_pdwFreeType = MEM_RELEASE | MEM_DECOMMIT; break;
		oNODEFAULT;
	}
}

static void deallocate(void* _BaseAddress, bool _UncommitAndUnreserve)
{
	DWORD flAllocationType, dwFreeType;
	size_t size = 0;
	get_allocation_type(_UncommitAndUnreserve ? allocation_type::reserve_and_commit : allocation_type::commit, _BaseAddress, false, &size, &flAllocationType, &dwFreeType);
	oVB(VirtualFreeEx(GetCurrentProcess(), _BaseAddress, 0, dwFreeType));
}

static void* allocate(allocation_type::value _AllocationType
	, void* _BaseAddress
	, size_t _Size
	, bool _UseLargePageSize)
{
	DWORD flAllocationType, dwFreeType;
	get_allocation_type(_AllocationType, _BaseAddress, _UseLargePageSize, &_Size, &flAllocationType, &dwFreeType);
	DWORD flProtect = get_access((_AllocationType & 0x1) ? access::read_write : access::read_only);
	void* p = VirtualAllocEx(GetCurrentProcess(), _BaseAddress, _Size, flAllocationType, flProtect);
	if (_BaseAddress && p != _BaseAddress)
	{
		deallocate(p, _AllocationType >= allocation_type::reserve_and_commit);
		oTHROW0(no_buffer_space);
	}
	return p;
}

void* reserve(void* _DesiredPointer, size_t _Size, bool _ReadWrite)
{
	return allocate(_ReadWrite 
		? allocation_type::reserve_read_write 
		: allocation_type::reserve
		, _DesiredPointer
		, _Size
		, false);
}

void unreserve(void* _Pointer)
{
	oVB(VirtualFreeEx(GetCurrentProcess(), _Pointer, 0, MEM_RELEASE));
}

void* commit(void* _BaseAddress, size_t _Size, bool _ReadWrite, bool _UseLargePageSize)
{
	return allocate(_ReadWrite 
		? allocation_type::commit_read_write 
		: allocation_type::commit
		, _BaseAddress
		, _Size
		, _UseLargePageSize);
}

void decommit(void* _Pointer)
{
	oVB(VirtualFreeEx(GetCurrentProcess(), _Pointer, 0, MEM_DECOMMIT));
}

void* reserve_and_commit(void* _BaseAddress, size_t _Size, bool _ReadWrite, bool _UseLargePageSize)
{
	return allocate(_ReadWrite 
		? allocation_type::reserve_and_commit_read_write 
		: allocation_type::reserve_and_commit
		, _BaseAddress
		, _Size
		, _UseLargePageSize);
}

void set_access(void* _BaseAddress, size_t _Size, access::value _Access)
{
	DWORD oldPermissions = 0;
	oVB(VirtualProtect(_BaseAddress, _Size, get_access(_Access), &oldPermissions));
}

void set_pagability(void* _BaseAddress, size_t _Size, bool _Pageable)
{
	oVB(_Pageable ? VirtualUnlock(_BaseAddress, _Size) : VirtualLock(_BaseAddress, _Size));
}

	} // namespace page_allocator
} // namespace ouro
