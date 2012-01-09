// $(header)
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
	#ifdef o64BIT
		, size_t nTagBits = 4
		, size_t nSizeBits = 12
		, size_t nPointerBits = 48
	#else
		, size_t nTagBits = 2
		, size_t nSizeBits = 4
		, size_t nPointerBits = 26
	#endif
>
class oConcurrentStack
{
	union header_t
	{
		uintptr_t All;
		struct
		{
			uintptr_t Tag : nTagBits;
			uintptr_t Size : nSizeBits;
			uintptr_t pHead : nPointerBits;
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
