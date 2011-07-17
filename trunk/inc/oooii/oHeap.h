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
#pragma once
#ifndef oHeap_h
#define oHeap_h

namespace oHeap
{
	enum TYPE
	{
		STACK, // the stack
		SYSTEM, // true underlying OS allocator
		LIBC, // libc's malloc
		STATIC, // heap shared by DLLs/modules, but unique per-process
		EXTERNAL, // a sibling or parent of libc's malloc, not visible to vanilla c api
		INTERNAL, // a user allocator that uses a heap allocated from the system allocator
		MAPPED, // memory that represents memory in another process or on another medium
	};

	struct BLOCK_DESC
	{
		void* Address;
		unsigned long long Size;
		unsigned long long Overhead;
		bool Used;
	};

	struct DESC
	{
		char Name[64];
		char Description[64];
		TYPE Type;
	};

	struct STATISTICS
	{
		unsigned long long BlocksFree;
		unsigned long long BlocksUsed;
		unsigned long long Blocks;
		unsigned long long BytesFree;
		unsigned long long BytesUsed;
		unsigned long long Bytes;
		unsigned long long Overhead;
		unsigned long long BlocksSmall;
		unsigned long long BlocksMedium;
		unsigned long long BlocksLarge;
		bool Valid;
	};

	struct PROCESS_HEAP_DESC
	{
		unsigned long long NumPageFaults;
		unsigned long long WorkingSet;
		unsigned long long PeakWorkingSet;
		unsigned long long PrivateUsage;
		unsigned long long PageFileUsage;
		unsigned long long PeakPageFileUsage;
	};

	struct GLOBAL_HEAP_DESC
	{
		unsigned long long TotalMemoryUsed;
		unsigned long long AvailablePhysical;
		unsigned long long TotalPhysical;
		unsigned long long AvailableVirtualProcess;
		unsigned long long TotalVirtualProcess;
		unsigned long long AvailablePaged;
		unsigned long long TotalPaged;
	};

	typedef void (*WalkerFn)(const BLOCK_DESC* pDesc, void* user, long flags);
	typedef void (*DescriberFn)(const DESC* pDesc, void* user, long flags);
	typedef void (*SummarizerFn)(const STATISTICS* pStats, void* user, long flags);

	static const unsigned long long SMALL_BLOCK_SIZE = 480;
	static const unsigned long long LARGE_BLOCK_SIZE = 2 * 1024 * 1024;
	// a medium-sized block is anything in between small and large

	inline bool IsSmall(unsigned long long _Size) { return _Size <= SMALL_BLOCK_SIZE; }
	inline bool IsMedium(unsigned long long _Size) { return _Size > SMALL_BLOCK_SIZE && _Size <= LARGE_BLOCK_SIZE; }
	inline bool IsLarge(unsigned long long _Size) { return _Size > LARGE_BLOCK_SIZE; }

	// Sums the specified BLOCK_DESC into the specified STATS
	void Add(STATISTICS* _pStats, const BLOCK_DESC* _pDesc);

	bool WalkExternals(WalkerFn _Walker, DescriberFn _Describer, SummarizerFn _Summarizer, void* _pUserData, long _Flags = 0);
	void SetSmallBlockHeapThreshold(unsigned long long _sbhMaxSize);

	void GetProcessDesc(PROCESS_HEAP_DESC* _pDesc);
	void GetGlobalDesc(GLOBAL_HEAP_DESC* _pDesc);

	unsigned int GetSystemAllocationGranularity();
} // namespace oHeap

#endif
