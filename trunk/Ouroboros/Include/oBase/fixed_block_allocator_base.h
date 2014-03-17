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
// Base class for allocators that allocate in fixed-sized blocks. The intent
// is to shared boiler-plate code between concurrent and non-concurrent 
// implementations as well as support very-low-level usage and general-purpose
// usage with minimal algorithmic code duplication (though there's a bit of 
// boilerplate repetion as a result).

// There are 5 flavors implemented:
// 1. The veriest base class: this handles the shared/boiler-plate 
//    implementations shared between a single-threaded and concurrent version.
// 2. A base class that statically specifies the block size.
// 3. A base class that statically specifies the block size as sizeof(T).
// 4. A base class that statically retains a block pool for the specified type.
// 5. A base class that requires a dynamic heap, but encapsulates the block size 
//    and count parameters.

// Most of this API is to accomodate the usage of this allocator in a larger slab 
// allocator where this would be used for each slab, but there are enough slabs to 
// factor out block size and capacity as an optimization. The code constructs here
// should melt away in the compiler, but allow enough encapsulation or optimization
// for many use cases.

#ifndef oBase_fixed_block_allocator_common_base_h
#define oBase_fixed_block_allocator_common_base_h

#include <oBase/byte.h>
#include <oBase/callable.h>
#include <oBase/config.h>
#include <stdexcept>

// Defines methods for calling ctors and dtors on allocations
#ifndef oHAS_VARIADIC_TEMPLATES
	#define oALLOCATOR_CONSTRUCT_N(n) \
		template<oCALLABLE_CONCAT(oARG_TYPENAMES,n)> \
		T* construct(oCALLABLE_CONCAT(oARG_DECL,n)) { T* p = allocate(); return p ? new (p) T(oCALLABLE_CONCAT(oARG_PASS,n)) : nullptr; }
	#define oALLOCATOR_CONSTRUCT oCALLABLE_PROPAGATE_SKIP0(oALLOCATOR_CONSTRUCT_N) T* construct() { T* p = allocate(); return p ? new (p) T() : nullptr; }
	#define oALLOCATOR_DESTROY() void destroy(T* _Instance) {	_Instance->T::~T(); deallocate(_Instance); }
#else
	#error Use variadic templates to implement construct()/destroy()
#endif

namespace ouro {

template<typename IndexT, typename FreelistT, size_t IndexMask>
class fixed_block_allocator_common_base
{
	fixed_block_allocator_common_base(const fixed_block_allocator_common_base&); /* = delete */
	const fixed_block_allocator_common_base& operator=(const fixed_block_allocator_common_base&); /* = delete */

public:
	typedef IndexT index_type;
	typedef unsigned int size_type;
	static const size_type index_size = sizeof(index_type);
	static const index_type index_mask = IndexMask & index_type(-1);
	static const size_type default_alignment = __max(sizeof(void*), oDEFAULT_MEMORY_ALIGNMENT);
	static const size_type max_num_blocks = (size_type(-1) - 1) & index_mask; // reserve invalid value

	fixed_block_allocator_common_base() : Blocks(nullptr) { Freelist = 0; }
	fixed_block_allocator_common_base(fixed_block_allocator_common_base&& _That) { operator=(std::move(_That)); }
	fixed_block_allocator_common_base& operator=(fixed_block_allocator_common_base&& _That)
	{
		if (this != &_That)
		{
			Freelist = _That.Freelist; _That.Freelist = 0;
			Blocks = _That.Blocks; _That.Blocks = nullptr;
		}

		return *this;
	}
	fixed_block_allocator_common_base(void* _pBlocks, size_type _BlockSize, size_type _NumBlocks)
		: Blocks(_pBlocks)
	{
		// check validity
		if (_BlockSize < sizeof(index_type))
			throw std::invalid_argument("block size must be at least the size of the index type");
		if (_NumBlocks >= invalid_index)
			throw std::invalid_argument("too many blocks specified for the index type used");
		if (!byte_aligned(_pBlocks, default_alignment))
			throw std::invalid_argument("the specified allocation of blocks must be properly aligned");

		clear(_BlockSize, _NumBlocks);
	}

