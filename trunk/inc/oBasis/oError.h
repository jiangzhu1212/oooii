// $(header)

// NOTE: This allocates thread_local memory and schedules it for freeing using
// oThreadAtExit, so if you see memory leaks relating to this code, ensure
// all threads call oThreadExit() at their end.

// This header defines a standard method for reporting errors in library code.
// Mainly this provides for a thread local last error to be set by functions
// that return false if there is a failure. On top of the GetLastError() or
// _get_errno() pattern this also provides for a matching customizable error 
// string so the erroring code can provide more explicit information such as 
// the filename that wasn't found or the user id of the refused connection. 
#pragma once
#ifndef oError_h
#define oError_h

#include <oBasis/oAssert.h>

enum oERROR
{
	oERROR_NONE, // no error has occurred
	oERROR_GENERIC, // other error that doesn't fit into one of the below
	oERROR_NOT_FOUND, // file not found, entity not found
	oERROR_REDUNDANT, // operation already in progress
	oERROR_CANCELED, // operation was canceled
	oERROR_AT_CAPACITY, // too many elements have already been allocated
	oERROR_END_OF_FILE, // eof
	oERROR_WRONG_THREAD, // function was called on a thread that risks a race condition
	oERROR_BLOCKING, // operation would block
	oERROR_TIMEOUT, // timeout expired
	oERROR_INVALID_PARAMETER, // invalid function parameter
	oERROR_TRUNCATED, // buffer was written to the buffer's limit, but there was more to write
	oERROR_IO, // an IO error occurred
	oERROR_REFUSED, // request actively denied by server/subsystem
	oERROR_PLATFORM, // underlying platform error - check string for specifics
};

// Sets a thread_local value and string that can be retrieved with API described
// below. This should be called by functions implementing a policy where functions
// return true for success, false for failure and on failure oErrorSetLast is 
// called to add more information about the error. NOTE: This function ALWAYS 
// returns false. That way a one-liner can be writen in the common pattern:
// if (somevalue == 0)
//   return oErrorSetLast(oERROR_INVALID_PARAMETER, "somevalue must be nonzero");
bool oErrorSetLastV(oERROR _Error, const char* _Format, va_list _Args);
inline bool oErrorSetLast(oERROR _Error, const char* _Format, ...) { va_list args; va_start(args, _Format); return oErrorSetLastV(_Error, _Format, args); }
inline bool oErrorSetLast(oERROR _Error) { return oErrorSetLast(_Error, nullptr);  }

// Returns the value of the last call to oErrorSetLast. This function is only
// valid immediately after a library function has returned false. Example:
//
// if (MyFunction())
// {	oERROR err = oErrorGetLast(); // this is meaningless and a bad practices: don't check for errors on success
//		if (!MyFunction())
//			MyPrint("Error: %s (%s)", oAsString(oErrorGetLast()), oErrorGetLastString());
// }
oERROR oErrorGetLast();
const char* oErrorGetLastString();

// Returns a monotonically increasing counter that increments each time 
// oErrorSetLast is called.
size_t oErrorGetLastCount();

// For functions that follow the pattern of returning true for success and false
// for failure while using oErrorSetLast, use this as a wrapper when the 
// function should never fail in a non-development situation.

#if oENABLE_ASSERTS == 1
	#define oVERIFY(fn) do { if (!(fn)) { oASSERT_PRINT(oASSERT_ASSERTION, oASSERT_IGNORE_ONCE, #fn, "%s: %s", oAsString(oErrorGetLast()), oErrorGetLastString()); } } while(false)
	#define oVERIFYIS(fn, expected) do { if ((fn != expected)) { oASSERT_PRINT(oASSERT_ASSERTION, oASSERT_IGNORE_ONCE, #fn, "%s: %s", oAsString(oErrorGetLast()), oErrorGetLastString()); } } while(false)
#else
	#define oVERIFY(fn) fn
	#define oVERIFYIS(fn, expected) fn
#endif
#endif
