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

#include <oBasis/oMacros.h>
#include <oBasis/oPlatformFeatures.h>
#include <oBasis/oFunction.h>
#include <stdarg.h> // va_start
#include <stddef.h> // errno_t

// _____________________________________________________________________________
// Platform-abstractions of string comparisons to support char and wchar_t.
// This also abstracts differences between standard C/C++, Posix, and SecureCRT
// forms. Use these in the code and let abstraction depending on compiler 
// support happen underneath these calls.

// returns the length of a nul-terminated string.
template<typename CHAR_T> size_t oStrlen(const CHAR_T* _String);

// like strcpy, but includes a limit check on the destination and can handle
// wchar_t <-> char conversions. This return nullptr if there's an error, such 
// as not having a large enough destination buffer. _NumDestinationChars is a 
// COUNTOF operation, not a SIZEOF operation, so the number does not change due 
// to the type of CHAR1_T. An additional option will zero the remaining buffer.
// This is especially useful when serializing a string buffer to disk to ensure
// the balance of the buffer is always a predictable zero and not some debug vs. 
// release shred pattern.
char* oStrcpy(char* _StrDestination, size_t _NumDestinationChars, const char* _StrSource, bool _ZeroBuffer = false);
char* oStrcpy(char* _StrDestination, size_t _NumDestinationChars, const wchar_t* _StrSource, bool _ZeroBuffer = false);
wchar_t* oStrcpy(wchar_t* _StrDestination, size_t _NumDestinationChars, const wchar_t* _StrSource, bool _ZeroBuffer = false);
wchar_t* oStrcpy(wchar_t* _StrDestination, size_t _NumDestinationChars, const char* _StrSource, bool _ZeroBuffer = false);

char* oStrncpy(char* _StrDestination, size_t _NumDestinationChars, const char* _StrSource, size_t _NumChars);
wchar_t* oStrncpy(wchar_t* _StrDestination, size_t _NumDestinationChars, const wchar_t* _StrSource, size_t _NumChars);

int oStrcmp(const char* _Str1, const char* _Str2);
int oStrcmp(const wchar_t* _Str1, const wchar_t* _Str2);

int oStricmp(const char* _Str1, const char* _Str2);
int oStricmp(const wchar_t* _Str1, const wchar_t* _Str2);

int oStrncmp(const char* _Str1, const char* _Str2, size_t _NumChars);
int oStrncmp(const wchar_t* _Str1, const wchar_t* _Str2, size_t _NumChars);

int oStrnicmp(const char* _Str1, const char* _Str2, size_t _NumChars);
int oStrnicmp(const wchar_t* _Str1, const wchar_t* _Str2, size_t _NumChars);

char* oStrcat(char* _StrDestination, size_t _NumDestinationChars, const char* _Source);
wchar_t* oStrcat(wchar_t* _StrDestination, size_t _NumDestinationChars, const wchar_t* _Source);

// this encapsulates snprintf-like functionality. Generally this is like printf
// to the specified buffer (sprintf), but will truncate with an elipse (...) at 
// the end. This will protect against buffer overflows. Favor this than any of 
// the C flavors since some aren't on every platform and others don't protect
// against overflow. This returns the number of chars written, or -1 if a 
// truncation occurred.
template<typename CHAR_T> int oVPrintf(CHAR_T* _StrDestination, size_t _NumDestinationChars, const CHAR_T* _Format, va_list _Args);
template<typename CHAR_T> int oPrintf(CHAR_T* _StrDestination, size_t _NumDestinationChars, const CHAR_T* _Format, ...) { va_list args; va_start(args, _Format); int n = oVPrintf(_StrDestination, _NumDestinationChars, _Format, args); va_end(args); return n; }

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

// Overwrites the specified buffer's last 4 bytes of capacity with "...\0" so if
// some strcpy were truncated, this would add a bit more visual sugar that the
// truncation took place.
template<typename CHAR_T> CHAR_T* oAddTruncationElipse(CHAR_T* _StrDestination, size_t _NumDestinationChars);

