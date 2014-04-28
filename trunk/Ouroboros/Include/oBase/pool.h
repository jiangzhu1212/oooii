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
// execution.

#ifndef oBase_pool_h
#define oBase_pool_h

#include <oBase/byte.h>
#include <oBase/callable.h>
#include <oBase/compiler_config.h>
#include <atomic>
#include <type_traits>

#define oCHECK_EMPTY() do { if (!empty()) throw std::exception("pool not empty"); } while(false)

// Defines methods for calling ctors and dtors on allocations
#ifndef oHAS_VARIADIC_TEMPLATES
	#define oPOOL_CREATE_N(n) \
		template<oCALLABLE_CONCAT(oARG_TYPENAMES,n)> \
		T* create(oCALLABLE_CONCAT(oARG_DECL,n)) { void* p = allocate_ptr(); return p ? new (p) T(oCALLABLE_CONCAT(oARG_PASS,n)) : nullptr; }
	#define oPOOL_CREATE oCALLABLE_PROPAGATE_SKIP0(oPOOL_CREATE_N) \
		T* create() { void* p = allocate_ptr(); return p ? new (p) T() : nullptr; }
	#define oPOOL_DESTROY() \
		void destroy(T* _Pointer) { _Pointer->T::~T(); deallocate(_Pointer); }
#else
	#error Use variadic templates to implement construct()/destroy()
#endif

namespace ouro {

template<typename T, bool Atomic>
class pool_base
{
public:
	typedef unsigned int size_type;

	static const unsigned int tag_mask = 0xff000000;
	static const unsigned int tag_one = 1 + ~tag_mask;
	static const unsigned int index_mask = ~tag_mask;

	// shrink the index type if necessary for very small block sizes
	typedef typename std::conditional<sizeof(T) < sizeof(unsigned short), unsigned char, 
		typename std::conditional<sizeof(T) < sizeof(unsigned int), unsigned short, unsigned int>::type>::type index_type;

	typedef typename std::conditional<Atomic, std::atomic_uint, unsigned int>::type freelist_type;

	// when allocate fails it returns this value
	static const index_type invalid_index = index_type(-1) & index_mask;
	static const index_type max_index = invalid_index - 1;
	static const size_type max_capacity_ = invalid_index;

	// return the size of the working memory required
	static size_type calc_size(size_type _Capacity) { return sizeof(T) * _Capacity; }

	pool_base()
		: Blocks(nullptr)
		, NumBlocks(0)
	{ Freelist = 0; }

	// _pMemory must be allocated to at least calc_size() sized
	pool_base(void* _pMemory, size_type _Capacity) { initialize(_pMemory, _Capacity); }
	~pool_base() { oCHECK_EMPTY(); }

	pool_base(pool_base&& _That) { operator=(std::move()); }
	pool_base& operator=(pool_base&& _That)
	{
		if (this != &_That)
		{
			if (valid())
				throw std::exception("move not supported into initialized pools");
			Blocks = _That.Blocks; _That.Blocks = nullptr;
			NumBlocks = _That.NumBlocks; _That.NumBlocks = 0;
			Freelist = (size_type)_That.Freelist; _That.Freelist = 0;
		}

		return *this;
	}

	void initialize(void* _pMemory, size_type _Capacity)
	{
		Blocks = (T*)_pMemory;
		NumBlocks = _Capacity;
		reset();
	}

	// returns the pointer passed to initialize() or the ctor
	void* deinitialize()
	{
		oCHECK_EMPTY();
		void* p = Blocks;
		Blocks = nullptr;
		NumBlocks = 0;
		Freelist = 0;
		return p;
	}

	// restores to the initial state while preserving memory usage
	void reset()
	{
		if (!valid())
			throw std::exception("initialize can only be called on an invalid pool");
		Freelist = 0;
		index_type i = 0;
		for (size_type n = NumBlocks - 1; i < n; i++)
			*(index_type*)at(i) = i + 1;
		*(index_type*)at(i) = invalid_index;
	}

	// returns if this allocator has been initialized
	bool valid() const { return !!Blocks; }

	// returns true if the specified index might be allocated from this allocator
	bool valid(index_type _Index) const { return (size_type)_Index < NumBlocks; }

