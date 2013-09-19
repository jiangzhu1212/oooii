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
#pragma once
#ifndef oString_h
#define oString_h

#include <oStd/fixed_string.h>
#include <oStd/function.h>
#include <oStd/macros.h>
#include <oBasis/oPlatformFeatures.h>
#include <oBasis/oInvalid.h>
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

//Unlike oStrcat that fails if strDestination is not large enough, these functions will fill as much as possible of the destination and then return
char* oStrncat(char* _StrDestination, size_t _NumDestinationChars, const char* _Source);
wchar_t* oStrncat(wchar_t* _StrDestination, size_t _NumDestinationChars, const wchar_t* _Source);

// Like strstr however a maximum number of characters to search can be specified
const char* oStrStr(const char* _pStr, const char* _pSubStr, size_t _MaxCharCount = oInvalid);

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

// Essentially a variadic oStrcat
errno_t oStrVAppendf(char* _StrDestination, size_t _SizeofStrDestination, const char* _Format, va_list _Args);
inline errno_t oStrAppendf(char* _StrDestination, size_t _SizeofStrDestination, const char* _Format, ...) { va_list args; va_start(args, _Format); errno_t err = oStrVAppendf(_StrDestination, _SizeofStrDestination, _Format, args); va_end(args); return err; }

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

// This is a helper function to skip _Count number of tokens delimited by 
// _pDelimiters in _pString
const char* oStrTokSkip(const char* _pString, const char* _pDelimiters, int _Count, bool _SkipDelimiters=true);

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
template<typename CHAR_T, size_t numchars> CHAR_T* oStrncat(CHAR_T (&_StrDestination)[numchars], const CHAR_T* _StrSource) { return oStrncat(_StrDestination, numchars, _StrSource); }
template<typename CHAR_T, size_t numchars> int oVPrintf(CHAR_T (&_StrDestination)[numchars], const CHAR_T* _Format, va_list _Args) { return oVPrintf(_StrDestination, numchars, _Format, _Args); }
template<typename CHAR_T, size_t numchars> int oPrintf(CHAR_T (&_StrDestination)[numchars], const CHAR_T* _Format, ...) { va_list args; va_start(args, _Format); int n = oVPrintf(_StrDestination, numchars, _Format, args); va_end(args); return n; }
template<size_t size> char* oNewlinesToDos(char (&_StrDestination)[size], const char* _StrSource) { return oNewlinesToDos(_StrDestination, size, _StrSource); }
template<size_t size> char* oPruneWhitespace(char (&_StrDestination)[size], const char* _StrSource, char _Replacement = ' ', const char* _ToPrune = oWHITESPACE) { return oPruneWhitespace(_StrDestination, size, _StrSource, _Replacement, _ToPrune); }
template<size_t size> errno_t oStrVAppendf(char (&_StrDestination)[size], const char* _Format, va_list _Args) { return oStrVAppendf(_StrDestination, size, _Format, _Args); }
template<size_t size> errno_t oStrAppendf(char (&_StrDestination)[size], const char* _Format, ...) { va_list args; va_start(args, _Format); errno_t err = oStrVAppendf(_StrDestination, size, _Format, args); va_end(args); return err; }

// oStd::fixed_string support
template<typename CHAR1_T, typename CHAR2_T, size_t capacity> CHAR1_T* oStrcpy(oStd::fixed_string<CHAR1_T, capacity>& _StrDestination, const CHAR2_T* _StrSource, bool _ZeroBuffer = false) { return oStrcpy(_StrDestination.c_str(), _StrDestination.capacity(), _StrSource, _ZeroBuffer); }
template<typename CHAR1_T, typename CHAR2_T, size_t capacity> CHAR1_T* oStrncpy(oStd::fixed_string<CHAR1_T, capacity>& _StrDestination, const CHAR2_T* _StrSource, size_t _NumChars) { return oStrncpy(_StrDestination.c_str(), _StrDestination.capacity(), _StrSource, _NumChars); }
template<typename CHAR_T, size_t capacity> CHAR_T* oStrcat(oStd::fixed_string<CHAR_T, capacity>& _StrDestination, const CHAR_T* _StrSource) { return oStrcat(_StrDestination.c_str(), _StrDestination.capacity(), _StrSource); }
template<typename CHAR_T, size_t capacity> size_t oStrlen(const oStd::fixed_string<CHAR_T, capacity>& _String) { return oStrlen(_String.c_str()); }
template<typename CHAR_T, size_t capacity> int oVPrintf(oStd::fixed_string<CHAR_T, capacity>& _StrDestination, const CHAR_T* _Format, va_list _Args) { return oVPrintf(_StrDestination.c_str(), _StrDestination.capacity(), _Format, _Args); }
template<typename CHAR_T, size_t capacity> int oPrintf(oStd::fixed_string<CHAR_T, capacity>& _StrDestination, const CHAR_T* _Format, ...) { va_list args; va_start(args, _Format); return oVPrintf(_StrDestination, _Format, args); }
template<size_t capacity> errno_t oStrAppendf(oStd::fixed_string<char, capacity>& _StrDestination, const char* _Format, ...) { va_list args; va_start(args, _Format); errno_t err = oStrVAppendf(_StrDestination.c_str(), _StrDestination.capacity(), _Format, args); va_end(args); return err; }

#endif