// replace all occurrences of strFind in strSource with strReplace and copy the result to strDestination
errno_t oReplace(char* oRESTRICT _StrResult, size_t _SizeofStrResult, const char* oRESTRICT _StrSource, const char* _StrFind, const char* _StrReplace);

// Search from the back of the string to find the specified substring
const char* oStrStrReverse(const char* _Str, const char* _SubStr);
char* oStrStrReverse(char* _Str, const char* _SubStr);

// Insert one string into another in-place. _InsertionPoint must point into 
// _StrSource If _ReplacementLength is non-zero, then that number of characters 
// from _InsertionPoint on will be overwritten by the _Insertion. This returns
// the position immediately after the new insertion. If there isn't enough room
// in the buffer, then nullptr is returned.
char* oInsert(char* _StrSource, size_t _SizeofStrSource, char* _InsertionPoint, size_t _ReplacementLength, const char* _Insertion);

// Essentially a variadic oStrcat
errno_t oStrVAppendf(char* _StrDestination, size_t _SizeofStrDestination, const char* _Format, va_list _Args);
inline errno_t oStrAppendf(char* _StrDestination, size_t _SizeofStrDestination, const char* _Format, ...) { va_list args; va_start(args, _Format); errno_t err = oStrVAppendf(_StrDestination, _SizeofStrDestination, _Format, args); va_end(args); return err; }

// Returns the appropriate suffix [st nd rd th] for a number
const char* oOrdinal(int _Number);

// Fills the specified buffer with a size in either bytes, KB, MB, GB, or TB 
// depending on the number of bytes specified.
char* oFormatMemorySize(char* _StrDestination, size_t _SizeofStrDestination, unsigned long long _NumBytes, size_t _NumPrecisionDigits);

// Fills the specified buffer with a size in days hours minutes seconds
char* oFormatTimeSize(char* _StrDestination, size_t _SizeofStrDestination, double _TimeInSeconds, bool _Abbreviated = false, bool _IncludeMS = true);

// For numbers, this inserts commas where they ought to be (every 3 numbers)
char* oFormatCommas(char* _StrDestination, size_t _SizeofStrDestination, int _Number);
char* oFormatCommas(char* _StrDestination, size_t _SizeofStrDestination, unsigned int _Number);

// Returns the nul-terminated string version of a fourcc code
char* oConvertFourcc(char* _StrDestination, size_t _SizeofStrDestination, int _Fourcc);

// Copies out the next key/value pair as delimited by _KeyValuePairSeparators and 
// updates where the parsing left off so that this can be called to go through a
// large set of key/value pairs.
bool oGetKeyValuePair(char* _KeyDestination, size_t _SizeofKeyDestination, char* _ValueDestination, size_t _SizeofValueDestination, char _KeyValueSeparator, const char* _KeyValuePairSeparators, const char* _SourceString, const char** _ppLeftOff = 0);

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
void oStrTokClose(char** _ppContext);

// Open and close pairings might be mismatched, in which case oStrTok will 
// return 0 early, call oStrTokClose automatically, but leave the context in a 
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
// subsequent calls should pass nullptr to those values.
// This searches through the specified command line arguments and matches either 
// "--LongName" or "-ShortName" and fills *value with the value for the arg (or 
// if ArgumentName is nullptr, meaning there is no arg, then *value gets the 
// option itself, or nullptr if the options does not exist.)
// returns:
// if there is a match, the ShortName of the match
// 0 for no more matches
// ' ' for regular arguments (non-option values)
// '?' for an unrecognized option
// ':' if there was a missing option value
// to move through an entire argv list, each iteration should do argv++, argc--;
// options specified in an array somewhere must be terminated with an extra
// CMDLINE_OPTION entry that is all nullptr/0 values.
char oOptTok(const char** _ppValue, int _Argc, const char* _Argv[], const oOption* _pOptions);

