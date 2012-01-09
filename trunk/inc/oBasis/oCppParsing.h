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
// Very basic C++ parsing intended to handle trival cases to 
// build simple compilers such as for reflection systems or 
// pre-process HLSL source before compiling.
#pragma once
#ifndef oCppParsing_h
#define oCppParsing_h

#include <oBasis/oFunction.h>
#include <ctype.h> // isalnum

// returns true for [A-Za-z0-9_]
inline bool oIsCppID(char c) { return isalnum(c) || c == '_'; }

// Move to the next id character, or one of the stop chars, whichever is first
const char* oMoveToNextID(const char* _pCurrent, const char* _Stop = "");
char* oMoveToNextID(char* _pCurrent, const char* _Stop = "");

// Returns the pointer into _TypeinfoName that represents just the name of the 
// user type, thus skipping any prefix [enum|class|struct|union]. This does not
// behave well for built-in types.
const char* oGetTypename(const char* _TypeinfoName);

// first param is assumed to be pointing to the open brace. From there this will 
// find the  brace at the same level of recursion - internal pairs are ignored.
const char* oGetNextMatchingBrace(const char* _pPointingAtOpenBrace, char _CloseBrace);

// Same as above, but multi-char delimiters
const char* oGetNextMatchingBrace(const char* _pPointingAtOpenBrace, const char* _OpenBrace, const char* _CloseBrace);

struct oIFDEF_BLOCK
{
	enum TYPE
	{
		UNKNOWN,
		IFDEF,
		IFNDEF,
		IF,
		ELIF,
		ELSE,
		ENDIF,
	};

	TYPE Type;
	const char* ExpressionStart; // what comes after one of the opening TYPES. NULL for ELSE and ENDIF
	const char* ExpressionEnd;
	const char* BlockStart; // The data within the block
	const char* BlockEnd;
};

// Where *_ppStrSourceCode is pointing into a C++ style source code string, this
// will find the next #ifdef or #ifndef block (currently anything with #if #elif 
// blocks will break this code) and fill the specified array of blocks with where
// the internal strings begin and end. This does so in a way that recursive #if*
// statements are skipped and the list consists only of those at the same level
// as the original #if*. This stops searching beyond _StrSourceCodeEnd, or if
// NULL then to the end of the string. Up to the _MaxNumBlocks is filled in.
// Iterating through the result can be done until a Type is ENDIF, or using an
// index up to *_pNumValidBlocks. The ENDIF node's BlockEnd points immediately 
// after the #endif statement.
bool oGetNextMatchingIfdefBlocks(oIFDEF_BLOCK* _pBlocks, size_t _MaxNumBlocks, size_t* _pNumValidBlocks, const char* _StrSourceCodeBegin, const char* _StrSourceCodeEnd);

// Zeros-out the entire section delimited by the open and close braces, useful
// for getting rid of block comments or #if 0/#endif blocks
char* oZeroSection(char* _pPointingAtOpenBrace, const char* _OpenBrace, const char* _CloseBrace, char _Replacement = ' ');

// Like this very C++ comment! This function replaces the comment with the 
// specified char.
char* oZeroLineComments(char* _String, const char* _CommentPrefix, char _Replacement = ' ');

struct oMACRO
{
	const char* Symbol;
	const char* Value;
};

// This function uses the specified macros to go through and evaluate #if*'s 
// statements using oGetNextMatchingIfdefBlocks (at the time of this writing #if 
// and #elif are not supported) and zero out undefined code.
// The final macro should be { 0, 0 } as a nul terminator.
// Returns _StrSourceCode or NULL if there's a failure (check oErrorGetLast()).
char* oZeroIfdefs(char* _StrSourceCode, const oMACRO* _pMacros, char _Replacement = ' ');

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
// merged includes. If this returns false, oErrorGetLst() will be oERROR_IO to
// indicate a problem in the specified Load function or oERROR_INVALID_PARAMETER 
// if the specified buffer is too small to hold the results of the merge.
bool oMergeIncludes(char* _StrSourceCode, size_t _SizeofStrSourceCode, const char* _SourceFullPath, oFUNCTION_LOAD_BUFFER _Load);

