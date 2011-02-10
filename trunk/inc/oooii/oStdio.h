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
#ifndef oStdio_h
#define oStdio_h

#include <oooii/oStddef.h>
#include <stdio.h>

// returns system ticks as seconds since program started
double oTimer();

// oTimer, but in in milliseconds
inline float oTimerMSF() { return (float)oTimer()*1000.0f; }
inline unsigned int oTimerMS() { return static_cast<unsigned int>(oTimerMSF()); }

// Wrapper for taking the diff between two oTimer() results. Using this minimizes
// type conversions such as double -> float and float -> unsigned int.
inline unsigned int oTimerGetDiffMS(double _StartTime, double _EndTime) { return static_cast<unsigned int>(static_cast<float>(_EndTime - _StartTime) * 1000.0f); }

class oScopedPartialTimeout
{
	// Sometimes it is necessary to call a system function that takes a timeout
	// value in a loop. So from interface/user space we'd like the calling function
	// to respect the timeout we gave, even if the lower-level code does something
	// less elegant. To encapsulate this common pattern, here is a scoped timeout
	// object that decrements a timeout value as code within it's scope executes.
	// Often usage of this object implies careful application of {} braces, which 
	// is still cleaner code than maintaining timer calls.

	unsigned int* pTimeoutMSCountdown;
	double Start;
public:
	// Pointer to a timeout value to update. It should be initialized to the user-
	// specified timeout initially and then allowed to be updated throughout. If 
	// the value oof the timeout is ~0u (oINFINITE_WAIT), then this class doesn't
	// update the value, thus allowing the ~0u value to be propagated.
	oScopedPartialTimeout(unsigned int* _pTimeoutMSCountdown);
	~oScopedPartialTimeout();

	// Updates the timeout in the same way that destructor does
	void UpdateTimeout();
};

errno_t oGetHostname(char* _Hostname, size_t _SizeofHostName);
template<size_t size> inline errno_t oGetHostname(char (&_Hostname)[size]) { return oGetHostname(_Hostname, size); }

// Wrapper for use assert/debug macros ONLY because it's returning thread-shared memory
inline const char* oGetHostname() { static char buf[512]; oGetHostname(buf); return buf; }

// _____________________________________________________________________________
// Advanced memcpy

// Sets an int value at a time. This is probably slower than
// c's memset, but this sets a full int value rather than
// a char value. This is useful for setting 0xdeadbeef or 
// 0xfeeefeee to memory rather than memset's dede or fefefe.
void oMemset4(void* _pDestination, long _Value, size_t _NumBytes);

// 2D copies for copying image data, stuff that's easier to 
// conceptualize as 2D rather than 1D.
void oMemcpy2d(void* oRESTRICT _pDestination, size_t _DestinationPitch, const void* oRESTRICT _pSource, size_t _SourcePitch, size_t _SourceRowSize, size_t _NumRows);
void oMemset2d(void* _pDestination, size_t _Pitch, int _Value, size_t _SetPitch, size_t _NumRows);

// sets words at a time.
void oMemset2d4(void* _pDestination, size_t _Pitch, long _Value, size_t _SetPitch, size_t _NumRows);

// Copies asymmetrical memory. This is most applicable when converting from AOS
// source to an SOA destination or vice versa. If copying SOA-style data to AOS-
// style data, ensure _pDestination is pointing at the first field in the struct 
// to write, not always the start of the struct itself.
void oMemcpyAsym(void* oRESTRICT _pDestination, size_t _DestinationStride, const void* oRESTRICT _pSource, size_t _SourceStride, size_t _NumElements);

// Copies 32-bit values to 16-bit values (sometimes useful when working with 
// graphics index buffers). Remember, _NumElements is count of unsigned ints 
// in _pSource and _pDestination has been pre-allocated to take at least the 
// same number of unsigned shorts.
void oMemcpyToUshort(unsigned short* _pDestination, const unsigned int* _pSource, size_t _NumElements);

// Reverse of oMemcpyToUshort. Remember, _NumElements is count of unsigned shorts
// in _pSource and _pDestination has been pre-allocated to take at least the
// same number of unsigned ints.
void oMemcpyToUint(unsigned int* _pDestination, const unsigned short* _pSource, size_t _NumElements);

// _____________________________________________________________________________
// File-related utilities. These are implemented using the fopen API. There is
// no abstraction for fopen, so use it directly for generic file operations not 
// covered by this API.

namespace oFile
{
	struct DESC
	{
		time_t Created;
		time_t Accessed;
		time_t Written;
		unsigned long long Size;
		bool Directory:1;
		bool Archive:1;
		bool Compressed:1;
		bool Encrypted:1;
		bool Hidden:1;
		bool ReadOnly:1;
		bool System:1;
		bool Offline:1;
		char Filename[_MAX_PATH];
	};