// Prints documentation for the specified options to the specified buffer.
char* oOptDoc(char* _StrDestination, size_t _SizeofStrDestination, const char* _AppName, const oOption* _pOptions);

// _____________________________________________________________________________
// Misc

// Come up with every combination of every option specified. _ppOptions is an  
// array of options lists. Each options list is an array of strings naming each 
// option. _pSizes is the number of options in each options list. Given all 
// that, this code will generate in the form of "opt1a|opt2a|opt3a" all 
// combinations of the specified strings. This uses a 4k internal string buffer
// that will be truncated if the permutation string gets too long.
void oPermutate(const char*** _ppOptions, size_t* _pSizes, int _NumOptions, oFUNCTION<void(const char* _Permutation)> _Visitor, const char* _Delim = "|");

//simple string parsing. easier to use that oStrTok. splits string by _Delimiter, calling back _Enumerator for each parsed string.
void oStrParse(const char* _StrSource, const char* _Delimiter, oFUNCTION<void(const char* _Value)> _Enumerator);

//Count the number of occurrences of _CountChar in _StrSource
int oStrCCount(const char* _StrSource, char _CountChar);

//returns index of first character that is different between the 2 strings. If strings are identical, then returns -1
int oStrFindFirstDiff(const char* _StrSource1, const char* _StrSource2);

// _____________________________________________________________________________
// Templated-on-size versions of the above API

