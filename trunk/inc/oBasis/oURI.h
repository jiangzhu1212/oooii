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
// URI parsing
#pragma once
#ifndef oURI_h
#define oURI_h

#include <oBasis/oMacros.h> // oCOUNTOF
#include <oBasis/oPlatformFeatures.h> // nullptr
#include <oBasis/oFixedString.h>


#define oMAX_SCHEME 32
#define oMAX_URI 512
#define oMAX_URIREF _MAX_PATH

struct oURIParts
{
	oURIParts() { Clear(); }
	oURIParts(const char* _Scheme, const char* _Authority, const char* _Path, const char* _Query, const char* _Fragment);
	inline void Clear() { *Scheme = *Authority = *Path = *Query = *Fragment = 0; }

	oFixedString<char, oMAX_SCHEME> Scheme;
	oStringURI Authority;
	oStringPath Path;
	oStringURI Query;
	oStringURI Fragment;
};

// Given a URI or URI reference, separate out the various components
bool oURIDecompose(const char* _URIReference, char* _Scheme, size_t _SizeofScheme, char* _Authority = nullptr, size_t _SizeofAuthority = 0, char* _Path = nullptr, size_t _SizeOfPath = 0, char* _Query = nullptr, size_t _SizeofQuery = 0, char* _Fragment = nullptr, size_t _SizeofFragment = 0);
inline bool oURIDecompose(const char* _URIReference, oURIParts* _pURIParts) { if (!_pURIParts) return false; return oURIDecompose(_URIReference, _pURIParts->Scheme, oCOUNTOF(_pURIParts->Scheme), _pURIParts->Authority, oCOUNTOF(_pURIParts->Authority), _pURIParts->Path, oCOUNTOF(_pURIParts->Path), _pURIParts->Query, oCOUNTOF(_pURIParts->Query), _pURIParts->Fragment, oCOUNTOF(_pURIParts->Fragment)); }

// Given the specified parts of a URI reference, create a single URI reference
char* oURIRecompose(char* _URIReference, size_t _SizeofURIReference, const char* _Scheme, const char* _Authority, const char* _Path, const char* _Query, const char* _Fragment);
inline char* oURIRecompose(char* _URIReference, size_t _SizeofURIReference, const oURIParts& _URIParts) { return oURIRecompose(_URIReference, _SizeofURIReference, _URIParts.Scheme, _URIParts.Authority, _URIParts.Path, _URIParts.Query, _URIParts.Fragment); }

// Create a cleaned-up copy of _SourceURI in _NormalizedURI.
char* oURINormalize(char* _NormalizedURI, size_t _SizeofNormalizedURI, const char* _SourceURI);

char* oURIFromAbsolutePath(char* _URI, size_t _SizeofURI, const char* _AbsolutePath);
char* oURIFromRelativePath(char* _URIReference, size_t _SizeofURIReference, const char* _RelativePath);
char* oURIToPath(char* _Path, size_t _SizeofPath, const char* _URI);
char* oURIMakeRelativeToBase(char* _URIReference, size_t _SizeofURIReference, const char* _URIBase, const char* _URI);
char* oURIMakeAbsoluteFromBase(char* _URIReference, size_t _SizeofURIReference, const char* _URIBase, const char* _URI);

char* oURIEnsureFileExtension(char* _URIReferenceWithExtension, size_t _SizeofURIReferenceWithExtension, const char* _SourceURIReference, const char* _Extension);

// Templated-on-size versions of the above functions
template<size_t schemeSize, size_t authoritySize, size_t pathSize, size_t querySize, size_t fragmentSize> bool oURIDecompose(const char* _URIReference, char (&_Scheme)[schemeSize], char (&_Authority)[authoritySize], char (&_Path)[pathSize], char (&_Query)[querySize], char (&_Fragment)[fragmentSize]) { return oURIDecompose(_URIReference, _Scheme, schemeSize, _Authority, authoritySize, _Path, pathSize, _Query, querySize, _Fragment, fragmentSize); }
template<size_t size> char* oURIRecompose(char(&_URIReference)[size], const char* _Scheme, const char* _Authority, const char* _Path, const char* _Query, const char* _Fragment) { return oURIRecompose(_URIReference, size, _Scheme, _Authority, _Path, _Query, _Fragment); }
template<size_t size> char* oURIRecompose(char (&_URIReference)[size], const oURIParts& _Parts) { return oURIRecompose(_URIReference, size, _Parts); }
template<size_t size> char* oURINormalize(char (&_NormalizedURI)[size], const char* _SourceURI) { return oURINormalize(_NormalizedURI, size, _SourceURI); }
template<size_t size> char* oURIFromAbsolutePath(char (&_URI)[size], const char* _AbsolutePath) { return oURIFromAbsolutePath(_URI, size, _AbsolutePath); }
template<size_t size> char* oURIFromRelativePath(char (&_URIReference)[size], const char* _RelativePath) { return oURIFromRelativePath(_URIReference, size, _RelativePath); }
template<size_t size> char* oURIToPath(char (&_Path)[size], const char* _URI) { return oURIToPath(_Path, size, _URI); }
template<size_t size> char* oURIMakeRelativeToBase(char (&_URIReference)[size], const char* _URIBase, const char* _URI) { return oURIMakeRelativeToBase(_URIReference, size, _URIBase, _URI); }
template<size_t size> char* oURIMakeAbsoluteFromBase(char (&_URIReference)[size], const char* _URIBase, const char* _URI) { return oURIMakeAbsoluteFromBase(_URIReference, size, _URIBase, _URI); }
template<size_t size> char* oURIEnsureFileExtension(char (&_URIReferenceWithExtension)[size], const char* _SourceURIReference, const char* _Extension) { return oURIEnsureFileExtension(_URIReferenceWithExtension, size, _SourceURIReference, _Extension); }

#endif
