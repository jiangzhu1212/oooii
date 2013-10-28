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

// {714C9432-EBF6-4232-9E2E-90692C294B8B}
oDEFINE_GUID_I(oBuffer, 0x714c9432, 0xebf6, 0x4232, 0x9e, 0x2e, 0x90, 0x69, 0x2c, 0x29, 0x4b, 0x8b);
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

// If _FreeFn is nullptr, then the allocation is not freed automatically
bool oBufferCreate(const char* _Name, void* _Allocation, size_t _Size, oBuffer::DeallocateFn _DeallocateFn, oBuffer** _ppBuffer);
bool oBufferCreate(const char* _Name, const void* _Allocation, size_t _Size, oBuffer::DeallocateFn _DeallocateFn, const oBuffer** _ppBuffer);

// For static buffers
template<typename T, size_t size> bool oBufferCreate(const char* _Name, const T (&_Allocation)[size], const oBuffer** _ppBuffer) { return oBufferCreate(_Name, _Allocation, size * sizeof(T), oBuffer::Noop, _ppBuffer); }

#endif
