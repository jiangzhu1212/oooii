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

#include <oBase/byte.h>

namespace ouro {

class linear_allocator
{
public:
	static const size_t default_alignment = 16;

	linear_allocator() : pHead(nullptr), pTail(nullptr), pEnd(nullptr) {}
	linear_allocator(void* _pArena, size_t _Size) { initialize(_pArena, _Size); }
	linear_allocator(linear_allocator&& _That) { operator=(std::move(_That)); }
	linear_allocator& operator=(linear_allocator&& _That)
	{
		if (this != &_That)
		{
			pHead = _That.pHead; _That.pHead = nullptr;
			pTail = _That.pTail; _That.pTail = nullptr;
			pEnd = _That.pEnd; _That.pEnd = nullptr;
		}
		return *this;
	}

	void initialize(void* _pArena, size_t _Size) 
	{
		pHead = _pArena;
		pTail = _pArena;
		pEnd = byte_add(_pArena, _Size);
	}

	void* allocate(size_t _Size, size_t _Alignment = default_alignment)
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

	void reset() { pTail = pHead; }
	bool valid(void* _Pointer) const { return _Pointer >= pHead && _Pointer < pTail; }
	size_t bytes_free() const { return byte_diff(pTail, pEnd); }

private:
	void* pHead;
	void* pTail;
	void* pEnd;

	linear_allocator(const linear_allocator&); /* = delete */
	const linear_allocator& operator=(const linear_allocator&); /* = delete */
};

} // namespace ouro

#endif
