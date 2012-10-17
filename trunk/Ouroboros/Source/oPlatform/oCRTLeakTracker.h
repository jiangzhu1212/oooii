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

#include <oBasis/oHash.h>
#include <oBasis/oLeakTracker.h>
#include <oBasis/oMutex.h>
#include <oPlatform/oSingleton.h>
#include <unordered_map>

// @oooii-tony: This is not well-designed. The underlying oLeakTracker part is
// a bit better, but this is just the clunky bridge/glue code that binds it to
// the system malloc. I'm not wholly sure what the best way to make it work is,
// so until then I won't bother describing it. If you want to use this class
// more explicitly that it's automagical current integration into the code, come
// find me until I really make this a first-class object.

struct oCRTLeakTracker : oProcessSingleton<oCRTLeakTracker>
{
	oCRTLeakTracker();
	~oCRTLeakTracker();

	inline void NewContext() threadsafe { pLeakTracker->NewContext(); }
	inline void CaptureCallstack(bool _Capture = true) threadsafe { pLeakTracker->CaptureCallstack(_Capture); }
	inline void Reset() threadsafe { pLeakTracker->Reset(); }
	inline void EnableThreadlocalTracking(bool _Enabled = true) threadsafe { pLeakTracker->EnableThreadlocalTracking(_Enabled); }

	void Enable(bool _Enabled = true);
	bool IsEnabled() const;

	void Report(bool _Report = true);
	bool IsReportEnabled() const;

	bool ReportLeaks(bool _CurrentContextOnly = true);

	int OnMallocEvent(int _AllocationType, void* _UserData, size_t _Size, int _BlockType, long _RequestNumber, const unsigned char* _Path, int _Line);

	// In rare low-level systems that need to persist after leaks have been 
	// reported, it is helpful not to report those allocations as a leak. For 
	// example a log file that is going to retain the leak report itself should
	// not be reported as a leak. DO NOT USE THIS TO JUST FIX LEAK REPORTS!
	void UntrackAllocation(void* _Pointer);

	static const oGUID GUID;

	inline void ReferenceDelay() threadsafe { pLeakTracker->ReferenceDelay(); }
	inline void ReleaseDelay() threadsafe { pLeakTracker->ReleaseDelay(); }

protected:

	static int MallocHook(int _AllocationType, void* _UserData, size_t _Size, int _BlockType, long _RequestNumber, const unsigned char* _Path, int _Line);

	oMutex Mutex;
	oLeakTracker* pLeakTracker;
	size_t NonLinearBytes;
	_CRT_ALLOC_HOOK OriginalAllocHook;
	bool Enabled;
	bool ReportEnabled;

	static oCRTLeakTracker* sInstanceForDeferredRelease;
	static void AtExit();
};

#endif