template<typename CHAR1_T, typename CHAR2_T, size_t size> CHAR1_T* oStrcpy(CHAR1_T (&_StrDestination)[size], const CHAR2_T* _StrSource, bool _ZeroBuffer = false) { return oStrcpy(_StrDestination, size, _StrSource, _ZeroBuffer); }
template<typename CHAR1_T, typename CHAR2_T, size_t size> CHAR1_T* oStrncpy(CHAR1_T (&_StrDestination)[size], const CHAR2_T* _StrSource, size_t _NumChars) { return oStrncpy(_StrDestination, size, _StrSource, _NumChars); }
template<typename CHAR_T, size_t numchars> CHAR_T* oStrcat(CHAR_T (&_StrDestination)[numchars], const CHAR_T* _StrSource) { return oStrcat(_StrDestination, numchars, _StrSource); }
template<typename CHAR_T, size_t numchars> int oVPrintf(CHAR_T (&_StrDestination)[numchars], const CHAR_T* _Format, va_list _Args) { return oVPrintf(_StrDestination, numchars, _Format, _Args); }
template<typename CHAR_T, size_t numchars> int oPrintf(CHAR_T (&_StrDestination)[numchars], const CHAR_T* _Format, ...) { va_list args; va_start(args, _Format); int n = oVPrintf(_StrDestination, numchars, _Format, args); va_end(args); return n; }
template<size_t size> char* oNewlinesToDos(char (&_StrDestination)[size], const char* _StrSource) { return oNewlinesToDos(_StrDestination, size, _StrSource); }
template<size_t size> char* oTrimLeft(char (&_Trimmed)[size], const char* _StrSource, const char* _ToTrim = oWHITESPACE) { return oTrimLeft(_Trimmed, size, _StrSource, _ToTrim); }
template<size_t size> char* oTrimRight(char (&_Trimmed)[size], const char* _StrSource, const char* _ToTrim = oWHITESPACE) { return oTrimRight(_Trimmed, size, _StrSource, _ToTrim); }
template<size_t size> char* oTrim(char (&_Trimmed)[size], const char* _StrSource, const char* _ToTrim = oWHITESPACE) { return oTrim(_Trimmed, size, _StrSource, _ToTrim); }
template<size_t size> char* oPruneWhitespace(char (&_StrDestination)[size], const char* _StrSource, char _Replacement = ' ', const char* _ToPrune = oWHITESPACE) { return oPruneWhitespace(_StrDestination, size, _StrSource, _Replacement, _ToPrune); }
template<typename CHAR_T, size_t numchars> CHAR_T* oAddTruncationElipse(CHAR_T (&_StrDestination)[numchars]) { return oAddTruncationElipse(_StrDestination, numchars); }
template<size_t size> errno_t oReplace(char (&_StrResult)[size], const char* _StrSource, const char* _StrFind, const char* _StrReplace) { return oReplace(_StrResult, size, _StrSource, _StrFind, _StrReplace); }
template<size_t size> char* oInsert(char (&_StrSource)[size], char* _InsertionPoint, size_t _ReplacementLength, const char* _Insertion) { return oInsert(_StrSource, size, _InsertionPoint, _ReplacementLength, _Insertion); }
template<size_t size> errno_t oStrVAppendf(char (&_StrDestination)[size], const char* _Format, va_list _Args) { return oStrVAppendf(_StrDestination, size, _Format, _Args); }
template<size_t size> errno_t oStrAppendf(char (&_StrDestination)[size], const char* _Format, ...) { va_list args; va_start(args, _Format); errno_t err = oStrVAppendf(_StrDestination, size, _Format, args); va_end(args); return err; }
template<size_t size> char* oFormatMemorySize(char (&_StrDestination)[size], unsigned long long _NumBytes, size_t _NumPrecisionDigits) { return oFormatMemorySize(_StrDestination, size, _NumBytes, _NumPrecisionDigits); }
template<size_t size> char* oFormatTimeSize(char (&_StrDestination)[size], double _TimeInSeconds, bool _Abbreviated = false, bool _IncludeMS = true) { return oFormatTimeSize(_StrDestination, size, _TimeInSeconds, _Abbreviated, _IncludeMS ); }
template<size_t size> char* oFormatCommas(char (&_StrDestination)[size], int _Number) { return oFormatCommas(_StrDestination, size, _Number); }
template<size_t size> char* oFormatCommas(char (&_StrDestination)[size], unsigned int _Number) { return oFormatCommas(_StrDestination, size, _Number); }
template<size_t size> char* oConvertFourcc(char (&_StrDestination)[size], int _Fourcc) { return oConvertFourcc(_StrDestination, size, _Fourcc); }
template<size_t size> char* oOptDoc(char (&_StrDestination)[size], const char* _AppName, const oOption* _pOptions) { return oOptDoc(_StrDestination, size, _AppName, _pOptions); }
template<size_t size> bool oGetKeyValuePair(char (&_KeyDestination)[size], char* _ValueDestination, size_t _SizeofValueDestination, char _KeyValueSeparator, const char* _KeyValuePairSeparators, const char* _SourceString, const char** _ppLeftOff = 0) { return oGetKeyValuePair(_KeyDestination, size, _ValueDestination, _SizeofValueDestination, _KeyValueSeparator, _KeyValuePairSeparators, _SourceString, _ppLeftOff); }
template<size_t size> bool oGetKeyValuePair(char* _KeyDestination, size_t _SizeofKeyDestination, char (&_ValueDestination)[size], char _KeyValueSeparator, const char* _KeyValuePairSeparators, const char* _SourceString, const char** _ppLeftOff = 0) { return oGetKeyValuePair(_KeyDestination, _SizeofKeyDestination, _ValueDestination, size, _KeyValueSeparator, _KeyValuePairSeparators, _SourceString, _ppLeftOff); }
template<size_t key_size, size_t value_size> bool oGetKeyValuePair(char (&_KeyDestination)[key_size], char (&_ValueDestination)[value_size], const char* _KeyValueSeparator, const char* _KeyValuePairSeparators, const char* _SourceString, const char** _ppLeftOff = 0) { return oGetKeyValuePair(_KeyDestination, key_size, _ValueDestination, value_size, _KeyValueSeparator, _KeyValuePairSeparators, _SourceString, _ppLeftOff); }

#endif
