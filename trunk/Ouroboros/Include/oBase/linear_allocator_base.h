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
// Code common to thread local and concurrent linear allocators
#pragma once
#ifndef oBase_linear_allocator_base_h
#define oBase_linear_allocator_base_h

#include <oBase/byte.h>
#include <oBase/config.h>

namespace ouro {

class linear_allocator_base
{
public:
	static const size_t default_alignment = oDEFAULT_MEMORY_ALIGNMENT;

	linear_allocator_base() : pHead(nullptr), pTail(nullptr), pEnd(nullptr) {}
	linear_allocator_base(void* _pArena, size_t _Size) { initialize(_pArena, _Size); }
	linear_allocator_base(linear_allocator_base&& _That) { operator=(std::move(_That)); }
	linear_allocator_base& operator=(linear_allocator_base&& _That)
	{
		if (this != &_That)
		{
			pHead = _That.pHead;
			pTail = _That.pTail;
			pEnd = _That.pEnd;
			_That.deinitialize();
		}
		return *this;
	}

	// Returns the area used to initialize this instance
	void* get_arena() const { return pHead; }

	// Associated the specified area with this allocator to be managed
	void initialize(void* _pArena, size_t _Size) 
	{
		pHead = _pArena;
		pTail = _pArena;
		pEnd = byte_add(_pArena, _Size);
	}

	// Eviscerates this instance
	void deinitialize()
	{
		pHead = pTail = pEnd = nullptr;
	}

	// tests if a pointer is in the arena of this instance
	bool valid(void* _Pointer) const { return _Pointer >= pHead && _Pointer < pEnd; }

	// returns the number of bytes available
	size_t bytes_free() const { return byte_diff(pTail, pEnd); }

protected:
	void* pHead;
	void* pTail;
	void* pEnd;

	linear_allocator_base(const linear_allocator_base&); /* = delete */
	const linear_allocator_base& operator=(const linear_allocator_base&); /* = delete */
};

} // namespace ouro

#endif
