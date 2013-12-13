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
#include <oBase/tlsf_allocator.h>
#include <oBase/assert.h>
#include <oBase/byte.h>
#include <oBase/config.h>
#include <oBase/fixed_string.h>
#include <oBase/throw.h>
#include <tlsf.h>

namespace ouro {

tlsf_allocator::tlsf_allocator()
	: hHeap(nullptr)
	, pArena(nullptr)
	, Size(0)
{}

tlsf_allocator::tlsf_allocator(void* _pArena, size_t _Size)
	: hHeap(nullptr)
	, pArena(_pArena)
	, Size(_Size)
{
	reset();
	if (!valid())
		throw std::invalid_argument("invalid arena and/or size specified");
}

static void trace_leaks(void* ptr, size_t size, int used, void* user)
{
	if (used)
	{
		sstring mem;
		format_bytes(mem, size, 2);
		oTRACE("tlsf leak: 0x%p %s", ptr, mem.c_str());
	}
}

tlsf_allocator::~tlsf_allocator()
{
	if (Stats.num_allocations)
	{
		tlsf_walk_heap(hHeap, trace_leaks, nullptr);
		throw std::runtime_error(formatf("allocator destroyed with %u outstanding allocations", Stats.num_allocations));
	}

	if (!valid())
		throw std::runtime_error("tlsf heap is corrupt");

	tlsf_destroy(hHeap);
}

tlsf_allocator::tlsf_allocator(tlsf_allocator&& _That)
{
	operator=(std::move(_That));
}

tlsf_allocator& tlsf_allocator::operator=(tlsf_allocator&& _That)
{
	if (this != &_That)
	{
		hHeap = _That.hHeap; _That.hHeap = nullptr;
		pArena = _That.pArena; _That.pArena = nullptr;
		Size = _That.Size; _That.Size = 0;
		Stats = _That.Stats; _That.Stats = stats();
	}

	return *this;
}

void* tlsf_allocator::allocate(size_t _Size, size_t _Alignment)
{
	void* p = tlsf_memalign(hHeap, _Alignment, __max(_Size, 1));
	if (p)
	{
		size_t blockSize = tlsf_block_size(p);
		Stats.num_allocations++;
		Stats.bytes_allocated += blockSize;
		Stats.bytes_free -= blockSize;
		Stats.bytes_allocated_peak = max(Stats.bytes_allocated_peak, Stats.bytes_allocated);
	}

	return p;
}

void* tlsf_allocator::reallocate(void* _Pointer, size_t _Size)
{
	size_t oldBlockSize = _Pointer ? tlsf_block_size(_Pointer) : 0;
	void* p = tlsf_realloc(hHeap, _Pointer, _Size);
	if (p)
	{
		size_t blockSizeDiff = tlsf_block_size(p) - oldBlockSize;
		Stats.bytes_allocated += blockSizeDiff;
		Stats.bytes_free -= blockSizeDiff;
		Stats.bytes_allocated_peak = max(Stats.bytes_allocated_peak, Stats.bytes_allocated);
	}

	return 0;
}

void tlsf_allocator::deallocate(void* _Pointer)
{
	if (_Pointer)
	{
		const size_t blockSize = tlsf_block_size(_Pointer);
		tlsf_free(hHeap, _Pointer);
		Stats.num_allocations--;
		Stats.bytes_allocated -= blockSize;
		Stats.bytes_free += blockSize;
	}
}

size_t tlsf_allocator::size(void* _Pointer) const 
{
	return tlsf_block_size(_Pointer);
}

bool tlsf_allocator::in_range(void* _Pointer) const
{
	return _Pointer >= pArena && _Pointer < byte_add(pArena, Size);
}

bool tlsf_allocator::valid() const
{
	return hHeap && !tlsf_check_heap(hHeap);
}

void tlsf_allocator::reset()
{
	if (!pArena || !Size)
		throw std::runtime_error("allocator not valid");

	void* pAlignedArena = byte_align(pArena, oDEFAULT_MEMORY_ALIGNMENT);
	size_t alignedArenaSize = Size - std::distance((char*)pArena, (char*)pAlignedArena);
	hHeap = tlsf_create(pAlignedArena, alignedArenaSize);
	Stats = stats();
	Stats.bytes_free = alignedArenaSize - tlsf_overhead();
}

static void tlsf_walker(void* ptr, size_t size, int used, void* user)
{
	std::function<void(void* _Pointer, size_t _Size, bool _IsUserAllocation)>& 
		Walker = *(std::function<void(void* _Pointer, size_t _Size, bool _Used)>*)user;
	Walker(ptr, size, !!used);
}

void tlsf_allocator::walk_heap(const std::function<void(void* _Pointer, size_t _Size, bool _Used)>& _Enumerator)
{
	if (_Enumerator)
		tlsf_walk_heap(hHeap, tlsf_walker, (void*)&_Enumerator);
}

} // namespace ouro
