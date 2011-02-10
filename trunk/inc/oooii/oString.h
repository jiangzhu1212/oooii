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
#ifndef oString_h
#define oString_h

#include <oooii/oStddef.h>
#include <ctype.h> // isalnum

#define oNEWLINE "\r\n"
#define oWHITESPACE " \t\v\f" oNEWLINE

// _____________________________________________________________________________
// String cleanup

// Convert all characters of the specified string to lowercase (in-place)
void oToLower(char* _String);

// Convert all characters of the specified string to uppercase (in-place)
void oToUpper(char* _String);

// Convert \n -> \r\n
char* oNewlinesToDos(char* _StrDestination, size_t _SizeofStrDestination, const char* _StrSource);

// Remove all chars found in _ToTrim from the beginning of the string. _Trimmed 
// can be the same as _StrSource. Returns _Trimmed.
char* oTrimLeft(char* _Trimmed, size_t _SizeofTrimmed, const char* _StrSource, const char* _ToTrim = oWHITESPACE);

// Remove all chars found in _ToTrim form the end of the string. strDestination can be the same as strSource. Returns dst.
char* oTrimRight(char* _Trimmed, size_t _SizeofTrimmed, const char* _StrSource, const char* _ToTrim = oWHITESPACE);

// Trims both the left and right side of a string
inline char* oTrim(char* _Trimmed, size_t _SizeofTrimmed, const char* _StrSource, const char* _ToTrim = oWHITESPACE) { return oTrimRight(_Trimmed, _SizeofTrimmed, oTrimLeft(_Trimmed, _SizeofTrimmed, _StrSource, _ToTrim), _ToTrim); }

// Replaces any run of whitespace with a single ' ' character. Returns _StrDestination
char* oPruneWhitespace(char* _StrDestination, size_t _SizeofStrDestination, const char* _StrSource, char _Replacement = ' ', const char* _ToPrune = oWHITESPACE);

// Converts a unicode 16-bit string to its 8-bit equivalent. If 
// _SizeofMultiByteString is 0, this returns the required buffer size to hold 
// the conversion (includes room for the nul terminator). Otherwise, this 
// returns the length of the string written (not including the nul terminator).
size_t oStrConvert(char* _MultiByteString, size_t _SizeofMultiByteString, const wchar_t* _StrUnicodeSource);

// Converts a multi-byte 8-bit string to its 16-bit unicode equivalent. If 
// _NumberOfCharactersInUnicodeString is 0, this returns the NUMBER OF CHARACTERS,
// (not the number of bytes) required for the destination bufferm including the 
// nul terminator. Otherwise, this returns the length of the string written.
size_t oStrConvert(wchar_t* _UnicodeString, size_t _NumberOfCharactersInUnicodeString, const char* _StrMultibyteSource);

// replace all occurrences of strFind in strSource with strReplace and copy the result to strDestination
errno_t oReplace(char* _StrResult, size_t _SizeofStrResult, const char* _StrSource, const char* _StrFind, const char* _StrReplace);

// Insert one string into another in-place. _InsertionPoint must point into 
// _StrSource If _ReplacementLength is non-zero, then that number of characters 
// from _InsertionPoint on will be overwritten by the _Insertion.
errno_t oInsert(char* _StrSource, size_t _SizeofStrResult, char* _InsertionPoint, size_t _ReplacementLength, const char* _Insertion);

// Essentially a variadic strcat_s
void oStrAppend( char* string, size_t sizeBuffer, const char* format, ... );

// Returns the appropriate suffix [st nd rd th] for a number
const char* oOrdinal(int _Number);

// Fills the specified buffer with a size in either bytes, KB, MB, GB, or TB 
// depending on the number of bytes specified.
errno_t oFormatMemorySize(char* _StrDestination, size_t _SizeofStrDestination, unsigned long long _NumBytes, size_t _NumPrecisionDigits);

// Fills the specified buffer with a size in days hours minutes seconds
errno_t oFormatTimeSize(char* _StrDestination, size_t _SizeofStrDestination, double _TimeInSeconds);

// For numbers, this inserts commas where they ought to be (every 3 numbers)
errno_t oFormatCommas(char* _StrDestination, size_t _SizeofStrDestination, int _Number);

// Returns the nul-terminated string version of a fourcc code
char* oConvertFourcc(char* _StrDestination, size_t _SizeofStrDestination, int _Fourcc);

