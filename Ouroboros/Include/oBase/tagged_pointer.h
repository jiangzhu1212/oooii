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
#ifndef oBase_tagged_pointer_h
#define oBase_tagged_pointer_h

#include <oBase/compiler_config.h>
#include <atomic>
#include <exception>

namespace ouro {

template<typename T>
class tagged_pointer
{
public:
	#if o32BIT == 1
		static const size_t tag_bits = 3;
		static const size_t tag_shift = 0;
		static const size_t required_alignment = 1 << tag_bits; // in bytes
	#else
		static const size_t tag_bits = 8;
		static const size_t tag_shift = (sizeof(void*) * 8) - tag_bits;
		static const size_t required_alignment = oDEFAULT_MEMORY_ALIGNMENT; // in bytes
	#endif
	static const size_t tag_mask = ((1ull << tag_bits) - 1ull) << tag_shift;

	tagged_pointer() : tag_and_pointer(0) {}
	tagged_pointer(const tagged_pointer& _That) : tag_and_pointer((uintptr_t)_That.tag_and_pointer) {}
	tagged_pointer(tagged_pointer&& _That) : tag_and_pointer((uintptr_t)_That.tag_and_pointer) { _That.tag_and_pointer = 0; }
	const tagged_pointer<T>& operator=(const tagged_pointer<T>& _That) { tag_and_pointer = (uintptr_t)_That.tag_and_pointer; return *this; }
	tagged_pointer<T>& operator=(tagged_pointer<T>&& _That) { tag_and_pointer = (uintptr_t)_That.tag_and_pointer; _That.tag_and_pointer = 0; return *this; }
	bool operator==(const tagged_pointer<T>& _That) const { return tag_and_pointer == _That.tag_and_pointer; }
	bool operator!=(const tagged_pointer<T>& _That) const { return tag_and_pointer != _That.tag_and_pointer; }

	tagged_pointer(void* _pointer, size_t _tag)
	{
		#ifdef _DEBUG
		if (((uintptr_t)_pointer & tag_mask) != 0)
			throw std::exception("tagged_pointer pointers must be aligned to 8-bytes allow room for the tag");
		#endif
		tag_and_pointer = (uintptr_t)_pointer 
		#if o32BIT == 1
			| (uintptr_t)(_tag & tag_mask);
		#else
			| (uintptr_t)(_tag << tag_shift);
		#endif
	}
		
	size_t tag() const
	{
		#if o32BIT == 1
			return tag_and_pointer & tag_mask;
		#else
			return tag_and_pointer >> tag_shift;
		#endif
	}
	
	T* pointer() const { return (T*)(tag_and_pointer & ~(tag_mask)); }
	
	inline bool cas(tagged_pointer<T>& Old, const tagged_pointer<T>& New)
	{
		uintptr_t O = Old.tag_and_pointer;
		return tag_and_pointer.compare_exchange_strong(O, New.tag_and_pointer);
	}

private:
	std::atomic<uintptr_t> tag_and_pointer;
};

} // namespace ouro

#endif
