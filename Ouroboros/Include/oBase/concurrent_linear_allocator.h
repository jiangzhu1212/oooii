// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Simple concurrent lienar allocator using atomics
#pragma once
#ifndef oBase_concurrent_linear_allocator_h
#define oBase_concurrent_linear_allocator_h

#include <oBase/linear_allocator_base.h>
#include <atomic>

namespace ouro {

struct concurrent_linear_allocator_traits
{
	typedef std::atomic<void*> tail_type;
};

class concurrent_linear_allocator : public linear_allocator_base<concurrent_linear_allocator_traits>
{
	// NOTE: None of the inherited APIs are thread safe - only allocate() and 
	// reset() are thread safe.

public:
	concurrent_linear_allocator() {}
	concurrent_linear_allocator(void* _pArena, size_t _Size) : linear_allocator_base(_pArena, _Size) {}
	concurrent_linear_allocator(concurrent_linear_allocator&& _That) : linear_allocator_base(std::move(_That)) {}
	concurrent_linear_allocator& operator=(concurrent_linear_allocator&& _That)
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

inline void* concurrent_linear_allocator::allocate(size_t _Size, size_t _Alignment)
{
	void* pNew, *pOld, *pAligned;
	pOld = pTail;
	do
	{
		pAligned = byte_align(pOld, _Alignment);
		pNew = byte_add(pAligned, _Size);
		if (pNew > end)
			return nullptr;
	} while (!pTail.compare_exchange_strong(pOld, pNew));
	return pAligned;
}

inline void concurrent_linear_allocator::reset()
{
	pTail.store(pHead);
}

}

#endif
