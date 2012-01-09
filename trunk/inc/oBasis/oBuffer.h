// $(header)

// oBuffer is intended to be the minimal amount of wrapper code 
// on top of a void* to achieve the following:
// 1. Close association of the methodology to free the buffer
// 2. Provide a mechanism for transfer of ownership (refcounting)
// 3. Thread protection of the buffer
// This class uses the oLockedPointer style API to provide scoped
// locking and access to non-threadsafe methods.
#pragma once
#ifndef oBuffer_h
#define oBuffer_h

#include <oBasis/oFunction.h>
#include <oBasis/oInterface.h>

interface oBuffer : oLockableInterface
{
	typedef oFUNCTION<void(void*)> DeallocateFn;

	// Convenience implementations of common allocation functions
	static inline void* New(size_t _Size) { return new unsigned char[_Size]; }
	static inline void Delete(void* _Pointer) { delete [] _Pointer; }
	static inline void Noop(void* _Pointer) {}
	
	// Use an oLockedPointer to access GetData()
	virtual void* GetData() = 0;
	virtual const void* GetData() const = 0;

	template<typename T> T* GetData() { return (T*)GetData(); }
	template<typename T> const T* GetData() const { return (T*)GetData(); }

	virtual size_t GetSize() const = 0;

	// The name of the buffer is useful for debugging
	virtual const char* GetName() const threadsafe = 0;
};

// If _FreeFn is 0, then the allocation is not free automatically
bool oBufferCreate(const char* _Name, void* _Allocation, size_t _Size, oBuffer::DeallocateFn _DeallocateFn, oBuffer** _ppBuffer);
bool oBufferCreate(const char* _Name, const void* _Allocation, size_t _Size, oBuffer::DeallocateFn _DeallocateFn, const oBuffer** _ppBuffer);
bool oBufferCreate(const char* _Name, void* _Allocation, size_t _Size, oBuffer::DeallocateFn _DeallocateFn, threadsafe oBuffer** _ppBuffer);
bool oBufferCreate(const char* _Name, const void* _Allocation, size_t _Size, oBuffer::DeallocateFn _DeallocateFn, threadsafe const oBuffer** _ppBuffer);

#endif
