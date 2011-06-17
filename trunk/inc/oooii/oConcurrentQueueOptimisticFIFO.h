// $(header)

// A thread-safe queue that uses atomics to ensure concurrency.
// What makes this implementation unique is it aggressively avoids
// atomic operations and goes back and fixes any errors introduced.
// For more information, follow the link below.
#pragma once
#ifndef oConcurrentQueueOptimisticFIFO_h
#define oConcurrentQueueOptimisticFIFO_h

#include <oooii/oConcurrentPooledAllocator.h>
#include <oooii/oTaggedPointer.h> 

template<typename T, typename Alloc = std::allocator<char> >
class oConcurrentQueueOptimisticFIFO
{
	/** <citation
		usage="Paper" 
		reason="Authors claim this is significantly more efficient than the MS-queue." 
		author="Edya Ladan-Mozes and Nir Shavit"
		description="http://people.csail.mit.edu/edya/publications/OptimisticFIFOQueue-journal.pdf"
	/>*/

	struct NODE
	{
		oTaggedPointer<NODE> Prev;
		oTaggedPointer<NODE> Next;
		T Value;
	};

	typedef oTaggedPointer<NODE> PTR;

	PTR Head;
	PTR Tail;
	typedef oConcurrentPooledAllocator<NODE, Alloc> AllocatorT;
	AllocatorT Pool;

	void fixList(const PTR& tail, const PTR& head) threadsafe
	{
		PTR curNext, cur = tail;
		while ((head == Head) && (cur != head))
		{
			curNext = cur->Next;
			curNext->Prev = PTR(cur.GetPointer(), cur.GetTag()-1);
			cur = PTR(curNext.GetPointer(), cur.GetTag()-1);
		}
	}

public:
	oConcurrentQueueOptimisticFIFO(const char* _DebugName, typename AllocatorT::PoolInitType _Type = typename AllocatorT::InitElementCount, size_t _Value = 100000, const Alloc& _Allocator = Alloc())
		: Pool(_DebugName, _Type, _Value, _Allocator)
	{
		NODE* n = (NODE*)Pool.Allocate();
		n->Next = PTR(0, 0);
		Head = Tail = PTR(n, 0);
	}

	~oConcurrentQueueOptimisticFIFO()
	{
		oASSERT(IsEmpty(), "oConcurrentQueueOptimisticFIFO %s not empty", GetDebugName());
		NODE* n = Head;
		Head = Tail = PTR(0, 0);
		Pool.Deallocate(n);
	}

	bool TryPush(const T& _Element) threadsafe
	{
		NODE* n = (NODE*)Pool.Allocate();
		if (!n) return false;
		n->Value = _Element;
		while (1)
		{
			PTR t = Tail;
			n->Next = PTR(t.GetPointer(), t.GetTag()+1);
			if (PTR::CAS(&Tail, PTR(n, t.GetTag()+1), t))
			{
				t->Prev = PTR(n, t.GetTag());
				return true;
			}
		}
		
		return false;
	}

	bool TryPop(T& _Element) threadsafe
	{
		PTR t, h, firstPrev;
		while (1)
		{
			h = Head;
			t = Tail;
			firstPrev = h->Prev;
			if (h == Head)
			{
				if (t != h)
				{
					if (firstPrev.GetTag() != h.GetTag())
					{
						fixList(t, h);
						continue;
					}
					
					_Element = firstPrev->Value;
					if (PTR::CAS(&Head, PTR(firstPrev.GetPointer(), h.GetTag()+1), h))
					{
						Pool.Deallocate(h);
						return true;
					}
				}

				else
					break;
			}
		}

		return false;
	}

	inline const char* GetDebugName() const threadsafe { return Pool.GetDebugName(); }
	inline bool IsValid() const { return Pool.IsValid(); }
	inline void Clear() threadsafe { T e; while (TryPop(e)); }
	inline bool IsEmpty() const { return Head == Tail; }
	inline size_t GetCapacity() const threadsafe { return Pool.GetCapacity(); }
	inline size_t GetSize() const { return Pool.GetSize(); } // SLOW!
	inline void Push(const T& _Element) threadsafe { while(!TryPush(_Element)); }
	inline void Pop(T& _Element) threadsafe { while(!TryPop(_Element)); }
};

#endif