	// Find first file prefixed with the specified wildcard. If this returns 
	// true, then context can be passed to oFindNextFile and must be closed when 
	// finished by using CloseFind. If this returns false, context will be NULL 
	// and thus not need to be closed. False is returned if there are no more 
	// matches.
	bool FindFirst(DESC* _pDesc, const char* _Wildcard, void** _pFindContext);
	
	// Iterate on a context created using FindFirst. False means there are no
	// more files.
	bool FindNext(DESC* _pDesc, void* _pFindContext);

	// This call must be made on the find context created with a successful call 
	// to FindFirst. If FindFirst fails, CloseFind should not be called.
	bool CloseFind(void* _pFindContext);

	// Returns size of an open file. This may open and close the file on some 
	// systems.
	size_t GetSize(FILE* _File);

	bool Touch(const char* _Path, time_t _PosixTimestamp);

	// Remember, the file must've been open with read access 
	// (i.e. fopen(..., "wb" or "wt"))
	bool Touch(FILE* _File, time_t _PosixTimestamp);

	bool MarkReadOnly(const char* _Path, bool _ReadOnly = true);

	bool MarkHidden(const char* _Path, bool _Hidden = true);

	bool Delete(const char* _Path);

	// Returns true if created, false otherwise. If the path already exists, 
	// oGetLastError will be EEXIST.
	bool CreateFolder(const char* _Path);

	// Returns true if the specified path exist
	bool Exists(const char* _Path);

	// Returns true if the specified file is a text file using the criteria found
	// in Perl, that is if a block in the beginning of the file contains less 
	// than a certain percentage of bytes that are 128 < byte < 0.
	bool IsText(FILE* _File);
	bool IsText(const char* _Path);

	inline bool IsBinary(FILE* _File) { return !IsText(_File); }
	inline bool IsBinary(const char* _Path) { return !IsText(_Path); }
};

// Uses the specified allocator to allocate _OutBuffer and fill it with the 
// entire contents of the file specified by _Path. Returns false if there was 
// an error.
bool oLoadBuffer(void** _ppOutBuffer, size_t* _pOutSize, void* (*_Allocate)(size_t _Size), const char* _Path, bool _AsText);

// Loads into a pre-allocated buffer. This fails with an oGetLastError() of 
// EINVAL if the buffer is too small to contain the contents of the file.
bool oLoadBuffer(void* _pOutBuffer, size_t _SizeofOutBuffer, size_t* _pOutSize, const char* _Path, bool _AsText);
template<typename T, size_t size> inline bool oLoadBuffer(T (&_pOutBuffer)[size], size_t* _pOutSize, const char* _Path, bool _AsText) { return oLoadBuffer(_pOutBuffer, size, _pOutSize, _Path, _AsText); }

bool oSaveBuffer(const char* _Path, const void* _pSource, size_t _SizeofSource, bool _AsText);

bool oSetEnvironmentVariable(const char* _Name, const char* _Value);
bool oGetEnvironmentVariable(char* _Value, size_t _SizeofValue, const char* _Name);
template<size_t size> inline bool oGetEnvironmentVariable(char (&_Value)[size], const char* _Name) { return oGetEnvironmentVariable(_Value, size, _Name); }

// Fills _StrEnvironment with all environment variables delimited by '\n'
errno_t oGetEnvironmentString(char* _StrEnvironment, size_t _SizeofStrEnvironment);
template<size_t size> inline errno_t oGetEnvironmentString(char (&_StrEnvironment)[size]) { return oGetEnvironmentString(_pEnvironment, size); }

// returns 0 if successful, or STRUNCATE if truncation occurred
int oGetNativeErrorDesc(char* _StrDestination, size_t _SizeofStrDestination, size_t _NativeError);
template<size_t size> inline int oGetNativeErrorDesc(char (&_StrDestination)[size], size_t _NativeError) { return oGetNativeErrorDesc(_StrDestination, size, _NativeError); }

// _____________________________________________________________________________
// Dynamic library loading

oDECLARE_HANDLE(oHDLL);

// Give this a lib to load, and a list of strings of functions to find, and this
// will return a handle to the lib and a list of resolved interfaces.
// If _LibName is specified as a filebase only, this function will prepend the 
// app path and append a build suffix. If .dll is specified as part of the name
// this function will pass the _LibName through unmodified.
oHDLL oLinkDLL(const char* _LibName, const char** _InterfaceFunctionNames, void** _ppInterfaces, size_t _CountofInterfaces);

// Ensure ALL interfaces/memory/pointers from the DLL are unloaded by this point,
// otherwise you'll get what looks like a valid object, but a bad virtual 
// dereference or some such because the actual vtable itself is no longer loaded.
void oUnlinkDLL(oHDLL _hDLL);

#endif
