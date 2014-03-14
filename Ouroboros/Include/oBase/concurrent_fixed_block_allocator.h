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
// Allocates in fixed-sized blocks. To reduce overhead 16-bit indices are 
// used limiting allocation count to 65335 entries. This allocator has O(1) 
// allocation and deallocation time and uses a single CAS to protect concurrency.

#ifndef oBase_concurrent_fixed_block_allocator_h
#define oBase_concurrent_fixed_block_allocator_h

#include <oBase/byte.h>
#include <oBase/config.h>
#include <atomic>
#include <stdexcept>

namespace ouro {

template<typename IndexT>
class concurrent_fixed_block_allocator_base
{
	static_assert( std::is_unsigned<IndexT>::value && sizeof(IndexT) <= 4, "IndexT must be unsigned char, unsigned short or unsigned int");

public:
	typedef IndexT index_type;
	typedef unsigned int size_type;
	static const size_type index_size = sizeof(index_type);
	static const size_type max_num_blocks = size_type(-1) - 1; // reserve invalid value

	// tagged pointers use the bottom 3 bits in 32-bit, so leave room for that.
	static const size_type default_alignment = __max(8, oDEFAULT_MEMORY_ALIGNMENT);

	concurrent_fixed_block_allocator_base() : Blocks(nullptr) { NextAvailable = 0; }
	concurrent_fixed_block_allocator_base(concurrent_fixed_block_allocator_base&& _That) { operator=(std::move(_That)); }
	concurrent_fixed_block_allocator_base& operator=(concurrent_fixed_block_allocator_base&& _That)
	{
		if (this != &_That)
		{
			NextAvailable = _That.NextAvailable; _That.NextAvailable = 0;
			Blocks = _That.Blocks; _That.Blocks = nullptr;
		}

		return *this;
	}

	// Manage the specified memory as a fixed block pool allocated to 
	// _BlockSize * _NumBlocks bytes. This does not manage the lifetime of
	// _pBlocks.
	concurrent_fixed_block_allocator_base(void* _pBlocks, size_type _BlockSize, size_type _NumBlocks);

	// Returns if this allocator has been initialized
	bool valid() const { return !!Blocks; }

	// Allocate a block or return nullptr if no available room
	void* allocate(size_type _BlockSize);

	// Mark a block as available
	void deallocate(size_type _BlockSize, size_type _NumBlocks, void* _Pointer);

	// Returns true if the specified pointer was allocated from this allocator.
	bool valid(size_type _BlockSize, size_type _NumBlocks, void* _Pointer) const;

	// SLOW! This walks the free list counting how many members there are. This is
	// designed for out-of-performance-critical code such as garbage collecting to
	// determine if all possible blocks are available and thus there are no 
	// outstanding allocations.
	size_type count_available(size_type _BlockSize) const;

	// number of indices allocated (SLOW! this loops through entire freelist 
	// each call)
	size_type size(size_type _BlockSize, size_type _NumBlocks) const
	{
		size_t nFree = 0;
		if (_NumBlocks)
			nFree = count_available(_BlockSize);
		return _NumBlocks - nFree;
	}

	// (SLOW! see size())
	bool empty(size_type _BlockSize, size_type _NumBlocks) const { return size() == 0; }

	// Returns true if all blocks have been allocated
	bool full() const { return NextAvailable.Index == invalid_index; }

	// This can be used to get the pointer passed to the constructor so it can
	// be freed if client code did not keep any other reference.
	void* const get_arena() const { return pArena; }

private:
	inline index_type* at(size_type _BlockSize, size_type _Index) { return static_cast<index_type*>(byte_add(Blocks, _BlockSize, _Index)); }
	inline const index_type* at(size_type _BlockSize, size_type _Index) const { return static_cast<index_type*>(byte_add(Blocks, _BlockSize, _Index)); }

	concurrent_fixed_block_allocator_base(const concurrent_fixed_block_allocator_base&); /* = delete */
	const concurrent_fixed_block_allocator_base& operator=(const concurrent_fixed_block_allocator_base&); /* = delete */

	union tagged_index_t
	{
		unsigned int all;
		struct { unsigned int tag : 8; unsigned int index : 24; };
	};

	static const index_type index_mask = sizeof(index_type) > 3 ? ((1 << 24) - 1) : index_type(-1);
	static const index_type invalid_index = index_mask;

