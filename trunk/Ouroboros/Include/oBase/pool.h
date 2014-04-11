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
// allocate in O(1) time from a preallocated array of blocks (fixed block
// allocator). This has permutations for concurrent and non-concurrent 
// execution as well as index-only for non-intrusive store of the freelist
// or intrusive store of the freelist to minimize overhead.

// (concurrent_)block_pool: void* alloc/dealloc of a templated size
// (concurrent_)index_pool: templated index type that only works with indices
// (concurrent_)object_pool: typed create/destroy in addition to void* alloc/dealloc

#ifndef oBase_pool_h
#define oBase_pool_h

#include <oBase/byte.h>
#include <oBase/callable.h>
#include <oBase/config.h>
#include <atomic>
#include <type_traits>

// the type must defined base_t and self_t and have no additional members (otherwise move is incomplete)
#define oPOOL_BOILERPLATE(_Type) \
	static size_type calc_size(size_type _Capacity) { return base_t::calc_size(_Capacity); } \
	_Type() {} \
	_Type(_Type&& _That) { operator=(std::move(_That)); } \
	_Type& operator=(_Type&& _That) { return (self_t&)base_t::operator=(std::move((base_t&&)_That)); } \
	_Type(void* _pPool, size_type _NumPooled) : base_t(_pPool, _NumPooled) {} \
	void* const get_memory_pointer() const { return base_t::get_memory_pointer(); } \
	void clear() { return base_t::clear(); } \
	bool valid() const { return base_t::valid(); } \
	bool valid(index_type _Index) const { return base_t::valid(_Index); } \
	index_type index(void* _Pointer) const { return base_t::index(_Pointer); } \
	void* pointer(index_type _Index) const { return base_t::pointer(_Index); } \
	size_type count_available() const { return base_t::count_available(); } \
	size_type size() const { return base_t::size(); } \
	size_type capacity() const { return base_t::capacity(); } \
	bool empty() const { return base_t::empty(); } \
	bool full() const { return base_t::full(); }

// Defines methods for calling ctors and dtors on allocations
#ifndef oHAS_VARIADIC_TEMPLATES
	#define oPOOL_CREATE_N(n) \
		template<oCALLABLE_CONCAT(oARG_TYPENAMES,n)> \
		T* create(oCALLABLE_CONCAT(oARG_DECL,n)) { void* p = allocate(); return p ? new (p) T(oCALLABLE_CONCAT(oARG_PASS,n)) : nullptr; }
	#define oPOOL_CREATE oCALLABLE_PROPAGATE_SKIP0(oPOOL_CREATE_N) \
		T* create() { void* p = allocate(); return p ? new (p) T() : nullptr; }
	#define oPOOL_DESTROY() \
		void destroy(T* _Pointer) { _Pointer->T::~T(); deallocate(_Pointer); }
#else
	#error Use variadic templates to implement construct()/destroy()
#endif

namespace ouro {
	
// base class that shares all the non-concurrent init and bookkeeping APIs
template<size_t BlockSize, unsigned int ConcurrencyTagMask>
class block_pool_base
{
	block_pool_base(const block_pool_base&); /* = delete */
	const block_pool_base& operator=(const block_pool_base&); /* = delete */

public:
	typedef unsigned int size_type;
	
	typedef typename std::conditional<ConcurrencyTagMask != 0, std::atomic<unsigned int>, 
		unsigned int>::type freelist_declaration_type;
	
	// shrink the index type if necessary for very small block sizes
	typedef typename std::conditional<BlockSize < sizeof(unsigned short), unsigned char, 
		typename std::conditional<BlockSize < sizeof(unsigned int), unsigned short, unsigned int>::type>::type index_type;
	
	typedef unsigned int freelist_type;
	static const freelist_type index_mask = ~ConcurrencyTagMask & index_type(-1);
	static const freelist_type invalid_index = index_mask;
	static const freelist_type max_index = invalid_index - 1;
	static const size_type index_size = sizeof(index_type);
	static const size_type block_size = size_type(BlockSize);

	// returns the number of bytes required to contain the blocks
	static size_type calc_size(size_type _Capacity)
	{
		return block_size * _Capacity;
	}

	block_pool_base() : Blocks(nullptr), NumBlocks(0) { Freelist = 0; }
	block_pool_base(block_pool_base&& _That) { operator=(std::move(_That)); }
	block_pool_base& operator=(block_pool_base&& _That)
	{
		if (this != &_That)
		{
			Blocks = _That.Blocks; _That.Blocks = nullptr;
			NumBlocks = _That.NumBlocks; _That.NumBlocks = 0;
			Freelist = (index_type)_That.Freelist; _That.Freelist = 0;
		}

		return *this;
	}

	block_pool_base(void* _pBlocks, size_type _NumBlocks)
		: Blocks(_pBlocks)
		, NumBlocks(_NumBlocks)
	{
		if (_NumBlocks > max_index)
			throw std::invalid_argument("too many blocks for the block size specified");
		if (!byte_aligned(Blocks, oDEFAULT_MEMORY_ALIGNMENT))
			throw std::invalid_argument("the specified allocation for the object pool must be properly aligned");
		clear();
	}

