/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
// Similar to Microsoft's InterlockedSList, this provides for a threadsafe
// singly linked list whose insertion has the behavior of a stack (LIFO).
// NOTE: The type T must include a member T* pNext. All allocation of nodes is
// the responsibility of the user because this data structure is often used in
// very low-level implementations such as allocations themselves.
#ifndef oConcurrentStack_h
#define oConcurrentStack_h

#include <oBasis/oAssert.h>
#include <oBasis/oPlatformFeatures.h>
#include <oBasis/oStdAtomic.h>
#include <oBasis/oThreadsafe.h>

template<typename T
		, size_t nTagBits = 4
		, size_t nSizeBits = 12
		, size_t nPointerBits = 48
>
class oConcurrentStack
{
	union header_t
	{
		unsigned long long All;
		struct
		{
			unsigned long long Tag : nTagBits;
			unsigned long long Size : nSizeBits;
			unsigned long long pHead : nPointerBits;
		};
	};

	header_t Head;

public:
	oConcurrentStack()
	{
		Head.All = 0;
	}

	~oConcurrentStack()
	{
		oASSERT(empty(), "Non-empty oConcurrentStack being freed");
	}

	static /*constexpr*/ size_t max_size() { return (1<<nSizeBits)-1; }
	inline bool empty() const threadsafe { return !Head.pHead; }
	inline size_t size() const threadsafe { return Head.Size; }

	inline bool try_push(T* _pElement) threadsafe
	{
		header_t New, Old;
		do 
		{
			Old.All = Head.All;
			if (Old.Size >= max_size())
				return false;
			_pElement->pNext = reinterpret_cast<T*>(Old.pHead);
			New.Tag = Old.Tag + 1;
			New.Size = Old.Size + 1;
			New.pHead = reinterpret_cast<uintptr_t>(_pElement);
			oASSERT(reinterpret_cast<T*>(New.pHead) == _pElement, "Truncation of pointer type occurred");
		} while (!oStd::atomic_compare_exchange(&Head.All, New.All, Old.All));
		return true;
	}

	inline void push(T* _pElement) threadsafe { while (!try_push(_pElement)); }

	inline T* peek() const threadsafe
	{
		header_t Old;
		Old.All = Head.All;
		return reinterpret_cast<T*>(Old.pHead);
	}

	inline T* pop() threadsafe
	{
		header_t New, Old;
		T* pElement = nullptr;
		do 
		{
			Old.All = Head.All;
			if (!Old.pHead)
				return nullptr;
			New.Tag = Old.Tag + 1;
			New.Size = Old.Size - 1;
			pElement = reinterpret_cast<T*>(Old.pHead);
			New.pHead = reinterpret_cast<uintptr_t>(pElement->pNext);
		} while (!oStd::atomic_compare_exchange(&Head.All, New.All, Old.All));
		return pElement;
	}

	inline T* pop_all()
	{
		T* pElement = nullptr;
		header_t New, Old;
		New.Size = 0;
		New.pHead = 0;
		do 
		{
			Old.All = Head.All;
			New.Tag = Old.Tag + 1;
			pElement = reinterpret_cast<T*>(Old.pHead);
		} while (!oStd::atomic_compare_exchange(&Head.All, New.All, Old.All));

		return pElement;
	}
};

#endif
