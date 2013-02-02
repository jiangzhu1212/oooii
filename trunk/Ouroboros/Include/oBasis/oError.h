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
#include <oBasis/oStringize.h>

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
	oERROR_CORRUPT, // File/buffer/message is corrupt
	oERROR_LEAKS, // The operation was logically successful, but resources remain inappropriately allocated (mostly a unit test return value)
};

// Sets a thread_local value and string that can be retrieved with API described
// below. This should be called by functions implementing a policy where functions
// return true for success, false for failure and on failure oErrorSetLast is 
// called to add more information about the error. NOTE: This function ALWAYS 
// returns false. That way a one-liner can be writen in the common pattern:
// if (somevalue == 0)
//   return oErrorSetLast(oERROR_INVALID_PARAMETER, "somevalue must be nonzero");
bool oErrorSetLast(oERROR _Error);
bool oErrorSetLastV(oERROR _Error, const char* _Format, va_list _Args);
inline bool oErrorSetLast(oERROR _Error, const char* _Format, ...) { va_list args; va_start(args, _Format); bool success = oErrorSetLastV(_Error, _Format, args); va_end(args); return success;}

// Retains the current error message adding the provided string to the front
// of the error message. As oErrorSetLast this always returns false
bool oErrorPrefixLastV(const char* _Format, va_list _Args);
inline bool oErrorPrefixLast(const char* _Format, ...) { va_list args; va_start(args, _Format); return oErrorPrefixLastV(_Format, args); }

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

size_t oErrorGetSizeofMessageBuffer();

// For functions that follow the pattern of returning true for success and false
// for failure while using oErrorSetLast, use this as a wrapper when the 
// function should never fail in a non-development situation.
#if oENABLE_ASSERTS == 1
	#define oVERIFY(_BoolResultFunction) do { if (!(_BoolResultFunction)) { oASSERT_PRINT(oASSERT_ASSERTION, oASSERT_IGNORE_ONCE, #_BoolResultFunction, "%s: %s", oAsString(oErrorGetLast()), oErrorGetLastString()); } } while(false)
#else
	#define oVERIFY(_BoolResultFunction) _BoolResultFunction
#endif

// This checks an oStd::future for an error
#define oVERIFYF(_oStdFuture) do { if (_oStdFuture.has_error()) { _oStdFuture.set_last_error(); oVERIFY(_oStdFuture.has_value()); } } while(false)

#endif