	// Hard-resets the pool, leaving any outstanding allocation dangling. Not threadsafe.
	void clear(size_type _BlockSize, size_type _NumBlocks)
	{
		Freelist = 0;
		index_type i = 0;
		index_type* p = static_cast<index_type*>(Blocks);
		for (; i < (_NumBlocks-1); i++)
			*at(_BlockSize, i) = i+1;
		*at(_BlockSize, i) = invalid_index;
	}

	// Returns if this allocator has been initialized
	bool valid() const { return !!Blocks; }

	// Returns true if the specified pointer was allocated from this allocator.
	bool valid(size_type _BlockSize, size_type _NumBlocks, void* _Pointer) const
	{
		return in_range(_Pointer, Blocks, _NumBlocks * _BlockSize) && ((byte_diff(_Pointer, Blocks) % _BlockSize) == 0);
	}

	// SLOW! This walks the free list counting how many members there are. This is
	// designed for out-of-performance-critical code such as garbage collecting to
	// determine if all possible blocks are available and thus there are no 
	// outstanding allocations.
	size_type count_available(size_type _BlockSize) const
	{
		size_type n = 0;
		index_type i = Freelist & index_mask;
		while (i != invalid_index)
		{
			n++;
			if (n > max_num_blocks)
				throw std::length_error("count of available blocks has exceeded the maximum allowable");

			i = *at(_BlockSize, i);
		}
		return n;
	}

	// number of indices allocated (SLOW! this loops through entire freelist 
	// each call)
	size_type size(size_type _BlockSize, size_type _NumBlocks) const
	{
		size_type nFree = 0;
		if (_NumBlocks)
			nFree = count_available(_BlockSize);
		return _NumBlocks - nFree;
	}

	size_type max_capacity() const { return max_num_blocks; }

	// (SLOW! see size())
	bool empty(size_type _BlockSize, size_type _NumBlocks) const { return size(_BlockSize, _NumBlocks) == 0; }

	// Returns true if all blocks have been allocated
	bool full() const { return Freelist.Index == invalid_index; }

	// This can be used to get the pointer passed to the constructor so it can
	// be freed if client code did not keep any other reference.
	void* const get_blocks_pointer() const { return Blocks; }

protected:
	static const index_type invalid_index = index_mask;

	inline index_type* at(size_type _BlockSize, size_type _Index) { return static_cast<index_type*>(byte_add(Blocks, _BlockSize, _Index)); }
	inline const index_type* at(size_type _BlockSize, size_type _Index) const { return static_cast<index_type*>(byte_add(Blocks, _BlockSize, _Index)); }

