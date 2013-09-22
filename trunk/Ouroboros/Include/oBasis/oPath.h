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
// Code for parsing/working with local file names as found on Linux/Windows
// systems.
#pragma once
#ifndef oPath_h
#define oPath_h

#include <oBasis/oStddef.h>
#include <oBase/fixed_string.h>
#include <oBase/function.h>

// Return pointer to file extension (i.e. the '.' character). If there is no 
// extension, this returns the pointer to the nul terminator of the string.
const char* oGetFileExtension(const char* _Path);
char* oGetFileExtension(char* _Path);

// Check if the file has one of the provided extensions
bool oHasMatchingExtension(const char* _Path, const char* _Extensions[], size_t _NumExtensions);

// Returns true if the specified character is a path separator on either Linux
// or Windows.
inline bool oIsSeparator(int _Char) { return _Char == '\\' || _Char == '/'; }

// Returns true if this is a Windows-style network path (UNC).
inline bool oIsUNCPath(const char* _Path) { return _Path && oIsSeparator(*_Path) && oIsSeparator(*(_Path+1)); }

// Returns true if a Windows-style drive letter followed by a colon is found.
// All other paths (including Linux-style paths) are considered to be relative.
inline bool oIsFullPath(const char* _Path) { return _Path && *_Path && *(_Path+1) == ':'; }

// Returns a pointer into _Path that points at the start of the file base name.
// (Of course the string continues through the extension to the end of the string).
const char* oGetFilebase(const char* _Path);
char* oGetFilebase(char* _Path);

// This copies the file basename into the specified destination and returns a 
// pointer to the destination. This does not include the extension. If the 
// specified buffer is too small or _Path is not valid, then this returns 
// nullptr.
char* oGetFilebase(char* _StrDestination, size_t _SizeofStrDestination, const char* _Path);

// Replaces or appends the specified _Extension to the specified _Path. This
// returns _Path.
char* oReplaceFileExtension(char* _Path, size_t _SizeofPath, const char* _Extension);

// Replaces the specified filename (base+extension) or appends if the last char
// in the string is a separator.
char* oReplaceFilename(char* _Path, size_t _SizeofPath, const char* _Filename);

// Removes the right-most file or dir name
// If _IgnoreTrailingSeparator is true, then C:\foo\ will return C:\ 
// If _IgnoreTrailingSeparator is false, then C:\foo\ will return C:\foo\ (noop) 
char* oTrimFilename(char* _Path, bool _IgnoreTrailingSeparator = false);
char* oTrimFileExtension(char* _Path);
char* oTrimFileLeadingSeperators(char* _Path);

// Ensures a backslash or forward slash is the rightmost char. Returns _Path
char* oEnsureSeparator(char* _Path, size_t _SizeofPath, char _FileSeparator = '/');

// Returns the length of characters from the beginning of the two specified 
// strings where they match. This uses oIsSeparator() so '/' == '\\' for this
// function.
size_t oGetCommonBaseLength(const char* _Path1, const char* _Path2, bool _CaseInsensitive = true);

// Takes a full path, and fills _StrDestination with a path relative to 
// _BasePath. Returns _StrDestination. If ZeroBuffer is true, the rest of the
// buffer after the null-terminator will be set to 0.
char* oMakeRelativePath(char* _StrDestination, size_t _SizeofStrDestination, const char* _FullPath, const char* _BasePath, char _FileSeparator = '/');

// Standard Unix/MS-DOS style wildcard matching
bool oMatchesWildcard(const char* _Wildcard, const char* _Path);

// _SearchPaths is a semi-colon-delimited set of strings (like to Window's PATH
// environment variable). _DotPath is a path to append if _RelativePath begins
// with a '.', like "./myfile.txt". This function goes through each of 
// _SearchPaths paths and _DotPath and prepends it to _RelativePath. If the 
// result causes the _PathExists function to return true, that path is copied 
// into _StrDestination and a pointer to _StrDestination is returned. If all
// paths are exhausted and none pass _PathExists, nullptr is returned.
char* oFindInPath(char* _StrDestination, size_t _SizeofStrDestination, const char* _SearchPaths, const char* _RelativePath, const char* _DotPath, const oFUNCTION<bool(const char* _Path)>& _PathExists);

