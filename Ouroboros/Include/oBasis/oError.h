// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// This is a replacement for _get_errno() because the standard does not support
// DLLs well. Sure the value is thread-local, but each DLL gets its own set of 
// thread-local values. oError does not.
#pragma once
#ifndef oError_h
#define oError_h

#include <oBase/assert.h>
#include <system_error>

// Sets a thread_local value and string that can be retrieved with API described
// below. This should be called by functions implementing a policy where functions
// return true for success, false for failure and on failure oErrorSetLast is 
// called to add more information about the error. NOTE: This function ALWAYS 
// returns false. That way a one-liner can be written in the common pattern:
// if (somevalue == 0)
//   return oErrorSetLast(std::errc::invalid_argument, "somevalue must be nonzero");
bool oErrorSetLast(errno_t _Error);
bool oErrorSetLastV(errno_t _Error, const char* _Format, va_list _Args);
inline bool oErrorSetLast(errno_t _Error, const char* _Format, ...) { va_list args; va_start(args, _Format); bool success = oErrorSetLastV(_Error, _Format, args); va_end(args); return success; }

inline bool oErrorSetLast(std::exception& _Exception)
{
	std::system_error* se = dynamic_cast<std::system_error*>(&_Exception);
	return oErrorSetLast(se ? static_cast<errno_t>(se->code().value()) : std::errc::protocol_error, _Exception.what());
}

// Retains the current error message adding the provided string to the front
// of the error message. As oErrorSetLast this always returns false
bool oErrorPrefixLastV(const char* _Format, va_list _Args);
inline bool oErrorPrefixLast(const char* _Format, ...) { va_list args; va_start(args, _Format); bool b = oErrorPrefixLastV(_Format, args); va_end(args); return b; }

// Returns the value of the last call to oErrorSetLast. This function is only
// valid immediately after a library function has returned false. Example:
//
// if (MyFunction())
// {	errno_t err = oErrorGetLast(); // this is meaningless and a bad practice: don't check for errors on success
//		if (!MyFunction())
//			MyPrint("Error: %s (%s)", oErrorAsString(oErrorGetLast()), oErrorGetLastString());
// }
errno_t oErrorGetLast();
const char* oErrorGetLastString();

// Returns the errno_t as a string exactly (as if it were an enum type)
const char* oErrorAsString(errno_t _Error);

// If no explicit error string is set, oErrorGetLastString() will return a call
// to this function.
const char* oErrorGetDefaultString(errno_t _Error);

// Returns a monotonically increasing counter that increments each time 
// oErrorSetLast is called.
size_t oErrorGetLastCount();

size_t oErrorGetSizeofMessageBuffer();

inline void oThrowLastError() { throw std::system_error(std::make_error_code((std::errc::errc)oErrorGetLast()), oErrorGetLastString()); }

// For functions that follow the pattern of returning true for success and false
// for failure while using oErrorSetLast, use this as a wrapper when the 
// function should never fail in a non-development situation.

#define oVERIFY_R(_BoolResultFunction) do { if (!(_BoolResultFunction)) { return false; } } while (false)

#if oENABLE_ASSERTS == 1
	#define oVERIFY(_BoolResultFunction) oASSERT(_BoolResultFunction, "%s: %s", oErrorAsString(oErrorGetLast()), oErrorGetLastString())
#else
	#define oVERIFY(_BoolResultFunction) _BoolResultFunction
#endif

#endif
