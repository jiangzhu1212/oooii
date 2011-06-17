// $(header)
// Interface for the lowest-level system allocator
// capable of handling pages of memory that can map
// a very large address space to a combination of 
// available RAM and a pagefile/swapfile on disk.
#pragma once
#ifndef oPageAllocator_h
#define oPageAllocator_h

#include <oooii/oStddef.h>

namespace oPageAllocator
{
	enum PAGE_STATUS
	{
		FREE,
		RESERVED,
		COMMITTED,
	};

	struct RANGE_DESC
	{
		// Whatever pointer was passed in, this is the start
		// of its page.
		void* BaseAddress;

		// This is not the size of the pointer specified in
		// GetRangeDesc(), but rather the size of all pages
		// from AllocationPageBase on that share the same
		// RANGE_DESC properties.
		size_t SizeInBytes;

		PAGE_STATUS Status;
		
		// Current access
		bool ReadWrite;

		// Private means used only by this process
		bool IsPrivate;
	};

	oAPI size_t GetPageSize();

	oAPI void GetRangeDesc(void* _BaseAddress, RANGE_DESC* _pRangeDesc);

	// Prevents other memory allocation operations from accessing
	// the specified range. If _ReadWrite is false, the pages 
	// containing specified memory range will throw a write access
	// violation if the memory space is written to. This can be 
	// changed with SetProtection().
	// If _DesiredPointer is not NULL, then the return value can
	// only be NULL on failure, or _DesiredPointer. If _DesiredPointer
	// is NULL, this will return any available pointer that suits _Size.
	oAPI void* Reserve(void* _DesiredPointer, size_t _Size, bool _ReadWrite = true);
	
	// Allows other memory allocation operations to access the 
	// specified range (size was determined in Reserve()). This 
	// automatically calls Decommit() if it hadn't been called 
	// on committed memory yet.
	oAPI void Unreserve(void* _Pointer);

	// Back up reserved memory with actual storage. This will fail
	// if called on unreserved memory, but will succeed/noop on 
	// already-committed memory.
	oAPI bool Commit(void* _BaseAddress, size_t _Size, bool _ReadWrite = true);

	// Remove storage from the specified range (size was determined
	// in Commit()). This will succeed/noop on already-decommited 
	// memory.
	oAPI bool Decommit(void* _Pointer);

	// Set access on committed ranges only. If any page in the specified 
	// range is not comitted, this will fail. _ReadWrite false will set memory
	// to read-only. Any writes to the memory will cause a write protection
	// exception to be thrown.
	oAPI bool SetReadWrite(void* _BaseAddress, size_t _Size, bool _ReadWrite);

	// Allows pages of memory to be marked as non-swapable, thus 
	// ensuring there is never a page fault on for such pages.
	oAPI bool SetPageablity(void* _BaseAddress, size_t _Size, bool _Pageable);
}

#endif
