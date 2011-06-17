// $(header)

// A thread-safe queue that uses atomics to ensure concurrency.
#pragma once
#ifndef oConcurrentQueueMS_h
#define oConcurrentQueueMS_h

#include <oooii/oConcurrentPooledAllocator.h>
#include <oooii/oTaggedPointer.h>

template<typename T, typename Alloc = std::allocator<char> >
class oConcurrentQueueMS
{
	/** <citation
		usage="Paper" 
		reason="The MS queue is often used as the benchmark for other concurrent queue algorithms, so here is an implementation to use to compare such claims." 
		author="Maged M. Michael and Michael L. Scott"
		description="http://www.cs.rochester.edu/research/synchronization/pseudocode/queues.html"
	/>*/

	struct NODE
	{
		NODE(const T& _Element) : Next(0, 0), Value(_Element) {}
		oTaggedPointer<NODE> Next;
		T Value;
	};

	typedef oTaggedPointer<NODE> PTR;

	PTR Head;
	PTR Tail;
	oConcurrentPooledAllocator<NODE, Alloc> Pool;

public:
	oConcurrentQueueMS(const char* _DebugName, oPooledAllocatorBase::PoolInitType _Type = oPooledAllocatorBase::InitElementCount, size_t _Value = 100000, const Alloc& _Allocator = Alloc())
		: Pool(_DebugName, _Type, _Value, _Allocator)
	{
		NODE* n = Pool.Construct(T(0));
		Head = Tail = PTR(n, 0);
	}

	~oConcurrentQueueMS()
	{
		oASSERT(IsEmpty(), "oConcurrentQueueMS %s not empty", GetDebugName());
		NODE* n = Head;
		Head = Tail = PTR(0, 0);
		Pool.Destroy(n);
	}

	bool TryPush(const T& _Element) threadsafe
	{
		NODE* n = Pool.Construct(_Element);
		if (!n) return false;
		PTR t, next;
		while (1)
		{
			t = Tail;
			next = t->Next;
			if (t == Tail)
			{
				if (!next)
				{
					if (PTR::CAS(&Tail->Next, PTR(n, next.GetTag()+1), next))
						break;
				}

				else
					PTR::CAS(&Tail, PTR(next, t.GetTag()+1), t);
			}
		}

		PTR::CAS(&Tail, PTR(n, t.GetTag()+1), t);
		return true;
	}

	bool TryPop(T& _Element) threadsafe
	{
		PTR h, t, next;
		while (1)
		{
			h = Head;
			t = Tail;
			next = h->Next;
			if (h == Head)
			{
				if (h.GetPointer() == t.GetPointer())
				{
					if (!next) return false;
					PTR::CAS(&Tail, PTR(next, t.GetTag()+1), t);
				}

				else
				{
					_Element = next->Value;
					if (PTR::CAS(&Head, PTR(next, h.GetTag()+1), h))
						break;
				}
			}
		}

		Pool.Destroy(h);
		return true;
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