// It is often the case things like compiler header search paths and defines are
// input as semi-colon delimited lists and then parsed into command line tool
// parameters to switches. This function consolidates that effort by writing to 
// the destination a series of switches prepended to each token in the specified
// token list.
char* oStrTokToSwitches(char* _StrDestination, size_t _SizeofStrDestination, const char* _Switch, const char* _Tokens, const char* _Separator);

// _____________________________________________________________________________
// Templated-on-size versions of the above functions

template<size_t size> bool oHasMatchingExtension(const char* _Path, const char* (&_Extensions)[size]) { return oHasMatchingExtension(_Path, _Extensions, size); }
template<size_t size> char* oGetFilebase(char (&_StrDestination)[size], const char* _Path) { return oGetFilebase(_StrDestination, size, _Path); }
template<size_t size> char* oReplaceFileExtension(char (&_Path)[size], const char* _Extension) { return oReplaceFileExtension(_Path, size, _Extension); }
template<size_t size> char* oReplaceFilename(char (&_Path)[size], const char* _Filename) { return oReplaceFilename(_Path, size, _Filename); }
template<size_t size> char* oEnsureSeparator(char (&_Path)[size]) { return oEnsureSeparator(_Path, size); }
template<size_t size> char* oMakeRelativePath(char (&_StrDestination)[size], const char* _FullPath, const char* _BasePath, char _FileSeparator = '/') { return oMakeRelativePath(_StrDestination, size, _FullPath, _BasePath, _FileSeparator); }
template<size_t size> char* oFindInPath(char (&_StrDestination)[size], const char* _SearchPath, const char* _RelativePath, const char* _DotPath, const oFUNCTION<bool(const char* _Path)>& _PathExists) { return oFindInPath(_StrDestination, size, _SearchPath, _RelativePath, _DotPath, _PathExists); }
template<size_t size> char* oStrTokToSwitches(char (&_StrDestination)[size], const char* _Switch, const char* _Tokens, const char* _Separator) { return oStrTokToSwitches(_StrDestination, size, _Switch, _Tokens, _Separator); }

// ouro::fixed_string support
template<size_t capacity> char* oGetFilebase(ouro::fixed_string<char, capacity>& _StrDestination, const char* _Path) { return oGetFilebase(_StrDestination, _StrDestination.capacity(), _Path); }
template<size_t capacity> char* oReplaceFileExtension(ouro::fixed_string<char, capacity>& _Path, const char* _Extension) { return oReplaceFileExtension(_Path, _Path.capacity(), _Extension); }
template<size_t capacity> char* oReplaceFilename(ouro::fixed_string<char, capacity>& _Path, const char* _Filename) { return oReplaceFilename(_Path, _Path.capacity(), _Filename); }
template<size_t capacity> char* oEnsureSeparator(ouro::fixed_string<char, capacity>& _Path, char _FileSeparator = '/') { return oEnsureSeparator(_Path, _Path.capacity(), _FileSeparator); }
template<size_t capacity> char* oMakeRelativePath(ouro::fixed_string<char, capacity>& _Path, const char* _FullPath, const char* _BasePath, char _FileSeparator = '/') { return oMakeRelativePath(_Path, _Path.capacity(), _FullPath, _BasePath, _FileSeparator); }
template<size_t capacity> char* oFindInPath(ouro::fixed_string<char, capacity>& _Path, const char* _SearchPath, const char* _RelativePath, const char* _DotPath, const oFUNCTION<bool(const char* _Path)>& _PathExists) { return oFindInPath(_Path, _Path.capacity(), _SearchPath, _RelativePath, _DotPath, _PathExists); }
template<size_t capacity> char* oStrTokToSwitches(ouro::fixed_string<char, capacity>& _StrDestination, const char* _Switch, const char* _Tokens, const char* _Separator) { return oStrTokToSwitches(_StrDestination, _StrDestination.capacity(), _Switch, _Tokens, _Separator); }

#endif