	// hard-resets the pool, leaving any outstanding allocation dangling. Not threadsafe.
	void clear()
	{
		Freelist = 0;
		index_type i = 0;
		for (index_type n = (index_type)NumBlocks - 1; i < n; i++)
			*at(i) = (i + 1);
		*at(i) = invalid_index;
	}

	// returns if this allocator has been initialized
	bool valid() const { return !!Blocks; }

	// returns true if the specified index was allocated from this allocator
	bool valid(index_type _Index) const { return _Index < NumBlocks; }

	// returns true if the specified pointer was allocated from this allocator
	bool valid(void* _Pointer) const { return in_range(_Pointer, Blocks, NumBlocks * block_size) && ((byte_diff(_Pointer, Blocks) % block_size) == 0); }

	// returns the index of a pointer allocated from this pool
	index_type index(void* _Pointer) const { return static_cast<index_type>(byte_diff(_Pointer, Blocks) / block_size); }

	// returns a pointer to the entry at the specified index
	void* pointer(index_type _Index) const { return static_cast<void*>(byte_add(Blocks, block_size, _Index)); }

	// SLOW! This walks the free list counting how many members there are. This is
	// designed for out-of-performance-critical code such as garbage collecting to
	// determine if all possible Blocks are available and thus there are no 
	// outstanding allocations.
	size_type count_available() const
	{
		size_type n = 0;
		index_type i = Freelist & index_mask;
		while (i != invalid_index)
		{
			n++;
			if (n > capacity())
				throw std::length_error("count of available Blocks has exceeded the maximum allowable");
			i = *at(i);
		}
		return n;
	}

	// number of Blocks allocated (SLOW! this loops through entire freelist 
	// each call)
	size_type size() const
	{
		size_type nFree = 0;
		if (NumBlocks)
			nFree = count_available();
		return NumBlocks - nFree;
	}

	// max number of Blocks to be allocated
	size_type capacity() const { return NumBlocks; }

	// (SLOW! see size())
	bool empty() const { return size() == 0; }

	// returns true if all Blocks have been allocated
	bool full() const { return (Freelist & index_mask) == invalid_index; }

	// retrieves the pointer used during construction so it can be 
	// be freed if no other reference is available.
	void* const get_memory_pointer() const { return Blocks; }

	// return a pointer to the nth entry
	index_type* at(size_type _Index) { return static_cast<index_type*>(byte_add(Blocks, block_size, _Index)); }
	const index_type* at(size_type _Index) const { return static_cast<index_type*>(byte_add(Blocks, block_size, _Index)); }

protected:
	static const freelist_type tag_mask = ConcurrencyTagMask;
	#pragma warning(disable:4307) // integral constant overflow
	static const freelist_type tag_one = 1 + ~ConcurrencyTagMask;
	#pragma warning(default:4307) // integral constant overflow

	void* Blocks;
	freelist_type NumBlocks;
	freelist_declaration_type Freelist;
};

// non-concurrent version of a fixed-sized block allocator
template<size_t BlockSize>
class block_pool : public block_pool_base<BlockSize, 0>
{
	typedef block_pool_base<BlockSize, 0> base_t;
	typedef block_pool<BlockSize> self_t;

public:
	typedef typename base_t::index_type index_type;
	typedef typename base_t::size_type size_type;
	static const size_type max_blocks = base_t::max_index;

	oPOOL_BOILERPLATE(block_pool)

	// allocate a pointer to a block or nullptr if no available room; also return the index of the new block or invalid_index
	void* allocate(index_type* _pIndex)
	{
		*_pIndex = Freelist & index_mask;
		if (Freelist == invalid_index)
			return nullptr;
		index_type* p = at(Freelist);
		Freelist = *p;
		return p;
	}

	index_type allocate_index()
	{
		index_type i;
		allocate(&i);
		return i;
	}

	// allocate a block or return nullptr if no available room
	void* allocate() { index_type i; return allocate(&i); }

	// mark a block as available by index
	void deallocate_index(index_type _Index)
	{
		if (!valid(_Index))
			throw std::out_of_range("the specified index was not allocated from this allocator");
		*at(_Index) = Freelist & index_mask;
		Freelist = _Index;
	}

	// mark a block as available by pointer
	void deallocate(void* _pBlock)
	{
		if (!valid(_pBlock))
			throw std::out_of_range("the specified pointer was not allocated from this allocator");
		*(index_type*)_pBlock = Freelist & index_mask;
		Freelist = (index_type)index_of(_pBlock, Blocks, block_size);
	}

	bool valid(void* _pObject) const { return base_t::valid(_pObject); }
};

// concurrent version of a fixed-sized block allocator
template<size_t BlockSize>
class concurrent_block_pool : public block_pool_base<BlockSize, 0xff000000>
{
	typedef block_pool_base<BlockSize, 0xff000000> base_t;
	typedef concurrent_block_pool<BlockSize> self_t;

public:
	typedef typename base_t::index_type index_type;
	typedef typename base_t::size_type size_type;
	static const size_type max_blocks = base_t::invalid_index - 1;

