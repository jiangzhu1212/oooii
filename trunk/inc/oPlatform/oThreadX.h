// $(header)
// Extension API to the oThread interface
#pragma once
#ifndef oThreading_h
#define oThreading_h

#include <oBasis/oThread.h>

// Returns the unique ID for the main thread that initialized the session for 
// the process.
oThread::id oThreadGetMainID();

// Return the native handle of the thread that calls this function (HANDLE on 
// Windows)
oThread::native_handle_type oThreadGetCurrentNativeHandle();

bool oThreadSetAffinity(oStd::thread& _Thread, size_t _AffinityMask);

enum oTHREAD_PRIORITY
{
	oTHREAD_PRIORITY_NOT_RUNNING,
	oTHREAD_PRIORITY_LOWEST,
	oTHREAD_PRIORITY_LOW,
	oTHREAD_PRIORITY_NORMAL,
	oTHREAD_PRIORITY_HIGH,
	oTHREAD_PRIORITY_HIGHEST,
};

bool oThreadSetPriority(threadsafe oThread& _Thread, oTHREAD_PRIORITY _Priority);
oTHREAD_PRIORITY oThreadGetPriority(oThread& _Thread);

#endif
