// $(header)
#pragma once
#ifndef oCRTLeakTracker_h
#define oCRTLeakTracker_h

#include <oBasis/oHash.h>
#include <oBasis/oLeakTracker.h>
#include <oBasis/oMutex.h>
#include <oPlatform/oSingleton.h>
#include <unordered_map>
#include "oDbgHelp.h"

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

	bool ReportLeaks(bool _AllContexts = false);

	int OnMallocEvent(int _AllocationType, void* _UserData, size_t _Size, int _BlockType, long _RequestNumber, const unsigned char* _Path, int _Line);

	static const oGUID GUID;

protected:

	static int MallocHook(int _AllocationType, void* _UserData, size_t _Size, int _BlockType, long _RequestNumber, const unsigned char* _Path, int _Line);

	oRef<oDbgHelp> DbgHelp;
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
