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
// A very simple fixed-size allocator that operates on memory allocated by
// someone else. This uses the oversized-allocation pattern where the memory
// operated on is some large buffer that extends beyond the size of the class
// itself. This allows direct mapping onto memory this class does not own and
// also avoids an indirect of having a pointer in the class itself. Since this
// object is designed to be a very low-level object, the added complexity is
// acceptable for the overhead savings.

// This allocator has O(1) allocation and deallocation time and uses a single
// CAS in each case to enable concurrency.

// There is a generic/void* version and also a templated version below it.
// Because this is used in a grow-by-chunk allocator (block_allocator) redundant 
// data such as block size and number of blocks is not cached by this class.
#ifndef oConcurrency_fixed_block_allocator_h
#define oConcurrency_fixed_block_allocator_h

#include <oStd/callable.h>
#include <oStd/config.h>
#include <oConcurrency/thread_safe.h>

// Defines methods for calling ctors and dtors on allocations
#ifndef oHAS_VARIADIC_TEMPLATES
	#define oALLOCATOR_CONSTRUCT_N(n) \
		template<oCALLABLE_CONCAT(oARG_TYPENAMES,n)> \
		T* construct(oCALLABLE_CONCAT(oARG_DECL,n)) threadsafe \
		{ T* p = allocate(); return p \
			? new (p) T(oCALLABLE_CONCAT(oARG_PASS,n)) \
			: nullptr; \
		}
	#define oALLOCATOR_CONSTRUCT oCALLABLE_PROPAGATE_SKIP0(oALLOCATOR_CONSTRUCT_N) \
	T* construct() threadsafe \
	{ T* p = allocate(); return p \
		? new (p) T() \
		: nullptr; \
	}
	#define oALLOCATOR_DESTROY() \
		void destroy(T* _Instance) threadsafe \
		{	_Instance->T::~T(); \
			deallocate(_Instance); \
		}
#else
	#error Use variadic templates to implement construct()/destroy()
#endif

namespace oConcurrency {

class fixed_block_allocator
{
public:
	// unsigned short is used for the freelist index, so only 16-bits worth of
	// blocks can be indexed. This is to enable very small allocation blocks, such
	// as 16-bit indices. If more blocks are required, consider using 
	// block_allocator. Note: 0xffff is a reserved value.
	static const size_t max_num_blocks = 65534;

	// tagged pointers use the bottom 3 bits in 32-bit, so leave room for that.
	static const size_t default_alignment = __max(8, oDEFAULT_MEMORY_ALIGNMENT);

	// Returns the number of bytes required to hold a valid fixed-size block 
	// allocator. Use this to allocate a buffer, then type-cast that buffer to
	// a fixed_block_allocator*, then call initialize. For example:
	//
	// fixed_block_allocator* pAlloc = (fixed_block_allocator*)malloc(
	//   fixed_block_allocator::calc_required_size(sizeof(MyObj, 32));
	// pAlloc->initialize(sizeof(MyObj), 32);
	static size_t calc_required_size(size_t _BlockSize, size_t _NumBlocks);

	// Initialize this fixed-size block allocator to be ready to allocate. This 
	// assumes sizeof(*this) == calc_required_size(_BlockSize, _NumBlocks) and 
	// will write to that memory (well beyond sizeof(*this)).
	fixed_block_allocator(size_t _BlockSize, size_t _NumBlocks);

	// Allocate a block. This will return nullptr if there is no available room.
	void* allocate(size_t _BlockSize) threadsafe;

	// Mark a block as available
	void deallocate(size_t _BlockSize, size_t _NumBlocks, void* _Pointer) threadsafe;

	// Returns true if the specified pointer was allocated from this allocator.
	bool valid(size_t _BlockSize, size_t _NumBlocks, void* _Pointer) const threadsafe;

	// Slow! This walks the free list counting how many members there are. This is
	// designed for out-of-performance-critical code such as garbage collecting to
	// determine if all possible blocks are available and thus there are no 
	// outstanding allocations.
	size_t count_available(size_t _BlockSize) const;

private:
	union tagged_index_t
	{
		unsigned int All;
		struct { unsigned int Tag : 8; unsigned int Index : 24; };
	};

	oCACHE_ALIGNED(tagged_index_t NextAvailable);

	fixed_block_allocator(const fixed_block_allocator&); /* = delete */
	const fixed_block_allocator& operator=(const fixed_block_allocator&); /* = delete */

	fixed_block_allocator(fixed_block_allocator&&); /* = delete */
	fixed_block_allocator&& operator=(fixed_block_allocator*&); /* = delete */
};

// Partially-specializes fixed_block_allocator by binding the block size. The 
// number of blocks is still required for most methods.
template<size_t S>
class fixed_block_allocator_s : fixed_block_allocator
{
public:
	static const size_t block_size = S;
	inline fixed_block_allocator_s(size_t _NumBlocks) : fixed_block_allocator(block_size, _NumBlocks) {}
	inline static size_t max_num_blocks() { fixed_block_allocator::max_num_blocks(); }
	inline static size_t calc_required_size(size_t _NumBlocks) { return fixed_block_allocator::calc_required_size(block_size, _NumBlocks); }
	inline bool valid(size_t _NumBlocks, void* _Pointer) const threadsafe { return fixed_block_allocator::IsValid(block_size, _NumBlocks, _Pointer); }
	inline size_t count_available() const { return fixed_block_allocator::count_available(block_size); }
	inline void* allocate() threadsafe { return fixed_block_allocator::allocate(block_size); }
	inline void deallocate(size_t _NumBlocks, void* _Pointer) threadsafe { return fixed_block_allocator::deallocate(block_size, _NumBlocks, _Pointer); }
};

// Partially-specializes fixed_block_allocator by binding the block size as 
// sizeof(T) but also provides construct/destroy since the type is known. The
// number of blocks is still required for most methods.
template<typename T>
class fixed_block_allocator_t : public fixed_block_allocator_s<sizeof(T)>
{
public:
	typedef T value_type;
	inline fixed_block_allocator_t(size_t _NumBlocks) : fixed_block_allocator_s(_NumBlocks) {}
	inline T* allocate() threadsafe { return static_cast<T*>(fixed_block_allocator_s<sizeof(T)>::allocate()); }
	inline void deallocate(size_t _NumBlocks, T* _Pointer) threadsafe { fixed_block_allocator_s<sizeof(T)>::deallocate(_NumBlocks, _Pointer); }
	oALLOCATOR_CONSTRUCT();
	void destroy(size_t _NumBlocks, T* _Instance) threadsafe
	{
		_Instance->T::~T();
		deallocate(_NumBlocks, _Instance);
	}
};

// Partially-specialized version of fixed_block_allocator that ensures there is
// an inline fixed store.
template<typename T, size_t Capacity>
class backed_fixed_block_allocator
{
public:
	backed_fixed_block_allocator() : Allocator(Capacity) {}
	inline T* allocate() threadsafe { return Allocator.allocate(); }
	inline void deallocate(T* _Pointer) threadsafe { return Allocator.deallocate(Capacity, _Pointer); }
	oALLOCATOR_CONSTRUCT();
	oALLOCATOR_DESTROY();
private:
	fixed_block_allocator_t<T> Allocator;
	T Backing[Capacity];
};

} // namespace oConcurrency

#endif
