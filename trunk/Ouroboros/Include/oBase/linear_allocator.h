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
	if (pNewTail <= pEnd)
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

} // namespace ouro

#endif