	void* Blocks;
	FreelistT Freelist;
};

// Partial specializations: these are intended to get a middle-derivation from fixed_block_allocator_common_base 
// (either fixed_block_allocator or concurrent_fixed_block_allocator).

// Partially-specializes by binding the block size. The number of blocks is still required for most methods.
template<typename fixed_block_allocator_baseT, size_t S>
class fixed_block_allocator_common_base_s : protected fixed_block_allocator_baseT
{
	typedef fixed_block_allocator_baseT base_t;
	typedef fixed_block_allocator_common_base_s<typename base_t::index_type, S> self_t;
public:
	typedef typename base_t::size_type size_type;
	typedef typename base_t::index_type index_type;
	static const size_type index_size = base_t::index_size;
	static const size_type max_num_blocks = base_t::max_num_blocks;
	static const size_type block_size = S;
	fixed_block_allocator_common_base_s() {}
	fixed_block_allocator_common_base_s(fixed_block_allocator_common_base_s&& _That) { operator=(std::move(_That)); }
	fixed_block_allocator_common_base_s(void* _pBlocks, size_type _NumBlocks) : base_t(_pBlocks, block_size, _NumBlocks) {}
	fixed_block_allocator_common_base_s& operator=(fixed_block_allocator_common_base_s&& _That) { return (self_t&)base_t::operator=(std::move((base_t&&)_That)); }
	void clear(size_type _NumBlocks) { base_t::clear(block_size, _NumBlocks); }
	bool valid() const { return base_t::valid(); }
	bool valid(size_type _NumBlocks, void* _Pointer) const { return base_t::valid(block_size, _NumBlocks, _Pointer); }
	void* allocate() { return base_t::allocate(block_size); }
	void deallocate(size_type _NumBlocks, void* _Pointer) { return base_t::deallocate(block_size, _NumBlocks, _Pointer); }
	size_type count_available() const { return base_t::count_available(block_size); }
	size_type size(size_type _NumBlocks) const { return base_t::size(block_size, _NumBlocks); }
	size_type max_capacity() const { return base_t::max_capacity(); }
	bool empty(size_type _NumBlocks) const { return base_t::empty(block_size, _NumBlocks); }
	bool full() const { return base_t::full(); }
};

// Partially-specializes by binding the block size as sizeof(T) and also provides construct/destroy since the 
// type is known. The number of blocks is still required for most methods.
template<typename fixed_block_allocator_baseT, typename T>
class fixed_block_allocator_common_base_t : protected fixed_block_allocator_common_base_s<fixed_block_allocator_baseT, sizeof(T)>
{
	typedef fixed_block_allocator_common_base_s<fixed_block_allocator_baseT, sizeof(T)> base_t;
	typedef fixed_block_allocator_common_base_t<fixed_block_allocator_baseT, T> self_t;
public:
	typedef typename base_t::size_type size_type;
	typedef typename base_t::index_type index_type;
	static const size_type index_size = typename base_t::index_size;
	static const size_type max_num_blocks = typename base_t::max_num_blocks;
	static const size_type block_size = typename base_t::block_size;
	typedef T value_type;
	fixed_block_allocator_common_base_t() {}
	fixed_block_allocator_common_base_t(fixed_block_allocator_common_base_t&& _That) { operator=(std::move(_That)); }
	fixed_block_allocator_common_base_t(void* _pBlocks, size_type _NumBlocks) : base_t(_pBlocks, _NumBlocks) {}
	fixed_block_allocator_common_base_t& operator=(fixed_block_allocator_common_base_t&& _That) { return (self_t&)base_t::operator=(std::move((base_t&&)_That)); }
	void clear(size_type _NumBlocks) { base_t::clear(block_size, _NumBlocks); }
	bool valid() const { return base_t::valid(); }
	bool valid(size_type _NumBlocks, void* _Pointer) const { return base_t::valid(block_size, _NumBlocks, _Pointer); }
	T* allocate() { return static_cast<T*>(base_t::allocate()); }
	void deallocate(size_type _NumBlocks, T* _Pointer) { base_t::deallocate(_NumBlocks, _Pointer); }
	size_type count_available() const { return base_t::count_available(); }
	size_type size(size_type _NumBlocks) const { return base_t::size(block_size, _NumBlocks); }
	size_type max_capacity() const { return base_t::max_capacity(); }
	bool empty(size_type _NumBlocks) const { return base_t::empty(block_size, _NumBlocks); }
	bool full() const { return base_t::full(); }
	oALLOCATOR_CONSTRUCT();
	void destroy(size_type _NumBlocks, T* _Instance)
	{
		_Instance->T::~T();
		deallocate(_NumBlocks, _Instance);
	}
};

// Uses statically allocated memory
template<typename fixed_block_allocator_baseT, typename T, size_t Capacity>
class fixed_block_allocator_static_base : protected fixed_block_allocator_common_base_t<fixed_block_allocator_baseT, T>
{
	typedef fixed_block_allocator_common_base_t<fixed_block_allocator_baseT, T> base_t;
	typedef fixed_block_allocator_static_base<fixed_block_allocator_baseT, T, Capacity> self_t;
public:
	typedef typename base_t::index_type index_type;
	static const size_type index_size = typename base_t::index_size;
	static const size_type max_num_blocks = typename base_t::max_num_blocks;
	static const size_type block_size = typename base_t::block_size;
	typedef typename base_t::value_type value_type;
	static const size_type capacity = Capacity;
	fixed_block_allocator_static_base() : base_t(Backing, Capacity) {}
	fixed_block_allocator_static_base(fixed_block_allocator_static_base&& _That) { operator=(std::move(_That)); }
	fixed_block_allocator_static_base& operator=(fixed_block_allocator_static_base&& _That) { return (self_t&)base_t::operator=(std::move((base_t&&)_That)); }
	void clear() { base_t::clear(block_size, Capacity); }
	bool valid() const { return base_t::valid(); }
	bool valid(void* _Pointer) const { return base_t::valid(block_size, capacity, _Pointer); }
	T* allocate() { return base_t::allocate(); }
	void deallocate(T* _Pointer) { base_t::deallocate(capacity, _Pointer); }
	void clear(size_type _NumBlocks) { base_t::clear(block_size, _NumBlocks); }
	size_type count_available() const { return base_t::count_available(); }
	size_type size() const { return base_t::size(block_size, capacity); }
	size_type max_capacity() const { return base_t::max_capacity(); }
	bool empty() const { return base_t::empty(block_size, capacity); }
	bool full() const { return base_t::full(); }
	oALLOCATOR_CONSTRUCT();
	oALLOCATOR_DESTROY();
protected:
	char Backing[block_size * capacity];
};

// Encapsulates passing around block size and num blocks.
template<typename fixed_block_allocator_baseT>
class fixed_block_allocator_encapsulated_base : protected fixed_block_allocator_baseT
{
	typedef fixed_block_allocator_baseT base_t;
	typedef fixed_block_allocator_encapsulated_base<fixed_block_allocator_baseT> self_t;
public:
	typedef typename base_t::index_type index_type;
	typedef typename base_t::size_type size_type;
	static const size_type index_size = typename base_t::index_size;