// _____________________________________________________________________________
// C++ Parsing (might be useful for other languages too)

// returns true for [A-Za-z0-9_]
inline bool oIsCppID(char c) { return isalnum(c) || c == '_'; }

// Move to the next id character, or one of the stop chars, whichever is first
const char* oMoveToNextID(const char* _pCurrent, const char* _Stop = "");
char* oMoveToNextID(char* _pCurrent, const char* _Stop = "");

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
// Returns _StrSourceCode or NULL if there's a failure (check oGetLastError()).
char* oZeroIfdefs(char* _StrSourceCode, const oMACRO* _pMacros, char _Replacement = ' ');

// Walks through from start counting lines up until the specified line.
size_t oGetLineNumber(const char* _Start, const char* _Line);

// some compilers return the full name (struct foo, or class bar). This function
// returns simply the identifier from that.
const char* oGetSimpleTypename(const char* _TypeinfoName);

// Given the string that is returned from typeid(someStdVec).name(), return the 
// string that represents the typename held in the vector. Returns dst
char* oGetStdVectorType(char* _StrDestination, size_t _SizeofStrDestination, const char* _TypeinfoName);

// Fills strDestination with the file name of the next found include path
// context should be the address of the pointer to a string of C++ source
// code, and it will be updated to point just after the found header. This 
// returns false, when there are no more matches.
bool oGetNextInclude(char* _StrDestination, size_t _SizeofStrDestination, const char** _ppContext);

// Given a buffer of source that uses #include statements, replace those 
// statements with the contents of the specified include files by using
// a user callback. The buffer must be large enough to accommodate all 
// merged includes. This can return EIO, meaning there was a problem in the 
// specified Load function or EINVAL if the specified buffer is too small to 
// hold the results of the merge.
errno_t oMergeIncludes(char* _StrSourceCode, size_t _SizeofStrSourceCode, const char* _SourceFullPath, oLOAD_BUFFER_FUNCTION _Load);

// Convert a buffer into a C++ array. This is useful when you want to embed data 
// in code itself. This fills the destination string with a declaration of the 
// form:
// const <specifiedType> <specifiedName>[] = { <buffer data> };
// This also defines a function of the form:
// void GetDesc<bufferName>(const char** ppBufferName, const void** ppBuffer, size_t* pSize)
// that can be externed and used to access the buffer. Any extension '.' in the
// specified bufferName will be replaced with '_', so GetDescMyFile_txt(...)

size_t oCodifyData(char* _StrDestination, size_t _SizeofStrDestination, const char* _BufferName, const void* _pBuffer, size_t _SizeofBuffer, size_t _WordSize);

bool oGetKeyValuePair(char* _KeyDestination, size_t _SizeofKeyDestination, char* _ValueDestination, size_t _SizeofValueDestination, char _KeyValueSeparator, const char* _KeyValuePairSeparators, const char* _SourceString, const char** _ppLeftOff = 0);

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
bool oHeadersAreUpToDate(const char* _StrSourceCode, const char* _SourceFullPath, const oMACRO* _pMacros, oPATH_EXISTS_FUNCTION _PathExists, oFUNCTION<time_t(const char* _Path)> _GetModifiedDate, oLOAD_BUFFER_FUNCTION _LoadHeaderFile, const char* _HeaderSearchPath);

// _____________________________________________________________________________
// String tokenization

// Like strtok_s, except you can additionally specify open and close scope chars 
// to ignore. For example:
// char* ctx;
// const char* s = "myparams1(0, 1), myparams2(2, 3), myparams3(4, 5)"
// const char* token = oStrTok(s, ",", &ctx, "(", ")");
// while (token)
// {
//		token = oStrTok(0, ",", &ctx, "(", ")");
//		printf("%s\n", token);
// }
// Yields:
// myparams1(0, 1)
// myparams2(2, 3)
// myparams3(4, 5)
char* oStrTok(const char* _Token, const char* _Delimiter, char** _ppContext, const char* _OpenScopeChars = "", const char* _CloseScopeChars = "");

// Use this to clean up a oStrTok iteration if iterating through all tokens is 
// unnecessary. This is automatically called when oStrTok returns 0.
void oCloseStrTok(char** _ppContext);

// Open and close pairings might be mismatched, in which case oStrTok will 
// return 0 early, call oCloseStrTok automatically, but leave the context in a 
// state that can be queried with this function. The context is not really valid 
// (i.e. all memory has been freed).
bool oStrTokFinishedSuccessfully(char** _ppContext);