	// returns true if the specified pointer might be allocated from this allocator
	bool valid(void* _Pointer) const { return in_range(_Pointer, Blocks, NumBlocks * sizeof(T)) && ((byte_diff(_Pointer, Blocks) % sizeof(T)) == 0); }

	// returns the index of a pointer allocated from this pool
	index_type index(void* _Pointer) const { return static_cast<index_type>(byte_diff(_Pointer, Blocks) / sizeof(T)); }

	// returns the pointer to the memory at the specified index. This does not guarantee the object is constructed.
	T* at(index_type _Index) { return _Index == invalid_index ? nullptr : static_cast<T*>(byte_add(Blocks, sizeof(T), _Index)); }
	const T* at(index_type _Index) const { return _Index == invalid_index ? nullptr : static_cast<T*>(byte_add(Blocks, sizeof(T), _Index)); }
	
	// SLOW! This walks the free list counting how many members there are
	size_type count_available() const
	{
		size_type n = 0;
		index_type i = index_type(Freelist & index_mask);
		while (i != invalid_index)
		{
			n++;
			if (n > capacity())
				throw std::length_error("count of available Blocks has exceeded the maximum allowable");
			i = *(index_type*)at(i);
		}
		return n;
	}

	// number of Blocks allocated (SLOW! this loops through entire freelist each call)
	size_type size() const
	{
		size_type nFree = 0;
		if (NumBlocks)
			nFree = count_available();
		return NumBlocks - nFree;
	}

	// returns the most elements that can be stored
	size_type max_capacity() const { return invalid_index; }

	// max number of Blocks to be allocated
	size_type capacity() const { return NumBlocks; }

	// (SLOW! see size())
	bool empty() const { return size() == 0; }

	// returns true if all Blocks have been allocated
	bool full() const { return index_type(Freelist & index_mask) == invalid_index; }

protected:
	T* Blocks;
	size_type NumBlocks;
	freelist_type Freelist;

	pool_base(pool_base&);
	const pool_base& operator=(const pool_base&);
};

template<typename T>
class pool : public pool_base<T, false>
{
	typedef pool_base<T, false> base_t;

public:
	typedef typename base_t::size_type size_type;
	typedef typename base_t::index_type index_type;
	typedef T value_type;

	static size_type calc_size(size_type _Capacity) { return base_t::calc_size(_Capacity); }
	
	pool() {}
	~pool() { oCHECK_EMPTY(); }
	pool(void* _pMemory, size_type _Capacity) : base_t(_pMemory, _Capacity) {}
	pool(pool&& _That) { operator=(std::move(_That)); }
	pool& operator=(pool&& _That) { return base_t::operator=(std::move((base_t&&)_That)); }

	// allocate an index from the freelist
	index_type allocate()
	{
		index_type i = index_type(Freelist & index_mask);
		if (i == invalid_index)
			return invalid_index;
		Freelist = *(freelist_type*)at(i);
		return i;
	}

	// return an index to the freelist
	void deallocate(index_type _Index)
	{
		if (!valid(_Index))
			throw std::out_of_range("the specified index was not allocated from this allocator");
		*(index_type*)at(_Index) = index_type(Freelist & index_mask);
		Freelist = (freelist_type)_Index;
	}

	void* allocate_ptr() { return at(allocate()); }
	void deallocate(void* _Pointer) { deallocate(index(_Pointer)); }
};

template<typename T>
class object_pool : public pool<T>
{
	typedef pool<T> base_t;

public:
	typedef typename base_t::size_type size_type;
	typedef typename base_t::index_type index_type;
	typedef T value_type;

	static size_type calc_size(size_type _Capacity) { return base_t::calc_size(_Capacity); }

	object_pool() {}
	~object_pool() { oCHECK_EMPTY(); }
	object_pool(void* _pMemory, size_type _Capacity) : base_t(_pMemory, _Capacity) {}
	object_pool(object_pool&& _That) { operator=(std::move(_That)); }
	object_pool& operator=(object_pool&& _That) { return (object_pool&)base_t::operator=(std::move((base_t&&)_That)); }

	oPOOL_CREATE();
	oPOOL_DESTROY();
};

} // namespace ouro

#endif
