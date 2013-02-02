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

#include <oBasis/oFixedString.h>
#include <oBasis/oMacros.h> // oCOUNTOF
#include <oBasis/oOperators.h>
#include <oBasis/oPlatformFeatures.h> // nullptr

#define oMAX_SCHEME 32
#define oMAX_URI 512
#define oMAX_AUTHORITY 128
#define oMAX_URIREF _MAX_PATH
#define oMAX_QUERY (2048 - oMAX_URI) // http://stackoverflow.com/questions/417142/what-is-the-maximum-length-of-a-url
#define oMAX_FRAGMENT 128
#define oRESERVED_URI_CHARS "!*'();:@&=+$,?#[]/"
#define oRESERVED_URI_CHARS_WITH_SPACE oRESERVED_URI_CHARS " "

struct oURIParts
{
	oURIParts() { Clear(); }
	oURIParts(const char* _Scheme, const char* _Authority, const char* _Path, const char* _Query, const char* _Fragment);
	inline void Clear() { *Scheme = *Authority = *Path = *Query = *Fragment = 0; }

	// one of the good practices from http://www.textuality.com/tag/uri-comp-2.html
	// @oooii-tony: How do case-sensitive file systems work with this?
	inline void ToLower() { oToLower(Scheme); oToLower(Authority); oToLower(Path);  oToLower(Query); oToLower(Fragment); }
	inline bool Empty() const { return Scheme.empty() && Authority.empty() && Path.empty() && Query.empty() && Fragment.empty(); }

	oFixedString<char, oMAX_SCHEME> Scheme;
	oFixedString<char, oMAX_AUTHORITY> Authority;
	oStringPath Path;
	oFixedString<char, oMAX_QUERY> Query;
	oFixedString<char, oMAX_FRAGMENT> Fragment;
};

// A URI reference is like a relative path and may not include an authority or 
// scheme, or even a full path. A URI is only a URI when it is fully specified,
// so this does a check to see if the scheme is in place.
bool oURIIsAbsolute(const char* _URIReference);

// Given a URI or URI reference, separate out the various components
bool oURIDecompose(const char* _URIReference, char* _Scheme, size_t _SizeofScheme, char* _Authority = nullptr, size_t _SizeofAuthority = 0, char* _Path = nullptr, size_t _SizeOfPath = 0, char* _Query = nullptr, size_t _SizeofQuery = 0, char* _Fragment = nullptr, size_t _SizeofFragment = 0);
inline bool oURIDecompose(const char* _URIReference, oURIParts* _pURIParts) { if (!_pURIParts) return false; return oURIDecompose(_URIReference, _pURIParts->Scheme, oCOUNTOF(_pURIParts->Scheme), _pURIParts->Authority, oCOUNTOF(_pURIParts->Authority), _pURIParts->Path, oCOUNTOF(_pURIParts->Path), _pURIParts->Query, oCOUNTOF(_pURIParts->Query), _pURIParts->Fragment, oCOUNTOF(_pURIParts->Fragment)); }

// Given the specified parts of a URI reference, create a single URI reference
char* oURIRecompose(char* _URIReference, size_t _SizeofURIReference, const char* _Scheme, const char* _Authority, const char* _Path, const char* _Query, const char* _Fragment);
inline char* oURIRecompose(char* _URIReference, size_t _SizeofURIReference, const oURIParts& _URIParts) { return oURIRecompose(_URIReference, _SizeofURIReference, _URIParts.Scheme, _URIParts.Authority, _URIParts.Path, _URIParts.Query, _URIParts.Fragment); }

// Create a cleaned-up copy of _SourceURI in _NormalizedURI.
inline char* oURINormalize(char* _NormalizedURI, size_t _SizeofNormalizedURI, const char* _SourceURI) { oURIParts parts; return oURIDecompose(_SourceURI, &parts) && oURIRecompose(_NormalizedURI, _SizeofNormalizedURI, parts) ? _NormalizedURI : nullptr; }

// Conceptually this strcpys source into destination, but replaces any char 
// found in the reserved char string with its %octet form. Because this 
// operation expands the length of the string, _StrDestination and _StrSource 
// must be separate buffers.
char* oURIPercentEncode(char* _StrDestination, size_t _SizeofStrDestination, const char* _StrSource, const char* _StrReservedChars = oRESERVED_URI_CHARS);

// Conceptually strcpy's source into destination, but replaces any percent-
// encoding into its true ASCII (not quite UTF-8 here) value. _StrDestination 
// and _StrSource can be the same buffer.
char* oURIPercentDecode(char* _StrDestination, size_t _SizeofStrDestination, const char* _StrSource);

// Generate a hash fit for a hashmap/unordered set. This expects a normalized
// URI reference as suggested here: http://www.textuality.com/tag/uri-comp-2.html
size_t oURIHash(const char* _URIReference);