// _____________________________________________________________________________
// Command-line parsing

struct oOption
{
	const char* LongName;
	char ShortName;
	const char* ArgumentName;
	const char* Description;
};

// Similar to strtok, first call should specify argv and argc from main(), and
// subsequent calls should pass 0 to those values.
// This searches through the specified command line arguments and matches either 
// "--LongName" or "-ShortName" and fills *value with the value for the arg (or 
// if ArgumentName is NULL, meaning there is no arg, then *value gets the option 
// itself, or NULL if the options does not exist.)
// returns:
// if there is a match, the ShortName of the match
// 0 for no more matches
// ' ' for regular arguments (non-option values)
// '?' for an unrecognized option
// ':' if there was a missing option value
// to move through an entire argv list, each iteration should do argv++, argc--;
// options specified in an array somewhere must be terminated with an extra
// CMDLINE_OPTION entry that is all NULL/0 values.
char oOptTok(const char** _ppValue, int _Argc, const char* _Argv[], const oOption* _pOptions);

// Prints documentation for the specified options to the specified buffer.
char* oOptDoc(char* _StrDestination, size_t _SizeofStrDestination, const char* _AppName, const oOption* _pOptions);

// _____________________________________________________________________________
// Type to string and string to type functions. The following types are supported:

// Currently the following are supported. If more is desired, create new
// typed template<> implementations of this declaration.
//
// C++ types, including:
// bool
// (unsigned) char
// (unsigned) short
// (unsigned) long
// (unsigned) int
// (unsigned) long long
// float, double
//
// and also 
// half, oColor
// The user can define additional types by defining typed templates.

template<typename T> errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const T& _Value);
template<typename T> errno_t oFromString(T* _pValue, const char* _StrSource);

// _____________________________________________________________________________
// Templated-on-size versions of the above API

