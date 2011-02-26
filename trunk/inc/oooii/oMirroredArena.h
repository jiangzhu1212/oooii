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
#pragma once
#ifndef oMirroredArena_h
#define oMirroredArena_h

#include <oooii/oInterface.h>

interface oMirroredArena : public oInterface
{
	// Allocates a large block of memory fit to be used with a custom allocator.
	// This API exposes the platform's memory protection and uses that mechanism
	// to determine the minimal set of changes to the arena's contents. This is 
	// useful when implementing push-buffer patterns and synchronizing multiple 
	// systems.

	// Arenas must fit strict alignment and max size requirements to facility
	// efficient internal implementation. Currently there isn't much advantage to 
	// creating an arena smaller than the value returned by GetMaxSize().

	enum USAGE
	{
		READ,
		READ_WRITE,
		READ_WRITE_DIFF,
	};

	struct DESC
	{
		DESC()
			: BaseAddress(0)
			, Size(0)
			, Usage(READ)
			, bIsReserved(false)
			, bIsCommitted(false)
		{}

		// On create, this is the base address the user will use to allocate memory
		// from the underlying platform. It must be aligned to GetRequiredAlignment().
		void* BaseAddress;

		// Size of the arena. It must be less than GetMaxSize().
		size_t Size;

		// Permissions used on the arena
		USAGE Usage;

		// True if this memory has been reserved.
		bool bIsReserved;

		// True if this memory has been committed.
		bool bIsCommitted;

		// Can only commit on reserved, uncommitted memory.
		bool bCanCommitMemory(){return (bIsReserved && !bIsCommitted);};
	};

	// RetrieveChanges will add a header to the returned buffer so query the size here
	static size_t GetHeaderSize();
	static size_t GetRequiredAlignment();
	static size_t GetMaxSize();

	// Interprets the specified buffer as a change buffer and returns it's total 
	// size.
	static size_t GetChangeBufferSize(const void* _pChangeBuffer);
	
	static bool Create(const DESC* _pDesc, oMirroredArena** _ppMirroredArena);

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;

	// Returns the number of pages dirtied (written to) at the instantaneous 
	// moment this is called. Anohter immediate call to this might result in a 
	// different value because any number of threads might be dirtying memory in 
	// this arena.
	virtual size_t GetNumDirtyPages() const threadsafe = 0;

	// Analyzes the arena for changes since the last call to retrieve changes and 
	// creates a buffer of those changes. If successful, the buffer is filled as 
	// well as _pSizeRetrieved with the actual size of the change buffer. If there 
	// is a failure or the specified buffer is too small, this will return false. 
	// Use oGetLastError() to for further information.
	//
	// This API is threadsafe even in the sense that any memory write into the 
	// arena during this call will either be reflected in _pChangeBuffer, or will
	// be locked out of the system until RetrieveChanges are finished. So while
	// ultimately all memory operations will be correct, applying these changes to
	// a remote computer and attempting to use this memory should be done with 
	// care because a buffer whose contents is expected to be wholly there might
	// have had its memcpy paused on the source side and thus not all the data is 
	// in its final state. Use with caution.
	//
	// (Intended usage of this feature. Large data writes such as loading a file
	// into memory or other lengthy operations might take a while, and other 
	// application logic can easily guard against the buffer's usage until the 
	// operation is complete. However it's nice to get started on moving the large
	// buffer to the remote computer and ApplyChanges(), thus balancing out the 
	// amount of data being transfered, and not having to wait doing little and 
	// then a large transfer at the end of the I/O.)
	virtual bool RetrieveChanges(void* _pChangeBuffer, size_t _SizeofChangeBuffer, size_t* _pSizeRetrieved) threadsafe = 0;

	// Apply the provided change buffer.
	virtual bool ApplyChanges(const void* _pChangeBuffer) = 0;

	// Reserves a size (determined by _pDesc) of virtual memory.  This marks the 
	// memory as reserved, but does not allocate and must be called before 
	// committing the memory.
	virtual void* VMemoryReserve(DESC& _pDesc) = 0;

	// Releases virtual memory back into the wild.  This will also decomit the
	// memory, so VMemoryDecommit() does not need to be called when unreserving.
	virtual void  VMemoryUnreserve(DESC& _pDesc) = 0;

	// Commit already reserved memory to virtual space.  This will fail if the 
	// memory is not reserved.
	virtual void* VMemoryCommit(DESC& _pDesc) = 0;

	// Marks the memory as ready to be re-allocated, but keeps it reserved so
	// malloc and LocalAlloc calls won't write into it.
	virtual void  VMemoryDecommit(DESC& _pDesc) = 0;
};

#endif
