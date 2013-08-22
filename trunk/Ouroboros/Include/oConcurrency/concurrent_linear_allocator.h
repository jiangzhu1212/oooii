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
// Simple allocator that has a fixed arena of memory and increments an index 
// with each allocation until all memory is used up. There is no deallocate(), 
// but reset() will set the index back to zero. This is useful for containers
// such as std::maps that are built up and searched and whose data is simple
// (i.e. no ref counting or handles/pointers that need to be cleaned up). Use of 
// this allocator can alleviate long destructor times in such containers where
// there's not a lot of use to the destruction because of the simple types.
//
// This uses the oversized-allocation pattern where the full arena is allocated
// and this class is overlaid on top of it to manage the buffer.
//
// Allocation is O(1) and a single CAS is used to protect concurrency.
#pragma once
#ifndef oConcurrency_concurrent_linear_allocator_h
#define oConcurrency_concurrent_linear_allocator_h

#include <oConcurrency/thread_safe.h>
#include <oStd/byte.h>
#include <oStd/oStdAtomic.h>

namespace oConcurrency {

class concurrent_linear_allocator
{
public:

	// This does not do much, since this object assumes it owns memory beyond 
	// sizeof(T). Call initialize to validate this object.
	concurrent_linear_allocator();
	
	// Sets this allocator to track allocations for the specified number of bytes
	// beyond the this pointer.
	void initialize(size_t _ArenaSize);

	// Allocates the specified number of bytes
	inline void* allocate(size_t _Size) threadsafe;

	// Allocates an object-sized buffer
	template<typename T>
	T* allocate(size_t _Size = sizeof(T)) threadsafe;

	// Resets the allocator to be empty. This might leave client code pointers
	// dangling.
	void reset() threadsafe;

	// Returns true if the specified pointer is in the range of this object's 
	// arena.
	bool valid(void* _Pointer) const threadsafe;

	// Returns how many bytes are left to be allocated.
	size_t bytes_available() const threadsafe;

private:
	void* Head;
	void* End;

	void* begin() threadsafe const { return (void*)(this+1); }
};

inline concurrent_linear_allocator::concurrent_linear_allocator()
	: Head(nullptr)
	, End(nullptr)
{}

inline void concurrent_linear_allocator::initialize(size_t _ArenaSize)
{
	End = oStd::byte_add(this, _ArenaSize);
	reset();
}

inline void* concurrent_linear_allocator::allocate(size_t _Size) threadsafe
{
	void* New, *Old;
	do
	{
		Old = Head;
		New = oStd::byte_add(Old, _Size);
		if (New > End)
			return nullptr;
		New = oStd::byte_align(New, oDEFAULT_MEMORY_ALIGNMENT);
	} while (!oStd::atomic_compare_exchange(&Head, New, Old));
	return Old;
}

template<typename T>
inline T* concurrent_linear_allocator::allocate(size_t _Size) threadsafe
{
	return reinterpret_cast<T*>(allocate(_Size));
}

inline void concurrent_linear_allocator::reset() threadsafe
{
	oStd::atomic_exchange(&Head, begin());
}

inline bool concurrent_linear_allocator::valid(void* _Pointer) const threadsafe
{
	return oStd::in_range(_Pointer, begin(), Head);
}

inline size_t concurrent_linear_allocator::bytes_available() const threadsafe
{
	ptrdiff_t diff = oStd::byte_diff(End, Head);
	return diff > 0 ? diff : 0;
}

} // namespace oConcurrency

#endif
