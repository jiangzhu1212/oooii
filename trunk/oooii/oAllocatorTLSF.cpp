/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#include "pch.h"
#include "oAllocatorTLSF.h"
#include <oooii/oByte.h>
#include "tlsf.h"

oAllocatorTLSF::oAllocatorTLSF(const char* _DebugName, const DESC* _pDesc, bool* _pSuccess)
	: Desc(*_pDesc)
{
	*_pSuccess = false;
	*DebugName = 0;
	strcpy_s(DebugName, oSAFESTRN(_DebugName));
	oAllocatorTLSF::Reset();
	if( !oAllocatorTLSF::IsValid() )
	{
		oSetLastError(EINVAL, "Failed to construct TLSF allocator");
		return;
	}

	*_pSuccess = true;
}

oAllocatorTLSF::~oAllocatorTLSF()
{
	oASSERT(Stats.NumAllocations == 0, "Allocator being destroyed with %u allocations still unfreed! This may leave dangling pointers.", Stats.NumAllocations);
	oASSERT(oAllocatorTLSF::IsValid(), "TLSF Heap is corrupt");
	tlsf_destroy(hPool);
}

void oAllocatorTLSF::GetDesc(DESC* _pDesc)
{
	*_pDesc = Desc;
}

void oAllocatorTLSF::GetStats(STATS* _pStats)
{
	*_pStats = Stats;
}

bool oAllocatorTLSF::IsValid()
{
	return hPool && !tlsf_check_heap(hPool);
}

void* oAllocatorTLSF::Allocate(size_t _NumBytes, size_t _Alignment)
{
	void* p = tlsf_memalign(hPool, _Alignment, __max(_NumBytes, 1));
	if (p)
	{
		size_t blockSize = tlsf_block_size(p);
		Stats.NumAllocations++;
		Stats.BytesAllocated += blockSize;
		Stats.BytesFree	-= blockSize;
		Stats.PeakBytesAllocated = __max(Stats.PeakBytesAllocated, Stats.BytesAllocated);
	}

	return p;
}

void* oAllocatorTLSF::Reallocate(void* _Pointer, size_t _NumBytes)
{
	size_t oldBlockSize = _Pointer ? tlsf_block_size(_Pointer) : 0;
	void* p = tlsf_realloc(hPool, _Pointer, _NumBytes);
	if (p)
	{
		size_t blockSizeDiff = tlsf_block_size(p) - oldBlockSize;
		Stats.BytesAllocated += blockSizeDiff;
		Stats.BytesFree	-= blockSizeDiff;
		Stats.PeakBytesAllocated = __max(Stats.PeakBytesAllocated, Stats.BytesAllocated);
	}

	return 0;
}

void oAllocatorTLSF::Deallocate(void* _Pointer)
{
	if (_Pointer)
	{
		size_t blockSize = tlsf_block_size(_Pointer);
		tlsf_free(hPool, _Pointer);
		Stats.NumAllocations--;
		Stats.BytesAllocated -= blockSize;
		Stats.BytesFree += blockSize;
	}
}

size_t oAllocatorTLSF::GetBlockSize(void* _Pointer)
{
	return tlsf_block_size(_Pointer);
}

void oAllocatorTLSF::Reset()
{
	memset(&Stats, 0, sizeof(Stats));
	void* pRealArenaStart = oByteAlign(oByteAdd(Desc.pArena, sizeof(oAllocatorTLSF)), oDEFAULT_MEMORY_ALIGNMENT);
	size_t realArenaSize = Desc.ArenaSize - std::distance((char*)Desc.pArena, (char*)pRealArenaStart);
	hPool = tlsf_create(pRealArenaStart, realArenaSize);
	Stats.NumAllocations = 0;
	Stats.BytesAllocated = 0;
	Stats.PeakBytesAllocated = 0;
	Stats.BytesFree = Desc.ArenaSize - tlsf_overhead();
}

struct TLSFContext
{
	oAllocator::WalkerFn Walker;
	void* pUserData;
	long Flags;
};

static void TLSFWalker(void* ptr, size_t size, int used, void* user)
{
	TLSFContext* ctx = (TLSFContext*)user;
	oAllocator::BLOCK_DESC b;
	b.Address = ptr;
	b.Size = size;
	b.Used = !!used;
	ctx->Walker(&b, ctx->pUserData, ctx->Flags);
}

void oAllocatorTLSF::WalkHeap(WalkerFn _Walker, void* _pUserData, long _Flags)
{
	TLSFContext ctx;
	ctx.Walker = _Walker;
	ctx.pUserData = _pUserData;
	ctx.Flags = _Flags;
	tlsf_walk_heap(hPool, TLSFWalker, &ctx);
}
