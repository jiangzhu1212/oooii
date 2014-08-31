// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oMirroredArena_h
#define oMirroredArena_h

#include <oBasis/oInterface.h>

// {89A0BD51-3ADF-42b6-A6DE-F0B9839168CF}
oDEFINE_GUID_I(oMirroredArena, 0x89a0bd51, 0x3adf, 0x42b6, 0xa6, 0xde, 0xf0, 0xb9, 0x83, 0x91, 0x68, 0xcf);
interface oMirroredArena : oInterface
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
		READ_WRITE_DIFF_NO_EXCEPTIONS,
	};

	struct DESC
	{
		DESC()
			: BaseAddress(0)
			, Size(0)
			, Usage(READ)
		{}

		// On create, this is the base address the user will use to allocate memory
		// from the underlying platform. It must be aligned to GetRequiredAlignment().
		void* BaseAddress;

		// Size of the arena. It must be less than GetMaxSize().
		size_t Size;

		// Permissions used on the arena
		USAGE Usage;
	};

	// RetrieveChanges will add a header to the returned buffer so query the size here
	static size_t GetHeaderSize();
	static size_t GetRequiredAlignment();
	static size_t GetMaxSize();

	// Interprets the specified buffer as a change buffer and returns it's total 
	// size.
	static size_t GetChangeBufferSize(const void* _pChangeBuffer);

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;

	// Returns the number of pages dirtied (written to) at the instantaneous 
	// moment this is called. Another immediate call to this might result in a 
	// different value because any number of threads might be dirtying memory in 
	// this arena.
	virtual size_t GetNumDirtyPages() const threadsafe = 0;

	// Analyzes the arena for changes since the last call to retrieve changes and 
	// creates a buffer of those changes. If successful, the buffer is filled as 
	// well as _pSizeRetrieved with the actual size of the change buffer. If there 
	// is a failure or the specified buffer is too small, this will return false. 
	// Use oErrorGetLast() to for further information.
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

	// Returns true if the specified user memory range has been fully recorded in
	// the specified change buffer populated by a call to RetrieveChanges.
	virtual bool IsInChanges(const void* _pAddress, size_t _Size, const void* _pChangeBuffer) const threadsafe = 0;
};

oAPI bool oMirroredArenaCreate(const oMirroredArena::DESC& _Desc, oMirroredArena** _ppMirroredArena);

#endif
