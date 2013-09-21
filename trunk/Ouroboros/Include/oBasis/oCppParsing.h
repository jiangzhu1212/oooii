/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
// Very basic C++ parsing intended to handle trival cases to 
// build simple compilers such as for reflection systems or 
// pre-process HLSL source before compiling.
#pragma once
#ifndef oCppParsing_h
#define oCppParsing_h

#include <oStd/function.h>
#include <ctype.h> // isalnum

// returns true for [A-Za-z0-9_]
inline bool oIsCppID(char c) { return isalnum(c) || c == '_'; }

// Move to the next id character, or one of the stop chars, whichever is first
const char* oMoveToNextID(const char* _pCurrent, const char* _Stop = "");
char* oMoveToNextID(char* _pCurrent, const char* _Stop = "");

// Walks through from start counting lines up until the specified line.
size_t oGetLineNumber(const char* _Start, const char* _Line);

// Given the string that is returned from typeid(someStdVec).name(), return the 
// string that represents the typename held in the vector. Returns dst
char* oGetStdVectorType(char* _StrDestination, size_t _SizeofStrDestination, const char* _TypeinfoName);

// Returns true if the specified symbol string is a symbol/template 
// instantiation used inside of std::bind. This is useful for filtering out 
// symbols such as from a call stack where this boiler-plate detail is more 
// noise that useful, so the user could instead print out a ... std::bind ...
// rather than the 8-10 implementation detail function calls.
bool oIsStdBindImplementationDetail(const char* _Symbol);

// Fills strDestination with the file name of the next found include path
// context should be the address of the pointer to a string of C++ source
// code, and it will be updated to point just after the found header. This 
// returns false, when there are no more matches.
bool oGetNextInclude(char* _StrDestination, size_t _SizeofStrDestination, const char** _ppContext);

// Given a buffer of source that uses #include statements, replace those 
// statements with the contents of the specified include files by using
// a user callback. The buffer must be large enough to accommodate all 
// merged includes. If this returns false, oErrorGetLst() will be std::errc::io_error to
// indicate a problem in the specified Load function or std::errc::invalid_argument 
// if the specified buffer is too small to hold the results of the merge.
bool oMergeIncludes(char* _StrSourceCode, size_t _SizeofStrSourceCode, const char* _SourceFullPath, const oFUNCTION<bool(void* _pDestination, size_t _SizeofDestination, const char* _Path)>& _Load);

// _____________________________________________________________________________
// Templated-on-size versions of the above API

template<size_t size> inline char* oGetStdVectorType(char (&_StrDestination)[size], const char* _TypeinfoName) { return oGetStdVectorType(_StrDestination, size, _TypeinfoName); }
template<size_t size> inline bool oGetNextInclude(char (&_StrDestination)[size], const char** _ppContext) { return oGetNextInclude(_StrDestination, size, _ppContext); }
template<size_t size> inline errno_t oMergeIncludes(char (&_StrSourceCode)[size], const char* _SourceFullPath, const oFUNCTION<bool(void* _pDestination, size_t _SizeofDestination, const char* _Path)>& _Load, char* _StrErrorMessage, size_t _SizeofStrErrorMessage) { return oMergeIncludes(_StrSourceCode, size, _SourceFullPath, Load, _StrErrorMessage, _SizeofStrErrorMessage); }
template<size_t size> inline errno_t oMergeIncludes(char* _StrSourceCode, size_t _SizeofStrSourceCode, const char* _SourceFullPath, const oFUNCTION<bool(void* _pDestination, size_t _SizeofDestination, const char* _Path)>& Load, char (&_StrErrorMessage)[size]) { return oMergeIncludes(_StrSourceCode, size, _SourceFullPath, Load, _StrErrorMessage, size); }
template<size_t size, size_t errSize> inline errno_t oMergeIncludes(char (&_StrSourceCode)[size], const char* _SourceFullPath, const oFUNCTION<bool(void* _pDestination, size_t _SizeofDestination, const char* _Path)>& Load, char (&_StrErrorMessage)[errSize]) { return oMergeIncludes(_StrSourceCode, size, _SourceFullPath, Load, _StrErrorMessage, errSize); }

#endif