	oPOOL_BOILERPLATE(concurrent_block_pool)

	// allocate a pointer to a block or nullptr if no available room; also return the index of the new block or invalid_index
	void* allocate(index_type* _pIndex)
	{
		index_type* p = nullptr;
		index_type newI, oldI = Freelist;
		do
		{
			*_pIndex = oldI & index_mask;
			if (*_pIndex == invalid_index)
				return nullptr;
			p = at(*_pIndex);
			newI = *p | ((oldI + tag_one) & tag_mask);

		} while (!Freelist.compare_exchange_strong(oldI, newI));
		return p;
	}

	index_type allocate_index()
	{
		freelist_type newI, oldI = Freelist;
		index_type i;
		do
		{
			i = oldI & index_mask;
			if (i == invalid_index)
				return i;
			index_type* p = at(i);
			newI = *p | ((oldI + tag_one) & tag_mask);

		} while (!Freelist.compare_exchange_strong(oldI, newI));
		return i;
	}

	// allocate a block or return nullptr if no available room
	void* allocate() { index_type i; return allocate(&i); }

	// mark a block as available by index
	void deallocate_index(index_type _Index)
	{
		if (!valid(_Index))
			throw std::out_of_range("the specified index was not allocated from this allocator");
		deallocate(at(_Index));
	}

	// mark a block as available by pointer
	void deallocate(void* _pBlock)
	{
		if (!valid(_pBlock))
			throw std::out_of_range("the specified pointer was not allocated from this allocator");
		index_type i = (index_type)index_of(_pBlock, Blocks, block_size);
		freelist_type newI, oldI = Freelist;
		do
		{
			*(index_type*)_pBlock = oldI & index_mask;
			newI = i | ((oldI + tag_one) & tag_mask);
		} while (!Freelist.compare_exchange_strong(oldI, newI));
	}

	bool valid(void* _pObject) const { return base_t::valid(_pObject); }
};

// non-concurrent index allocator with no store other than the freelist
template<typename IndexT>
class index_pool : private block_pool<sizeof(IndexT)>
{
	typedef block_pool<sizeof(IndexT)> base_t;
	typedef index_pool<IndexT> self_t;
public:
	typedef IndexT index_type;
	typedef base_t::size_type size_type;
	static const index_type invalid_index = base_t::invalid_index;

	oPOOL_BOILERPLATE(index_pool)

	index_type allocate() { return base_t::allocate_index(); }
	void deallocate(index_type _Index) { base_t::deallocate_index(_Index); }
	void* const get_index_pointer() const { return base_t::get_memory_pointer(); }
};

// concurrent index allocator with no store other than the freelist
template<typename IndexT>
class concurrent_index_pool : private concurrent_block_pool<sizeof(IndexT)>
{
	typedef concurrent_block_pool<sizeof(IndexT)> base_t;
	typedef concurrent_index_pool<IndexT> self_t;
public:
	typedef IndexT index_type;
	typedef base_t::size_type size_type;
	static const index_type invalid_index = base_t::invalid_index;

	oPOOL_BOILERPLATE(concurrent_index_pool)

	index_type allocate() { return base_t::allocate_index(); }
	void deallocate(index_type _Index) { base_t::deallocate_index(_Index); }
	void* const get_index_pointer() const { return base_t::get_memory_pointer(); }
};

// non-concurrent object pool with calls to ctors and dtors
template<typename T>
class object_pool : private block_pool<sizeof(T)>
{
	typedef block_pool<sizeof(T)> base_t;
	typedef object_pool<T> self_t;

public:
	typedef typename base_t::size_type size_type;
	typedef typename base_t::index_type index_type;
	typedef T object_type;
	static const index_type invalid_index = base_t::invalid_index;
	static const size_type max_objects = base_t::max_blocks;

	oPOOL_BOILERPLATE(object_pool)

	oPOOL_CREATE();
	oPOOL_DESTROY();

	bool valid(object_type* _pObject) const { return base_t::valid(_pObject); }
};

// concurrent object pool with calls to ctors and dtors
template<typename T>
class concurrent_object_pool : private concurrent_block_pool<sizeof(T)>
{
	typedef concurrent_block_pool<sizeof(T)> base_t;
	typedef concurrent_object_pool<T> self_t;

public:
	typedef typename base_t::size_type size_type;
	typedef typename base_t::index_type index_type;
	typedef T object_type;
	static const index_type invalid_index = base_t::invalid_index;
	static const size_type max_objects = base_t::max_blocks;

	oPOOL_BOILERPLATE(concurrent_object_pool)

	oPOOL_CREATE();
	oPOOL_DESTROY();

	bool valid(object_type* _pObject) const { return base_t::valid(_pObject); }
};

} // namespace ouro

#endif