	oCACHE_ALIGNED(std::atomic_uint NextAvailable);
	void* Blocks;
};

template<typename IndexT>
concurrent_fixed_block_allocator_base<IndexT>::concurrent_fixed_block_allocator_base(void* _pBlocks, size_type _BlockSize, size_type _NumBlocks)
	: Blocks(_pBlocks)
{
	NextAvailable = 0;

	// check validity
	if (_BlockSize < sizeof(index_type))
		throw std::invalid_argument("block size must be at least the size of the index type");

	if (_NumBlocks >= invalid_index)
		throw std::invalid_argument("too many blocks specified for the index type used");

	if (!byte_aligned(_pBlocks, default_alignment))
		throw std::invalid_argument("the specified allocation of blocks must be properly aligned");

	// initialize
	index_type i = 0;
	index_type* p = static_cast<index_type*>(Blocks);
	for (; i < (_NumBlocks-1); i++)
		*at(_BlockSize, i) = i+1;
	*at(_BlockSize, i) = invalid_index;
}

template<typename IndexT>
void* concurrent_fixed_block_allocator_base<IndexT>::allocate(size_type _BlockSize)
{
	index_type* pFreeIndex = nullptr;
	tagged_index_t New, Old;
	Old.all = NextAvailable;
	do
	{
		if (Old.index == invalid_index)
			return nullptr;
		pFreeIndex = at(_BlockSize, Old.index);
		New.index = *pFreeIndex;
		New.tag = Old.tag + 1;
	} while (!NextAvailable.compare_exchange_strong(Old.all, New.all));

	return reinterpret_cast<void*>(pFreeIndex);
}

template<typename IndexT>
void concurrent_fixed_block_allocator_base<IndexT>::deallocate(size_type _BlockSize, size_type _NumBlocks, void* _Pointer)
{
	if (!valid(_BlockSize, _NumBlocks, _Pointer))
		throw std::out_of_range("the specified pointer was not allocated from this allocator");
	tagged_index_t New, Old;
	Old.all = NextAvailable;
	do
	{
		*reinterpret_cast<index_type*>(_Pointer) = static_cast<index_type>(Old.index & index_mask);
		New.index = index_of(_Pointer, Blocks, _BlockSize);
		New.tag = Old.tag + 1;
	} while (!NextAvailable.compare_exchange_strong(Old.all, New.all));
}

template<typename IndexT>
bool concurrent_fixed_block_allocator_base<IndexT>::valid(size_type _BlockSize, size_type _NumBlocks, void* _Pointer) const
{
	ptrdiff_t diff = byte_diff(_Pointer, Blocks);
	return in_range(_Pointer, Blocks, _NumBlocks * _BlockSize) && ((diff % _BlockSize) == 0);
}

template<typename IndexT>
typename concurrent_fixed_block_allocator_base<IndexT>::size_type concurrent_fixed_block_allocator_base<IndexT>::count_available(size_type _BlockSize) const
{
	size_type n = 0;
	tagged_index_t Next;
	Next.all = NextAvailable;
	index_type i = static_cast<index_type>(Next.index);
	while (i != invalid_index)
	{
		n++;
		if (n > max_num_blocks)
			throw std::length_error("count of available blocks has exceeded the maximum allowable");

		i = *at(_BlockSize, i);
	}
	return n;
}

// Partially-specializes concurrent_fixed_block_allocator by binding the block size. The 
// number of blocks is still required for most methods.
template<typename IndexT, size_t S>
class concurrent_fixed_block_allocator_s : protected concurrent_fixed_block_allocator_base<IndexT>
{
	typedef concurrent_fixed_block_allocator_base<IndexT> fba_t;
	typedef concurrent_fixed_block_allocator_s<IndexT, S> self_t;
	concurrent_fixed_block_allocator_s(const concurrent_fixed_block_allocator_s&); /* = delete */
	const concurrent_fixed_block_allocator_s& operator=(const concurrent_fixed_block_allocator_s&); /* = delete */
public:
	typedef typename fba_t::index_type index_type;
	static const size_type index_size = fba_t::index_size;
	static const size_type max_num_blocks = fba_t::max_num_blocks;
	static const size_type block_size = S;
	concurrent_fixed_block_allocator_s() {}
	concurrent_fixed_block_allocator_s(concurrent_fixed_block_allocator_s&& _That) { operator=(std::move(_That)); }
	concurrent_fixed_block_allocator_s(void* _pBlocks, size_type _NumBlocks) : fba_t(_pBlocks, block_size, _NumBlocks) {}
	concurrent_fixed_block_allocator_s& operator=(concurrent_fixed_block_allocator_s&& _That) { return (self_t&)fba_t::operator=(std::move((fba_t&&)_That)); }
	bool valid() const { return fba_t::valid(); }
	void* allocate() { return fba_t::allocate(block_size); }
	void deallocate(size_type _NumBlocks, void* _Pointer) { return fba_t::deallocate(block_size, _NumBlocks, _Pointer); }
	bool valid(size_type _NumBlocks, void* _Pointer) const { return fba_t::valid(block_size, _NumBlocks, _Pointer); }
	size_type count_available(size_type _BlockSize) const { return fba_t::count_available(_BlockSize); }
	size_type size(size_type _NumBlocks) const { return fba_t::size(block_size, _NumBlocks); }
	bool empty(size_type _NumBlocks) const { return fba_t::empty(block_size, _NumBlocks); }
	bool full() const { return fba_t::full(); }
};

// Partially-specializes concurrent_fixed_block_allocator by binding the block size as 
// sizeof(T) but also provides construct/destroy since the type is known. The
// number of blocks is still required for most methods.
template<typename IndexT, typename T>
class concurrent_fixed_block_allocator_t : protected concurrent_fixed_block_allocator_s<IndexT, sizeof(T)>
{
	typedef concurrent_fixed_block_allocator_s<IndexT, sizeof(T)> fba_t;
	typedef concurrent_fixed_block_allocator_t<IndexT, T> self_t;
	concurrent_fixed_block_allocator_t(const concurrent_fixed_block_allocator_t&); /* = delete */
	const concurrent_fixed_block_allocator_t& operator=(const concurrent_fixed_block_allocator_t&); /* = delete */
public:
	typedef typename fba_t::index_type index_type;
	static const size_type index_size = fba_t::index_size;
	static const size_type max_num_blocks = fba_t::max_num_blocks;
	static const size_type block_size = fba_t::block_size;
	typedef T value_type;
	concurrent_fixed_block_allocator_t() {}
	concurrent_fixed_block_allocator_t(concurrent_fixed_block_allocator_t&& _That) { operator=(std::move(_That)); }
	concurrent_fixed_block_allocator_t(void* _pBlocks, size_type _NumBlocks) : fba_t(_pBlocks, _NumBlocks) {}
	concurrent_fixed_block_allocator_t& operator=(concurrent_fixed_block_allocator_t&& _That) { return (self_t&)fba_t::operator=(std::move((fba_t&&)_That)); }
	bool valid() const { return fba_t::valid(); }
	T* allocate() { return static_cast<T*>(fba_t::allocate()); }
	void deallocate(size_type _NumBlocks, T* _Pointer) { fba_t::deallocate(_NumBlocks, _Pointer); }
	bool valid(size_type _NumBlocks, void* _Pointer) const { return fba_t::valid(block_size, _NumBlocks, _Pointer); }
	size_type count_available(size_type _BlockSize) const { return fba_t::count_available(_BlockSize); }
	size_type size(size_type _NumBlocks) const { return fba_t::size(block_size, _NumBlocks); }
	bool empty(size_type _NumBlocks) const { return fba_t::empty(block_size, _NumBlocks); }
	bool full() const { return fba_t::full(); }
	T* create() { T* p = self_t::allocate(); new (p) T(); return p; }
	T* create(const T& _That) { T* p = self_t::allocate(); new (p) T(_That); return p; }
	T* create(T&& _That) { T* p = self_t::allocate(); new (p) T(std::move(_That)); return p; }
	template<typename U> T* create(const U& _U) { T* p = self_t::allocate(); new (p) T(_U); return p; }
	void destroy(size_type _NumBlocks, T* _Instance) { _Instance->T::~T(); self_t::deallocate(_NumBlocks, _Instance); }
};

// Partially-specialized version of concurrent_fixed_block_allocator using statically 
// allocated memory.
template<typename IndexT, typename T, size_t Capacity>
class concurrent_fixed_block_allocator_static : protected concurrent_fixed_block_allocator_t<IndexT, T>
{
	typedef concurrent_fixed_block_allocator_t<IndexT, T> fba_t;
	typedef concurrent_fixed_block_allocator_static<IndexT, T, Capacity> self_t;
	concurrent_fixed_block_allocator_static(const concurrent_fixed_block_allocator_static&); /* = delete */
	const concurrent_fixed_block_allocator_static& operator=(const concurrent_fixed_block_allocator_static&); /* = delete */
public:
	typedef typename fba_t::index_type index_type;
	static const size_type index_size = fba_t::index_size;
	static const size_type max_num_blocks = fba_t::max_num_blocks;
	static const size_type block_size = fba_t::block_size;
	typedef T value_type;
	static const size_type capacity = Capacity;
	concurrent_fixed_block_allocator_static() : fba_t(Backing, Capacity) {}
	concurrent_fixed_block_allocator_static(concurrent_fixed_block_allocator_static&& _That) { operator=(std::move(_That)); }
	concurrent_fixed_block_allocator_static& operator=(concurrent_fixed_block_allocator_static&& _That) { return (self_t&)fba_t::operator=(std::move((fba_t&&)_That)); }
	bool valid() const { return fba_t::valid(); }
	T* allocate() { return fba_t::allocate(); }
	void deallocate(T* _Pointer) { fba_t::deallocate(capacity, _Pointer); }
	bool valid(void* _Pointer) const { return fba_t::valid(block_size, capacity, _Pointer); }
	size_type count_available() const { return fba_t::count_available(block_size); }
	size_type size() const { return fba_t::size(block_size, capacity); }
	bool empty() const { return fba_t::empty(block_size, capacity); }
	bool full() const { return fba_t::full(); }
	T* create() { T* p = self_t::allocate(); new (p) T(); return p; }
	T* create(const T& _That) { T* p = self_t::allocate(); new (p) T(_That); return p; }
	T* create(T&& _That) { T* p = self_t::allocate(); new (p) T(std::move(_That)); return p; }
	template<typename U> T* create(const U& _U) { T* p = self_t::allocate(); new (p) T(_U); return p; }
	void destroy(T* _Instance) { _Instance->T::~T(); self_t::deallocate(_Instance); }
private:
	char Backing[block_size * capacity];
};

// Encapsulates passing around block size and num blocks.
template<typename IndexT>
class concurrent_fixed_block_allocator : protected concurrent_fixed_block_allocator_base<IndexT>
{
	typedef typename concurrent_fixed_block_allocator_base<IndexT> fba_t;
	concurrent_fixed_block_allocator(const concurrent_fixed_block_allocator&); /* = delete */
	const concurrent_fixed_block_allocator& operator=(const concurrent_fixed_block_allocator&); /* = delete */
public:
	typedef typename fba_t::index_type index_type;
	static const size_type index_size = fba_t::index_size;

