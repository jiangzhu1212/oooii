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
#ifndef oBuffer_h
#define oBuffer_h

#include <oooii/oInterface.h>
#include <memory.h>

interface oBuffer : public oInterface
{
	typedef oFUNCTION<void(void*)> DeallocateFn;

	// Convenience implementations of common free functions
	static inline void Delete(void* _Pointer) { delete [] _Pointer; }
	static inline void Noop(void* _Pointer) {}

	// If _FreeFn is 0, then the allocation is not free automatically
	static bool Create(const char* _Name, void* _Allocation, size_t _Size, DeallocateFn _DeallocateFn, threadsafe oBuffer** _ppBuffer);

	// Load a file into a newly allocated buffer using malloc to allocate the memory.
	static bool Create(const char* _Path, bool _IsText, threadsafe oBuffer** _ppBuffer);

	virtual void Lock() threadsafe = 0;
	virtual void LockRead() const threadsafe = 0;
	virtual void Unlock() threadsafe = 0;
	virtual void UnlockRead() const threadsafe = 0;
	
	// Use an oLockedPointer to access GetData()
	virtual void* GetData() = 0;
	virtual const void* GetData() const = 0;

	template<typename T> T* GetData() { return (T*)GetData(); }
	template<typename T> const T* GetData() const { return (T*)GetData(); }

	// const and threadsafe because the size of an oBuffer does not change over its 
	// lifetime (no realloc)
	virtual size_t GetSize() const threadsafe = 0;
	virtual const char* GetName() const threadsafe = 0;
	
	// Convenience function for calling an all- or partial-buffer update.
	void Update(const void* _pData, size_t _SizeOfData) threadsafe;

	// Convenience function for retrieving a copy of the specified buffer. Returns 
	// the number of bytes written to the specified destination.
	size_t Read(void** _ppDestination, size_t _SizeofDestination) const threadsafe;
};

// oLockedPointer support
inline void intrusive_ptr_lock(threadsafe oBuffer* p) { p->Lock(); }
inline void intrusive_ptr_unlock(threadsafe oBuffer* p) { p->Unlock(); }
inline void intrusive_ptr_lock_read(threadsafe oBuffer* p) { p->LockRead(); }
inline void intrusive_ptr_unlock_read(threadsafe oBuffer* p) { p->UnlockRead(); }

interface oBufferPool : oInterface
{
	// The buffer pool breaks a larger allocation into several smaller buffers and recycles the memory as buffers go out of scope.
	// There is no explicit return as this is handled purely through the oBuffer's refcount
	static bool Create(const char* _Name, void* _Allocation, size_t _AllocationSize, size_t _IndividualBufferSize, oBuffer::DeallocateFn _DeallocateFn, threadsafe oBufferPool** _ppBuffer);
	virtual bool GetFreeBuffer(threadsafe oBuffer** _ppBuffer) threadsafe = 0;
};

#endif