	fixed_block_allocator_encapsulated_base() : base_t(), BlockSize(0), NumBlocks(0) {}
	fixed_block_allocator_encapsulated_base(fixed_block_allocator_encapsulated_base&& _That) { operator=(std::move(_That)); }

	fixed_block_allocator_encapsulated_base(void* _pBlocks, size_type _BlockSize, size_type _NumBlocks)
		: base_t(_pBlocks, _BlockSize, _NumBlocks)
		, BlockSize(_BlockSize)
		, NumBlocks(_NumBlocks)
	{}

	fixed_block_allocator_encapsulated_base& operator=(fixed_block_allocator_encapsulated_base&& _That)
	{
		if (this != &_That)
		{
			base_t::operator=(std::move((base_t&&)_That));
			BlockSize = _That.BlockSize; _That.BlockSize = 0;
			NumBlocks = _That.NumBlocks; _That.NumBlocks = 0;
		}
		return *this;
	}

	size_type max_num_blocks() const { return NumBlocks; }
	size_type block_size() const { return BlockSize; }
	void clear() { base_t::clear(BlockSize, NumBlocks); }
	bool valid() const { return base_t::valid(); }
	bool valid(void* _Pointer) const { return base_t::valid(BlockSize, NumBlocks, _Pointer); }
	void* allocate() { return base_t::allocate(BlockSize); }
	void deallocate(void* _Pointer) { return base_t::deallocate(BlockSize, NumBlocks, _Pointer); }
	size_type count_available() const { return base_t::count_available(BlockSize); }
	size_type size() const { return base_t::size(BlockSize, NumBlocks); }
	size_type max_capacity() const { return base_t::max_capacity(); }
	bool empty() const { return base_t::empty(BlockSize, NumBlocks); }
	bool full() const { return base_t::full(); }

private:
	size_type BlockSize;
	size_type NumBlocks;
};

} // namespace ouro

#endif
