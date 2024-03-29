// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// Interface for the lowest-level system allocator capable of handling pages of 
// memory that can map a very large address space to a combination of available 
// RAM and a pagefile/swapfile on disk.

#pragma once

namespace ouro { namespace page_allocator {

/* enum class */ namespace status
{	enum value {
	
	free,
	reserved,
	committed,

};}

/* enum class */ namespace access
{	enum value {

	none,
	read_only,
	read_write,

};}

struct range
{
	// Whatever pointer was passed in, this is the start of its page
	void* base;

	// Size of all pages from base on that share the same properties
	size_t size;

	status::value status;
		
	// Current access (false means read-only)
	bool read_write;

	// Private means used only by this process
	bool is_private;
};

size_t page_size();
size_t large_page_size();

// Returns a description of the longest run from the specified address of pages
// that share the same properties.
range get_range(void* _Base);

// Prevents other page_allocator operations from accessing the specified memory 
// range. If _ReadWrite is false the pages containing specified memory range 
// will throw a write access violation if the memory space is written to. This 
// can be changed with set_read_write(). If _DesiredPointer is not nullptr then 
// the return value can only be nullptr on failure, or _DesiredPointer. If 
// _DesiredPointer is nullptr, this will return any available pointer that suits 
// _Size.
void* reserve(void* _DesiredPointer, size_t _Size, bool _ReadWrite = true);

// Allows other memory allocation operations to access the specified range (size 
// was determined in reserve). This automatically calls decommit if it hadn't 
// been called on committed memory yet.
void unreserve(void* _Pointer);

// If _BaseAddress is not nullptr then this backs up reserved memory with 
// actual storage and returns _BasedAddress. If _BaseAddress is nullptr this 
// will allocate and return new memory or nullptr on failure. If 
// _UseLargePageSize is true then this can fail because the process cannot gain
// permission to allocate large page sizes.
void* commit(void* _BaseAddress, size_t _Size, bool _ReadWrite = true, bool _UseLargePageSize = false);

// Remove storage from the specified range (size was determined in commit). This 
// will succeed/noop on already-decommited memory.
void decommit(void* _Pointer);

// Accomplishes reserve and commit in one operation
void* reserve_and_commit(void* _BaseAddress, size_t _Size, bool _ReadWrite = true, bool _UseLargePageSize = false);

// Set access on committed ranges only. If any page in the specified range is 
// not comitted, this will fail. Violating the access policy will raise an 
// exception.
void set_access(void* _BaseAddress, size_t _Size, access::value _Access);

// Allows pages of memory to be marked as non-swapable, thus ensuring there is 
// never a page fault for such pages.
void set_pagability(void* _BaseAddress, size_t _Size, bool _Pageable);

}}