template<size_t size> inline char* oNewlinesToDos(char (&_StrDestination)[size], const char* _StrSource) { return oNewlinesToDos(_StrDestination, size, _StrSource); };
template<size_t size> inline char* oTrimLeft(char (&_Trimmed)[size], const char* _StrSource, const char* _ToTrim = oWHITESPACE) { return oTrimLeft(_Trimmed, size, _StrSource, _ToTrim); }
template<size_t size> inline char* oTrimRight(char (&_Trimmed)[size], const char* _StrSource, const char* _ToTrim = oWHITESPACE) { return oTrimRight(_Trimmed, size, _StrSource, _ToTrim); }
template<size_t size> inline char* oTrim(char (&_Trimmed)[size], const char* _StrSource, const char* _ToTrim = oWHITESPACE) { return oTrim(_Trimmed, size, _StrSource, _ToTrim); }
template<size_t size> inline char* oPruneWhitespace(char (&_StrDestination)[size], const char* _StrSource, char _Replacement = ' ', const char* _ToPrune = oWHITESPACE) { return oPruneWhitespace(_StrDestination, size, _StrSource, _Replacement, _ToPrune); }
template<size_t size> inline size_t oStrConvert(char (&_StrDestination)[size], const wchar_t* _StrSource) { return oStrConvert(_StrDestination, size, _StrSource); }
template<size_t size> inline size_t oStrConvert(wchar_t (&_StrDestination)[size], const char* _StrSource) { return oStrConvert(_StrDestination, size, _StrSource); }
template<size_t size> inline errno_t oReplace(char (&_StrResult)[size], const char* _StrSource, const char* _StrFind, const char* _StrReplace) { return oReplace(_StrResult, size, _StrSource, _StrFind, _StrReplace); }
template<size_t size> inline errno_t oInsert(char (&_StrSource)[size], char* _InsertionPoint, size_t _ReplacementLength, const char* _Insertion) { return oInsert(_StrSource, size, _InsertionPoint, _ReplacementLength, _Insertion); }
template<size_t size> inline errno_t oFormatMemorySize(char (&_StrDestination)[size], unsigned long long _NumBytes, size_t _NumPrecisionDigits) { return oFormatMemorySize(_StrDestination, size, _NumBytes, _NumPrecisionDigits); }
template<size_t size> inline errno_t oFormatTimeSize(char (&_StrDestination)[size], double _TimeInSeconds) { return oFormatTimeSize(_StrDestination, size, _TimeInSeconds); }
template<size_t size> inline errno_t oFormatCommas(char (&_StrDestination)[size], int _Number) { return oFormatCommas(_StrDestination, size, _Number); }
template<size_t size> inline char* oConvertFourcc(char (&_StrDestination)[size], int _Fourcc) { return oConvertFourcc(_StrDestination, size, _Fourcc); }
template<size_t size> inline char* oGetStdVectorType(char (&_StrDestination)[size], const char* _TypeinfoName) { return oGetStdVectorType(_StrDestination, size, _TypeinfoName); }
template<size_t size> inline bool oGetNextInclude(char (&_StrDestination)[size], const char** _ppContext) { return oGetNextInclude(_StrDestination, size, _ppContext); }
template<size_t size> inline bool oGetNextMatchingIfdefBlocks(oIFDEF_BLOCK (&_pBlocks)[size], size_t* _pNumValidBlocks, const char* _StrSourceCodeBegin, const char* _StrSourceCodeEnd) { return oGetNextMatchingIfdefBlocks(_pBlocks, size, _pNumValidBlocks, _StrSourceCodeBegin, _StrSourceCodeEnd); }
template<size_t size> inline errno_t oMergeIncludes(char (&_StrSourceCode)[size], const char* _SourceFullPath, oLOAD_BUFFER_FUNCTION _Load, char* _StrErrorMessage, size_t _SizeofStrErrorMessage) { return oMergeIncludes(_StrSourceCode, size, _SourceFullPath, Load, _StrErrorMessage, _SizeofStrErrorMessage); }
template<size_t size> inline errno_t oMergeIncludes(char* _StrSourceCode, size_t _SizeofStrSourceCode, const char* _SourceFullPath, oLOAD_BUFFER_FUNCTION Load, char (&_StrErrorMessage)[size]) { return oMergeIncludes(_StrSourceCode, size, _SourceFullPath, Load, _StrErrorMessage, size); }
template<size_t size, size_t errSize> inline errno_t oMergeIncludes(char (&_StrSourceCode)[size], const char* _SourceFullPath, oLOAD_BUFFER_FUNCTION Load, char (&_StrErrorMessage)[errSize]) { return oMergeIncludes(_StrSourceCode, size, _SourceFullPath, Load, _StrErrorMessage, errSize); }
template<size_t size> inline size_t oCodifyData(char (&_StrDestination)[size], const char* _BufferName, const void* _pBuffer, size_t _SizeofBuffer, size_t _WordSize) { return oCodifyData(_StrDestination, size, _BufferName, pBuffer, _SizeofBuffer, _WordSize); }
template<size_t size> char* oOptDoc(char (&_StrDestination)[size], const char* _AppName, const oOption* _pOptions) { return oOptDoc(_StrDestination, size, _AppName, _pOptions); }
template<size_t size, typename T> errno_t oToString(char (&_StrDestination)[size], const T& _Value) { return oToString(_StrDestination, size, _Value); }
template<size_t size> inline bool oGetKeyValuePair(char (&_KeyDestination)[size], char* _ValueDestination, size_t _SizeofValueDestination, char _KeyValueSeparator, const char* _KeyValuePairSeparators, const char* _SourceString, const char** _ppLeftOff = 0) { return oGetKeyValuePair(_KeyDestination, size, _ValueDestination, _SizeofValueDestination, _KeyValueSeparator, _KeyValuePairSeparators, _SourceString, _ppLeftOff); }
template<size_t size> inline bool oGetKeyValuePair(char* _KeyDestination, size_t _SizeofKeyDestination, char (&_ValueDestination)[size], char _KeyValueSeparator, const char* _KeyValuePairSeparators, const char* _SourceString, const char** _ppLeftOff = 0) { return oGetKeyValuePair(_KeyDestination, _SizeofKeyDestination, _ValueDestination, size, _KeyValueSeparator, _KeyValuePairSeparators, _SourceString, _ppLeftOff); }
template<size_t key_size, size_t value_size> inline bool oGetKeyValuePair(char (&_KeyDestination)[key_size], char (&_ValueDestination)[value_size], const char* _KeyValueSeparator, const char* _KeyValuePairSeparators, const char* _SourceString, const char** _ppLeftOff = 0) { return oGetKeyValuePair(_KeyDestination, key_size, _ValueDestination, value_size, _KeyValueSeparator, _KeyValuePairSeparators, _SourceString, _ppLeftOff); }

#endif