char* oURIFromAbsolutePath(char* _URI, size_t _SizeofURI, const char* _AbsolutePath);
char* oURIFromRelativePath(char* _URIReference, size_t _SizeofURIReference, const char* _RelativePath);

// Converts a URI to a file path. This expects a file scheme, and also handles
// UNC paths properly. NOTE: Prefer oSystemURIToPath in oPlatform/oSystem.h 
// because it supports specifying oSYSPATHs as authorities.
char* oURIToPath(char* _Path, size_t _SizeofPath, const char* _URI);

// Convert an oURIParts strcut to a path, properly handling UNC paths. NOTE: 
// Prefer oSystemURIPartsToPath in oPlatform/oSystem.h because it supports 
// specifying oSYSPATHs as authorities.
char* oURIPartsToPath(char* _Path, size_t _SizeofPath, const oURIParts& _URIParts);

char* oURIMakeRelativeToBase(char* _URIReference, size_t _SizeofURIReference, const char* _URIBase, const char* _URI);
char* oURIMakeAbsoluteFromBase(char* _URIReference, size_t _SizeofURIReference, const char* _URIBase, const char* _URI);

char* oURIEnsureFileExtension(char* _URIReferenceWithExtension, size_t _SizeofURIReferenceWithExtension, const char* _SourceURIReference, const char* _Extension);

// Assumes a pattern of key=val&key=val&key=val&... and iterates through each key value pair
void oURIQueryEnumKeyValuePairs(const char* _URIQuery, oFUNCTION<void(const char* _Key, const char* _Value)> _Enumerator);

// Templated-on-size versions of the above functions
template<size_t schemeSize, size_t authoritySize, size_t pathSize, size_t querySize, size_t fragmentSize> bool oURIDecompose(const char* _URIReference, char (&_Scheme)[schemeSize], char (&_Authority)[authoritySize], char (&_Path)[pathSize], char (&_Query)[querySize], char (&_Fragment)[fragmentSize]) { return oURIDecompose(_URIReference, _Scheme, schemeSize, _Authority, authoritySize, _Path, pathSize, _Query, querySize, _Fragment, fragmentSize); }
template<size_t size> char* oURIRecompose(char(&_URIReference)[size], const char* _Scheme, const char* _Authority, const char* _Path, const char* _Query, const char* _Fragment) { return oURIRecompose(_URIReference, size, _Scheme, _Authority, _Path, _Query, _Fragment); }
template<size_t size> char* oURIRecompose(char (&_URIReference)[size], const oURIParts& _Parts) { return oURIRecompose(_URIReference, size, _Parts); }
template<size_t size> char* oURINormalize(char (&_NormalizedURI)[size], const char* _SourceURI) { return oURINormalize(_NormalizedURI, size, _SourceURI); }
template<size_t size> char* oURIPercentEncode(char (&_StrDestination)[size], const char* _StrSource, const char* _StrReservedChars = oRESERVED_URI_CHARS) { return oURIPercentEncode(_StrDestination, size, _StrSource, _StrReservedChars); }
template<size_t size> char* oURIPercentDecode(char (&_StrDestination)[size], const char* _StrSource) { return oURIPercentDecode(_StrDestination, size, _StrSource); }
template<size_t size> char* oURIFromAbsolutePath(char (&_URI)[size], const char* _AbsolutePath) { return oURIFromAbsolutePath(_URI, size, _AbsolutePath); }
template<size_t size> char* oURIFromRelativePath(char (&_URIReference)[size], const char* _RelativePath) { return oURIFromRelativePath(_URIReference, size, _RelativePath); }
template<size_t size> char* oURIToPath(char (&_Path)[size], const char* _URI) { return oURIToPath(_Path, size, _URI); }
template<size_t size> char* oURIPartsToPath(char (&_Path)[size], const oURIParts& _URIParts) { return oURIPartsToPath(_Path, size, _URIParts); }
template<size_t size> char* oURIMakeRelativeToBase(char (&_URIReference)[size], const char* _URIBase, const char* _URI) { return oURIMakeRelativeToBase(_URIReference, size, _URIBase, _URI); }
template<size_t size> char* oURIMakeAbsoluteFromBase(char (&_URIReference)[size], const char* _URIBase, const char* _URI) { return oURIMakeAbsoluteFromBase(_URIReference, size, _URIBase, _URI); }
template<size_t size> char* oURIEnsureFileExtension(char (&_URIReferenceWithExtension)[size], const char* _SourceURIReference, const char* _Extension) { return oURIEnsureFileExtension(_URIReferenceWithExtension, size, _SourceURIReference, _Extension); }

