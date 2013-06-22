/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
// Interface for the lowest-level system allocator capable of handling pages of 
// memory that can map a very large address space to a combination of available 
// RAM and a pagefile/swapfile on disk.
#pragma once
#ifndef oPageAllocator_h
#define oPageAllocator_h

#include <oBasis/oStddef.h>
#include <oStd/macros.h>

enum oPAGE_STATUS
{
	oPAGE_FREE,
	oPAGE_RESERVED,
	oPAGE_COMMITTED,
};

struct oPAGE_RANGE_DESC
{
	// Whatever pointer was passed in, this is the start of its page.
	void* BaseAddress;

	// This is not the size of the pointer specified in oPageGetRangeDesc(), but 
	// rather the size of all pages from AllocationPageBase on that share the same
	// RANGE_DESC properties.
	size_t SizeInBytes;

	oPAGE_STATUS Status;
		
	// Current access
	bool ReadWrite;

	// Private means used only by this process
	bool IsPrivate;
};

// Returns the size of a default page on the current system. This is the minimum 
// atom that the page allocator allocates.
oAPI size_t oPageGetPageSize();

// Returns the size of a large (huge) page sized to reduce TLB cache misses.
// This is an advanced API. Unless client code is well-benchmarked to prove the 
// need for large pages, use oPageGetPageSize instead.
oAPI size_t oPageGetLargePageSize();

// Populates the specified oPAGE_RANGE_DESC with information about the longest 
// run of same-described memory starting at the specified base address.
oAPI void oPageGetRangeDesc(void* _BaseAddress, oPAGE_RANGE_DESC* _pRangeDesc);

// Prevents other memory allocation operations from accessing the specified 
// range. If _ReadWrite is false, the pages containing specified memory range 
// will throw a write access violation if the memory space is written to. This 
// can be changed with oPageSetReadWrite().
// If _DesiredPointer is not nullptr, then the return value can only be nullptr 
// on failure, or _DesiredPointer. If _DesiredPointer is nullptr, this will 
// return any available pointer that suits _Size.
oAPI void* oPageReserve(void* _DesiredPointer, size_t _Size, bool _ReadWrite = true);

// Allows other memory allocation operations to access the specified range (size 
// was determined in oPageReserve()). This automatically calls oPageDecommit() 
// if it hadn't been called on committed memory yet.
oAPI void oPageUnreserve(void* _Pointer);

// If _BaseAddress is not nullptr, then this backs up reserved memory with 
// actual storage and returns _BasedAddress. If _BaseAddress is nullptr, this 
// will allocate and return new memory, or nullptr on failure. If 
// _UseLargePageSize is true, then this can fail because the process cannot gain
// permission to allocate large page sizes.
oAPI void* oPageCommit(void* _BaseAddress, size_t _Size, bool _ReadWrite = true, bool _UseLargePageSize = false);

// Accomplishes Reserve and Commit in one operation
oAPI void* oPageReserveAndCommit(void* _BaseAddress, size_t _Size, bool _ReadWrite = true, bool _UseLargePageSize = false);

// Remove storage from the specified range (size was determined in 
// oPageCommit()). This will succeed/noop on already-decommited memory.
oAPI bool oPageDecommit(void* _Pointer);

// Set access on committed ranges only. If any page in the specified range is 
// not comitted, this will fail. _ReadWrite false will set memory to read-only. 
// Any writes to the memory will cause a write protection exception to be thrown.
oAPI bool oPageSetReadWrite(void* _BaseAddress, size_t _Size, bool _ReadWrite);

// Set access on committed ranges only. If any page in the specified range is 
// not committed, this will fail.  This allows no access to pages, useful when
// debugging
oAPI bool oPageSetNoAccess(void* _BaseAddress, size_t _Size);

// Allows pages of memory to be marked as non-swapable, thus ensuring there is 
// never a page fault for such pages.
oAPI bool oPageSetPageablity(void* _BaseAddress, size_t _Size, bool _Pageable);

//Be careful with this. allocates in multiples of pages. Probably bad to use for maps, lists, ect. small vectors probably not a good idea either.
//	should be fine for vectors that are usually at least 4k.
//	If you use this, may want to control growth manually (call reserve liberally at even page sized sizes), std::vector will probably grow and realloc at unfortunate sizes
template<typename T> struct oPageAllocator
{
	oDEFINE_STD_ALLOCATOR_BOILERPLATE(oPageAllocator)
	oPageAllocator() {}
	template<typename U> oPageAllocator(oPageAllocator<U> const& other) {}
	inline pointer allocate(size_type count, const_pointer hint = 0) { return static_cast<pointer>(oPageCommit(nullptr, sizeof(T) * count)); }
	inline void deallocate(pointer p, size_type count) { oPageDecommit(p); }
	inline const oPageAllocator& operator=(const oPageAllocator& other) {}
};

oDEFINE_STD_ALLOCATOR_VOID_INSTANTIATION(oPageAllocator)
oDEFINE_STD_ALLOCATOR_OPERATOR_EQUAL(oPageAllocator) { return true; }
//template aliases would be nice

#endif
