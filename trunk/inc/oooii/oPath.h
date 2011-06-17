// $(header)

// Generic code for working with paths, both parsing and searching. Querying
// the platform for system/environmental paths is factored into oStdio to 
// centralize platform-dependent code. Though this uses a small number of 
// platform-dependent functions, there is enough generic code here to keep
// it separate from the centralized platform details.
#pragma once
#ifndef oPath_h
#define oPath_h

#include <oooii/oStdio.h> // oSYSPATH

inline bool oIsFileSeparator(char c) { return c == '\\' || c == '/'; }
inline bool oIsUNCPath(const char* _Path) { return _Path && oIsFileSeparator(*_Path) && oIsFileSeparator(*(_Path+1)); }
inline bool oIsFullPath(const char* _Path) { return _Path && _Path && *(_Path+1) == ':'; }

// Return pointer to file base
const char* oGetFilebase(const char* _Path);
char* oGetFilebase(char* _Path);
errno_t oGetFilebase(char* _Filebase, size_t _SizeofFilebase, const char* _Path);

// Return pointer to file extension (i.e. the '.' character)
// if there is no extension, this returns the pointer to the 
// nul terminator of the string.
const char* oGetFileExtension(const char* _Path);
char* oGetFileExtension(char* _Path);

// Return pointer to the volume name
inline const char* oGetVolumeName(const char* _Path) { const char* p = _Path; if (oIsUNCPath(p)) p += 2; return p; }
inline char* oGetVolumeName(char* _Path) { char* p = _Path; if (oIsUNCPath(p)) p += 2; return p; }

// Inserts or replaces the current extension with the specified one
errno_t oReplaceFileExtension(char* _Path, size_t _SizeofPath, const char* _Extension);

// Removes the right-most file or dir name
char* oTrimFilename(char* _Path);
char* oTrimFileExtension(char* _Path);

// Ensures a backslash or forward slash is the rightmost char
// returns new length of path
errno_t oEnsureFileSeparator(char* _Path, size_t _SizeofPath);

// Converts . and .. into something meaningful and replaces fseps with the specified one.
// _CleanedPath and _SourcePath can be the same pointer.
errno_t oCleanPath(char* _CleanedPath, size_t _SizeofCleanedPath, const char* _SourcePath, char _FileSeparator = '/');

// Returns the path to a .txt file with the name of the current exe 
// concatenated with the (optionally) specified suffix and a sortable timestamp 
// in the filename to ensure uniqueness.
errno_t oGetLogFilePath(char* _StrDestination, size_t _SizeofStrDestination, const char* _ExeSuffix = 0);

// Standard Unix/MS-DOS style wildcard matching
bool oMatchesWildcard(const char* _Wildcard, const char* _Path);

// Fills path if the relative path exists when one of the paths in search_path 
// is prepended. The format of _SearchPath is paths separated by a semi-colon. 
// This also takes a _DotPath, which is what to append if relpath begin with a 
// "./". NOTE: If you want _RelativePath as-is to be found, include "." in 
// the search path.
bool oFindInPath(char* _ResultingFullPath, size_t _SizeofResultingFullPath, const char* _SearchPath, const char* _RelativePath, const char* _DotPath, oPATH_EXISTS_FUNCTION _PathExists);

// Find a file in the specified system path
bool oFindInSysPath(char* _ResultingFullPath, size_t _SizeofResultingFullPath, oSYSPATH _SysPath, const char* _RelativePath, const char* _DotPath, oPATH_EXISTS_FUNCTION _PathExists);

// Searches all system and environment paths, as well as extraSearchPath which 
// is a string of paths delimited by semi-colons. _RelativePath is the filename/
// partial path to be matched against the various prefixes to get a full path.
bool oFindPath(char* _ResultingFullPath, size_t _SizeofResultingFullPath, const char* _RelativePath, const char* _DotPath, const char* _ExtraSearchPath, oPATH_EXISTS_FUNCTION _PathExists);

//returns the index to one after the last path separator where the previous part of the path is the same between the 2 arguments.
size_t oCommonPath(const char* _path1, const char* _path2);

//takes a full path, and creates a path relative to the reference path.
void oMakeRelativePath(char* _relativePath, const char* _fullPath, const char* _referencePath);

// _____________________________________________________________________________
// Templated-on-size versions of the above functions

template<size_t size> inline errno_t oGetFilebase(char (&_Filebase)[size], const char* _Path) { return oGetFilebase(_Filebase, size, _Path); }
template<size_t size> inline errno_t oReplaceFileExtension(char (&_Path)[size], const char* _Extension) { return oReplaceFileExtension(_Path, size, _Extension); }
template<size_t size> inline errno_t oEnsureFileSeparator(char (&_Path)[size]) { return oEnsureFileSeparator(_Path, size); }
template<size_t size> inline errno_t oCleanPath(char (&_SizeofCleanedPath)[size], const char* _SourcePath, char _FileSeparator = '/') { return oCleanPath(_SizeofCleanedPath, size, _SourcePath, _FileSeparator); }
template<size_t size> inline errno_t oGetLogFilePath(char (&_StrDestination)[size], const char* _ExeSuffix = 0) { return oGetLogFilePath(_StrDestination, size, _ExeSuffix); }
template<size_t size> inline bool oFindInPath(char (&_ResultingFullPath)[size], const char* _SearchPath, const char* _RelativePath, const char* _DotPath, oPATH_EXISTS_FUNCTION _PathExists) { return oFindInPath(_ResultingFullPath, size, _SearchPath, _RelativePath, _DotPath, _PathExists); }
template<size_t size> inline bool oFindInSysPath(char(&_ResultingFullPath)[size], oSYSPATH _SysPath, const char* _RelativePath, const char* _DotPath, oPATH_EXISTS_FUNCTION _PathExists) { return oFindInSysPath(_ResultingFullPath, size, _SysPath, _RelativePath, _DotPath, _PathExists); }
template<size_t size> inline bool oFindPath(char (&_ResultingFullPath)[size], const char* _RelativePath, const char* _DotPath, const char* _ExtraSearchPath, oPATH_EXISTS_FUNCTION _PathExists) { return oFindPath(_ResultingFullPath, size, _RelativePath, _DotPath, _ExtraSearchPath, _PathExists); }
#endif
