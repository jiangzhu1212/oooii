// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Simple non-threadsafe linear allocator
#pragma once
#ifndef oBase_linear_allocator_h
#define oBase_linear_allocator_h

#include <oBase/linear_allocator_base.h>

namespace ouro {

struct linear_allocator_traits
{
	typedef void* tail_type;
};

class linear_allocator : public linear_allocator_base<linear_allocator_traits>
{
public:
	linear_allocator() {}
	linear_allocator(void* _pArena, size_t _Size) : linear_allocator_base(_pArena, _Size) {}
	linear_allocator(linear_allocator&& _That) : linear_allocator_base(std::move(_That)) {}
	linear_allocator& operator=(linear_allocator&& _That)
	{
		linear_allocator_base::operator=(std::move((linear_allocator_base&&)_That));
		return *this;
	}
	
	// Allocates memory or nullptr if out of memory.
	void* allocate(size_t _Size, size_t _Alignment = oDEFAULT_MEMORY_ALIGNMENT);
	template<typename T> T* allocate(size_t _Size = sizeof(T), size_t _Alignment = oDEFAULT_MEMORY_ALIGNMENT) { return (T*)allocate(_Size, _Alignment); }

	// Reset the linear allocator to full availability
	void reset();
};

inline void* linear_allocator::allocate(size_t _Size, size_t _Alignment)
{ 
	void* p = byte_align(pTail, _Alignment);
	void* pNewTail = byte_add(p, _Size);
	if (pNewTail <= end)
	{
		pTail = pNewTail;
		return p;
	}
	return nullptr;
}

inline void linear_allocator::reset()
{
	pTail = pHead;
}

}

#endif