// Convert a buffer into a C++ array. This is useful when you want to embed data 
// in code itself. This fills the destination string with a declaration of the 
// form:
// const <specifiedType> <specifiedName>[] = { <buffer data> };
// This also defines a function of the form:
// void GetDesc<bufferName>(const char** ppBufferName, const void** ppBuffer, size_t* pSize)
// that can be externed and used to access the buffer. Any extension '.' in the
// specified bufferName will be replaced with '_', so GetDescMyFile_txt(...)

size_t oCodifyData(char* _StrDestination, size_t _SizeofStrDestination, const char* _BufferName, const void* _pBuffer, size_t _SizeofBuffer, size_t _WordSize);

// Walk through a C++ style source file and check all #include statements for 
// their date compared to the source file itself. _SourceFullPath is a semi-colon
// delimited list of paths.

// GetModifiedDate() should return the timestamp at which the file at the 
// specified full path was modified, or 0 if the file could not be found.

// NOTE: This function uses the specified _HeaderSearchPath for all headers 
// recursively which may include system headers, so ensure system paths are 
// specified in the search path as well, or optionally special-case certain
// filenames in the functions. For example, you could test for windows.h in each
// function and return true that it exists, return a modifed data of 0x1 so that
// it's a very very old/unchanged file, and LoadHeaderFile loads an empty file so
// that the algorithm doesn't have to recurse. NOTE: The array of macros must
// be NUL-terminated, meaning a value of {0,0} must be the last entry in the 
// oMACRO array.
bool oHeadersAreUpToDate(const char* _StrSourceCode, const char* _SourceFullPath, const oMACRO* _pMacros, oFUNCTION_PATH_EXISTS _PathExists, oFUNCTION<time_t(const char* _Path)> _GetModifiedDate, oFUNCTION_LOAD_BUFFER _LoadHeaderFile, const char* _HeaderSearchPath);

// _____________________________________________________________________________
// Templated-on-size versions of the above API

template<size_t size> inline char* oGetStdVectorType(char (&_StrDestination)[size], const char* _TypeinfoName) { return oGetStdVectorType(_StrDestination, size, _TypeinfoName); }
template<size_t size> inline bool oGetNextInclude(char (&_StrDestination)[size], const char** _ppContext) { return oGetNextInclude(_StrDestination, size, _ppContext); }
template<size_t size> inline bool oGetNextMatchingIfdefBlocks(oIFDEF_BLOCK (&_pBlocks)[size], size_t* _pNumValidBlocks, const char* _StrSourceCodeBegin, const char* _StrSourceCodeEnd) { return oGetNextMatchingIfdefBlocks(_pBlocks, size, _pNumValidBlocks, _StrSourceCodeBegin, _StrSourceCodeEnd); }
template<size_t size> inline errno_t oMergeIncludes(char (&_StrSourceCode)[size], const char* _SourceFullPath, oFUNCTION_LOAD_BUFFER _Load, char* _StrErrorMessage, size_t _SizeofStrErrorMessage) { return oMergeIncludes(_StrSourceCode, size, _SourceFullPath, Load, _StrErrorMessage, _SizeofStrErrorMessage); }
template<size_t size> inline errno_t oMergeIncludes(char* _StrSourceCode, size_t _SizeofStrSourceCode, const char* _SourceFullPath, oFUNCTION_LOAD_BUFFER Load, char (&_StrErrorMessage)[size]) { return oMergeIncludes(_StrSourceCode, size, _SourceFullPath, Load, _StrErrorMessage, size); }
template<size_t size, size_t errSize> inline errno_t oMergeIncludes(char (&_StrSourceCode)[size], const char* _SourceFullPath, oFUNCTION_LOAD_BUFFER Load, char (&_StrErrorMessage)[errSize]) { return oMergeIncludes(_StrSourceCode, size, _SourceFullPath, Load, _StrErrorMessage, errSize); }
template<size_t size> inline size_t oCodifyData(char (&_StrDestination)[size], const char* _BufferName, const void* _pBuffer, size_t _SizeofBuffer, size_t _WordSize) { return oCodifyData(_StrDestination, size, _BufferName, pBuffer, _SizeofBuffer, _WordSize); }

#endif