	concurrent_fixed_block_allocator() : fba_t(), BlockSize(0), NumBlocks(0) {}
	concurrent_fixed_block_allocator(concurrent_fixed_block_allocator&& _That) { operator=(std::move(_That)); }

	concurrent_fixed_block_allocator(void* _pBlocks, size_type _BlockSize, size_type _NumBlocks)
		: concurrent_fixed_block_allocator_base(_pBlocks, _BlockSize, _NumBlocks)
		, BlockSize(_BlockSize)
		, NumBlocks(_NumBlocks)
	{}

	concurrent_fixed_block_allocator& operator=(concurrent_fixed_block_allocator&& _That)
	{
		fba_t::operator=(std::move((fba_t&&)_That));
		BlockSize = _That.BlockSize; _That.BlockSize = 0;
		NumBlocks = _That.NumBlocks; _That.NumBlocks = 0;
	}

	size_type max_num_blocks() const { return NumBlocks; }
	size_type block_size() const { return BlockSize; }
	bool valid() const { return fba_t::valid(); }
	void* allocate() { return fba_t::allocate(BlockSize); }
	void deallocate(void* _Pointer) { return fba_t::deallocate(BlockSize, NumBlocks, _Pointer); }
	bool valid(void* _Pointer) const { return fba_t::valid(BlockSize, NumBlocks, _Pointer); }
	size_type count_available() const { return fba_t::count_available(BlockSize); }
	size_type size() const { return fba_t::size(BlockSize, NumBlocks); }
	bool empty() const { return fba_t::empty(BlockSize, NumBlocks); }
	bool full() const { return fba_t::full(); }

private:
	size_type BlockSize;
	size_type NumBlocks;
};

} // namespace ouro

#endif
