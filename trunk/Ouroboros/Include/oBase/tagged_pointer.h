// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// A pointer that gives up some of its address space to protect against ABA 
// concurrency issues.
#pragma once
#ifndef oBase_tagged_pointer_h
#define oBase_tagged_pointer_h

#include <oCompiler.h>
#include <atomic>
#include <exception>

#if o32BIT == 1
	#define oTAGGED_POINTER_ALIGNMENT 8
#else
	#define oTAGGED_POINTER_ALIGNMENT 1
#endif

namespace ouro {

template<typename T>
class tagged_pointer
{
public:
	#if o32BIT == 1
		static const size_t tag_bits = 3;
		static const size_t tag_shift = 0;
	#else
		static const size_t tag_bits = 8;
		static const size_t tag_shift = (sizeof(void*) * 8) - tag_bits;
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

}

#endif