// oFixedString support
template<size_t capacity> char* oURIRecompose(oFixedString<char, capacity>& _URIReference, const char* _Scheme, const char* _Authority, const char* _Path, const char* _Query, const char* _Fragment) { return oURIRecompose(_URIReference, _URIReference.capacity(), _Scheme, _Authority, _Path, _Query, _Fragment); }
template<size_t capacity> char* oURIRecompose(oFixedString<char, capacity>& _URIReference, const oURIParts& _Parts) { return oURIRecompose(_URIReference, _URIReference.capacity(), _Parts); }
template<size_t capacity> char* oURINormalize(oFixedString<char, capacity>& _NormalizedURI, const char* _SourceURI) { return oURINormalize(_NormalizedURI, _NormalizedURI.capacity(), _SourceURI); }
template<size_t capacity> char* oURIPercentEncode(oFixedString<char, capacity>& _StrDestination, const char* _StrSource, const char* _StrReservedChars = oRESERVED_URI_CHARS) { return oURIPercentEncode(_StrDestination, _StrDestination.capacity(), _StrSource, _StrReservedChars); }
template<size_t capacity> char* oURIPercentDecode(oFixedString<char, capacity>& _StrDestination, const char* _StrSource) { return oURIPercentDecode(_StrDestination, _StrDestination.capacity(), _StrSource); }
template<size_t capacity> char* oURIFromAbsolutePath(oFixedString<char, capacity>& _URI, const char* _AbsolutePath) { return oURIFromAbsolutePath(_URI, _URI.capacity(), _AbsolutePath); }
template<size_t capacity> char* oURIFromRelativePath(oFixedString<char, capacity>& _URIReference, const char* _RelativePath) { return oURIFromRelativePath(_URIReference, _URIReference.capacity(), _RelativePath); }
template<size_t capacity> char* oURIToPath(oFixedString<char, capacity>& _Path, const char* _URI) { return oURIToPath(_Path, _Path.capacity(), _URI); }
template<size_t capacity> char* oURIPartsToPath(oFixedString<char, capacity>& _Path, const oURIParts& _URIParts) { return oURIPartsToPath(_Path, _Path.capacity(), _URIParts); }
template<size_t capacity> char* oURIMakeRelativeToBase(oFixedString<char, capacity>& _URIReference, const char* _URIBase, const char* _URI) { return oURIMakeRelativeToBase(_URIReference, _URIReference.capacity(), _URIBase, _URI); }
template<size_t capacity> char* oURIMakeAbsoluteFromBase(oFixedString<char, capacity>& _URIReference, const char* _URIBase, const char* _URI) { return oURIMakeAbsoluteFromBase(_URIReference, _URIReference.capacity(), _URIBase, _URI); }
template<size_t capacity> char* oURIEnsureFileExtension(oFixedString<char, capacity>& _URIReferenceWithExtension, const char* _SourceURIReference, const char* _Extension) { return oURIEnsureFileExtension(_URIReferenceWithExtension, _URIReferenceWithExtension.capacity(), _SourceURIReference, _Extension); }


// Class that enforces some well-formed URI ideas as described here:
// http://www.textuality.com/tag/uri-comp-2.html. Favor oURI whenever possible
// especially in associative containers like std::map or std::unordered_map
class oURI : public oCompareable<oURI>
{
	// NOTE: This class is designed to enforce normalized URI formatting and 
	// comparison rules, thus printf'ing or modifying the buffers directly is not 
	// allowed.

public:
	oURI() : HashID(0) {}
	oURI(const char* _URIReference) { operator=(_URIReference); }

	inline const char* Scheme() const { return URIParts.Scheme; }
	inline const char* Authority() const { return URIParts.Authority; }
	inline const char* Path() const { return URIParts.Path; }
	inline const char* Query() const { return URIParts.Query; }
	inline const char* Fragment() const { return URIParts.Fragment; }

	inline size_t Hash() const { return HashID; }
	inline operator size_t() const { return HashID; }

	inline bool Empty() const { return URIParts.Empty(); }
	inline bool Valid() const { return !Empty(); }
	inline bool IsRelativeReference() const { return !Empty() && URIParts.Scheme.empty(); }

	// @oooii-tony: This fragment test is iffy, read section 4.3 in 
	// http://tools.ietf.org/html/rfc3986#section-4.3. The absolute-URI = ...
	// part is different than the verbiage, so go with the pseudo-code.
	inline bool IsAbsolute() const { return !IsRelativeReference() && URIParts.Fragment.empty(); }

	inline void Clear() { URIParts.Clear(); HashID = 0; }

	const oURI& operator=(const char* _That); // This does non-trivial work, including reformatting (tolower) and hash calculation
	bool operator==(const oURI& _That) const; // fit for std::unordered_set
	bool operator<(const oURI& _That) const; // fit for std::map

private:
	oURIParts URIParts;
	size_t HashID;
};

// Use this for std::unordered_map
struct oURIHasher { size_t operator()(const oURI& _URI) const { return _URI.Hash(); } };

#endif