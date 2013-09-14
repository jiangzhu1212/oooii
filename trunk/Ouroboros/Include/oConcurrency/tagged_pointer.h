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
// A pointer that gives up some of its address space to protect against ABA 
// concurrency issues.
#pragma once
#ifndef oConcurrency_tagged_pointer_h
#define oConcurrency_tagged_pointer_h

#include <oConcurrency/thread_safe.h>
#include <oStd/atomic.h>
#include <stdexcept>

namespace oConcurrency {

template<
	typename T, 
	size_t TagBits = 
	// Reserve the low 3-bits on 32-bit systems so that default alignment usually
	// makes this negligible and client code can align allocs if not. The basic
	// assumption is that there won't be that much concurrency on 32-bit systems
	// anymore, since what is all that processing power working on? Not more than
	// 4 GB of memory!
	#ifdef o32BIT
		3
	#else 
		8
	#endif
>
class tagged_pointer
{
	#ifdef o32BIT
		static const size_t tag_shift = 0;
	#else
		static const size_t tag_shift = (sizeof(void*) * 8) - TagBits;
	#endif
	static const size_t tag_mask = ((1ull << TagBits) - 1ull) << tag_shift;

public:
	#ifdef o32BIT
		static const size_t required_alignment = 1 << TagBits; // in bytes
	#else
		static const size_t required_alignment = oDEFAULT_MEMORY_ALIGNMENT; // in bytes
	#endif

	tagged_pointer() : TagAndPointer(0) {}
	tagged_pointer(void* _Pointer, size_t _Tag)
	{
		#ifdef _DEBUG
		if (((uintptr_t)_Pointer & tag_mask) != 0)
			throw std::runtime_error("tagged_pointer pointers must be aligned to allow room for the tag");
		#endif
		TagAndPointer = (uintptr_t)_Pointer 
		#ifdef o32BIT
			| (uintptr_t)(_Tag & tag_mask);
		#else
			| (uintptr_t)(_Tag << tag_shift);
		#endif
	}
		
	tagged_pointer(const threadsafe tagged_pointer& _That) : TagAndPointer(_That.TagAndPointer) {}
	const tagged_pointer<T>& operator=(const tagged_pointer<T>& _That) { TagAndPointer = _That.TagAndPointer; return *this; }
	bool operator==(const tagged_pointer<T>& _That) const { return TagAndPointer == _That.TagAndPointer; }
	bool operator==(const threadsafe tagged_pointer<T>& _That) const threadsafe { return TagAndPointer == _That.TagAndPointer; }
	bool operator!=(const tagged_pointer<T>& _That) const { return TagAndPointer != _That.TagAndPointer; }
	bool operator!=(const threadsafe tagged_pointer<T>& _That) const threadsafe { return TagAndPointer != _That.TagAndPointer; }
	size_t tag() const
	{
		#ifdef o32BIT
			return TagAndPointer & tag_mask;
		#else
			return TagAndPointer >> tag_shift;
		#endif
	}
	
	T* ptr() const { return (T*)(TagAndPointer & ~(tag_mask)); }
	
	static inline bool CAS(threadsafe tagged_pointer<T>* Destination, const tagged_pointer<T>& New, const tagged_pointer<T>& Old)
	{
		return oStd::atomic_compare_exchange(&Destination->TagAndPointer, New.TagAndPointer, Old.TagAndPointer);
	}

private:
	uintptr_t TagAndPointer;
};

} // namespace oConcurrency

#endif
