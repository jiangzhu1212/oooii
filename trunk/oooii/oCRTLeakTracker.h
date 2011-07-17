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
#ifndef oCRTLeakTracker_h
#define oCRTLeakTracker_h

#include <oooii/oHash.h>
#include <oooii/oSingleton.h>
#include <unordered_map>

struct oCRTMallocTracker : oProcessSingleton<oCRTMallocTracker>
{
	static const size_t STACK_TRACE_DEPTH = 32;
	static const size_t STACK_TRACE_OFFSET = 8;

	struct ALLOCATION_DESC
	{
		void* Address;
		size_t Size;
		unsigned long long StackTrace[STACK_TRACE_DEPTH];
		unsigned int NumStackEntries;
		unsigned int Line;
		unsigned int RequestNumber;
		char Path[_MAX_PATH];
		inline bool operator==(const ALLOCATION_DESC& _Other) { return Address == _Other.Address; }
	};

	oCRTMallocTracker();
	~oCRTMallocTracker();

	void Enable(bool _Enabled = true);
	bool IsEnabled() const;

	void Report(bool _Report = true);
	bool IsReportEnabled() const;

	void ReportLeaks();

protected:

	void OnAllocation(void* _Address, size_t _Size, long _RequestNumber, const char* _Path, unsigned int _Line, bool _IsReallocation);
	void OnDeallocation(void* _Address);

	static int CallOriginalHook(int _AllocationType, void* _UserData, size_t _Size, int _BlockType, long _RequestNumber, const unsigned char* _Path, int _Line);
	static int MallocHook(int _AllocationType, void* _UserData, size_t _Size, int _BlockType, long _RequestNumber, const unsigned char* _Path, int _Line);

	static struct HashAllocation { size_t operator()(long _RequestNumber) const { return oHash_superfast(&_RequestNumber, sizeof(_RequestNumber)); } };
	typedef std::tr1::unordered_map<long, ALLOCATION_DESC, HashAllocation> allocations_t;

	allocations_t Allocations;
	_CRT_ALLOC_HOOK OriginalAllocHook;
	bool Enabled;
	bool ReportEnabled;
	bool InInternalProcesses; // don't track allocations this context makes to do the tracking

	static oCRTMallocTracker* sInstanceForDeferredRelease;
	static void AtExit();
};

#endif
