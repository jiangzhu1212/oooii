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
// Because this is used in a grow-by-chunk allocator (oBlockAllocatorGrowable)
// redundant data such as block size and number of blocks is not cached by this
// class.

#ifndef oBlockAllocatorFixed_h
#define oBlockAllocatorFixed_h

#include <oBasis/oAssert.h>
#include <oBasis/oCallable.h>
#include <oBasis/oInvalid.h>
#include <oBasis/oNoncopyable.h>
#include <oBasis/oThreadsafe.h>

// Defines methods for calling ctors and dtors on allocations
#ifndef oHAS_VARIADIC_TEMPLATES
	#define oALLOCATOR_CREATE() \
		T* Create() threadsafe { T* p = Allocate(); return p ? new (p) T() : nullptr; } \
		template<oARG_TYPENAMES1> T* Create(oARG_DECL1) threadsafe { T* p = Allocate(); return p ? new (p) T(oARG_PASS1) : nullptr; } \
		template<oARG_TYPENAMES2> T* Create(oARG_DECL2) threadsafe { T* p = Allocate(); return p ? new (p) T(oARG_PASS2) : nullptr; } \
		template<oARG_TYPENAMES3> T* Create(oARG_DECL3) threadsafe { T* p = Allocate(); return p ? new (p) T(oARG_PASS3) : nullptr; } \
		template<oARG_TYPENAMES4> T* Create(oARG_DECL4) threadsafe { T* p = Allocate(); return p ? new (p) T(oARG_PASS4) : nullptr; } \
		template<oARG_TYPENAMES5> T* Create(oARG_DECL5) threadsafe { T* p = Allocate(); return p ? new (p) T(oARG_PASS5) : nullptr; } \
		template<oARG_TYPENAMES6> T* Create(oARG_DECL6) threadsafe { T* p = Allocate(); return p ? new (p) T(oARG_PASS6) : nullptr; } \
		template<oARG_TYPENAMES7> T* Create(oARG_DECL7) threadsafe { T* p = Allocate(); return p ? new (p) T(oARG_PASS7) : nullptr; } \
		template<oARG_TYPENAMES8> T* Create(oARG_DECL8) threadsafe { T* p = Allocate(); return p ? new (p) T(oARG_PASS8) : nullptr; } \
		template<oARG_TYPENAMES9> T* Create(oARG_DECL9) threadsafe { T* p = Allocate(); return p ? new (p) T(oARG_PASS9) : nullptr; } \
		template<oARG_TYPENAMES10> T* Create(oARG_DECL10) threadsafe { T* p = Allocate(); return p ? new (p) T(oARG_PASS10) : nullptr; }
	#define oALLOCATOR_DESTROY() \
		void Destroy(T* _Instance) threadsafe \
		{	oASSERT(_Instance, "Destroying a null pointer"); \
			_Instance->T::~T(); \
			Deallocate(_Instance); \
		}
#else
	#error Use variadic templates to implement Create()/Destroy()
#endif

class oBlockAllocatorFixed : oNoncopyable
{
	union tagged_index_t
	{
		unsigned int All;
		struct 
		{
			unsigned int Tag : 8;
			unsigned int Index : 24;
		};
	};

	tagged_index_t NextAvailable;

public:

	oBlockAllocatorFixed()
	{
		NextAvailable.All = oInvalid;
	}

	// unsigned short is used for the freelist index, so only 16-bits worth of
	// blocks can be indexed. This is to enable very small allocation blocks, such
	// as 16-bit indices. If more blocks are required, consider using 
	// oBlockAllocatorGrowable. Note: 0xffff is a reserved value.
	static const size_t max_num_blocks = 65534;

	/*constexpr*/ static size_t GetMaxNumBlocks() { return max_num_blocks; }

	// Returns the number of bytes required to hold a valid fixed-size block 
	// allocator.
	static size_t CalculateRequiredSize(size_t _BlockSize, size_t _NumBlocks);

	// Returns true if the specified pointer was allocated from this allocator.
	bool IsValid(size_t _BlockSize, size_t _NumBlocks, void* _Pointer) const threadsafe;

	// Initialize this fixed-size block allocator to be fully empty
	void Initialize(size_t _BlockSize, size_t _NumBlocks);

	// Slow! This walks the free list counting how many members there are. This is
	// designed for out-of-performance-critical code such as garbage collecting to
	// determine if all possible blocks are available and thus there are no 
	// outstanding allocations.
	size_t CountAvailable(size_t _BlockSize) const;

	// Allocate a block. This will return nullptr if there is no available room.
	void* Allocate(size_t _BlockSize) threadsafe;

	// Mark a block as available
	void Deallocate(size_t _BlockSize, size_t _NumBlocks, void* _Pointer) threadsafe;
};

template<size_t S> class oBlockAllocatorFixedS : oBlockAllocatorFixed
{
public:
	static const size_t block_size = S;
	inline static size_t GetMaxNumBlocks() { oBlockAllocatorFixed::GetMaxNumBlocks(); }
	inline static size_t CalculateRequiredSize(size_t _NumBlocks) { return oBlockAllocatorFixed::CalculateRequiredSize(block_size, _NumBlocks); }
	inline bool IsValid(size_t _NumBlocks, void* _Pointer) const threadsafe { return oBlockAllocatorFixed::IsValid(block_size, _NumBlocks, _Pointer); }
	inline void Initialize(size_t _NumBlocks) { oBlockAllocatorFixed::Initialize(block_size, _NumBlocks); }
	inline size_t CountAvailable() const { return oBlockAllocatorFixed::CountAvailable(block_size); }
	inline void* Allocate() threadsafe { return oBlockAllocatorFixed::Allocate(block_size); }
	inline void Deallocate(size_t _NumBlocks, void* _Pointer) threadsafe { return oBlockAllocatorFixed::Deallocate(block_size, _NumBlocks, _Pointer); }
};

// A templated version of the above that uses Create/Destroy rather than 
// Allocate/Deallocate.
template<typename T>
class oBlockAllocatorFixedT : public oBlockAllocatorFixedS<sizeof(T)>
{
public:
	typedef T value_type;
	inline T* Allocate() threadsafe { return static_cast<T*>(oBlockAllocatorFixedS<sizeof(T)>::Allocate()); }
	inline void Deallocate(size_t _NumBlocks, T* _Pointer) threadsafe { oBlockAllocatorFixedS<sizeof(T)>::Deallocate(_NumBlocks, _Pointer); }
	oALLOCATOR_CREATE();
	void Destroy(size_t _NumBlocks, T* _Instance) threadsafe
	{
		_Instance->T::~T();
		Deallocate(_NumBlocks, _Instance);
	}
};

template<typename T, size_t Capacity>
class oBlockAllocatorBackedT
{
public:
	oBlockAllocatorBackedT()
	{
		Allocator.Initialize(Capacity);
	}

	inline T* Allocate() threadsafe { return Allocator.Allocate(); }
	inline void Deallocate(T* _Pointer) threadsafe { return Allocator.Deallocate(Capacity, _Pointer); }
	oALLOCATOR_CREATE();
	oALLOCATOR_DESTROY();
private:
	oBlockAllocatorFixedT<T> Allocator;
	T Backing[Capacity];
};

#endif
