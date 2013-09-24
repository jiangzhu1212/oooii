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
#include <oBasis/oURI.h>
#include <oBase/assert.h>
#include <oBasis/oPath.h>
#include <oBasis/oError.h>
#include <cerrno>
#include <cstdlib>
#include <regex>

using namespace std;
using namespace ouro;

bool oURIIsURI(const char* _URIReference)
{
	oURIParts parts;
	oURIDecompose(_URIReference, &parts);
	return oSTRVALID(parts.Scheme);
}

bool oURIIsAbsolute(const char* _URIReference)
{
	oURIParts parts;
	oURIDecompose(_URIReference, &parts);
	return oSTRVALID(parts.Scheme) && !oSTRVALID(parts.Fragment);
}

bool oURIIsAbsolute(const oURIParts& _Parts)
{
	return oSTRVALID(_Parts.Scheme) && !oSTRVALID(_Parts.Fragment);
}

bool oURIIsSameDocument(const char* _URIReference, const char* _DocumentURI)
{
	// Early out if _URIReference is empty or only has a fragment
	if (!oSTRVALID(_URIReference) || _URIReference[0]=='#')
		return true;

	// See if we end up with the same URIs (minus fragment) if we'd resolve
	// the _URIReference with the _DocumentAbsoluteURI
	uri_string ResolvedURI;
	oURIResolve(ResolvedURI, _DocumentURI, _URIReference); // TODO: Rename this function to resolve, as absolute URIs can't have fragments and this function doesn't intend that

	// Note: an Absolute URI can not contain a fragment, so we could also
	// require an absolute document URI as input, then remove the fragment 
	// from ResolvedURI and then simply do a single string compare.
	oURIParts DocumentParts;
	oURIDecompose(_DocumentURI, &DocumentParts);

	oURIParts ResolvedParts;
	oURIDecompose(ResolvedURI, &ResolvedParts);

	return _stricmp(DocumentParts.Scheme, ResolvedParts.Scheme)==0 && 
		_stricmp(DocumentParts.Authority, ResolvedParts.Authority)==0 &&
		_stricmp(DocumentParts.Path, ResolvedParts.Path)==0 &&
		_stricmp(DocumentParts.Query, ResolvedParts.Query)==0;
}

oURIParts::oURIParts(const char* _Scheme, const char* _Authority, const char* _Path, const char* _Query, const char* _Fragment)
{
	strlcpy(Scheme, oSAFESTR(_Scheme));
	strlcpy(Authority, oSAFESTR(_Authority));
	strlcpy(Path, oSAFESTR(_Path));
	strlcpy(Query, oSAFESTR(_Query));
	strlcpy(Fragment, oSAFESTR(_Fragment));
}

// Copies a begin/end iterator range into the specified destination as a string
static char* oStrcpy(char* _StrDestination, size_t _SizeofStrDestination, const char* _Start, const char* _End)
{
	if (!_StrDestination || !_Start || !_End) return nullptr;
	const size_t sourceLength = std::distance(_Start, _End);
	if (_SizeofStrDestination < (1+sourceLength)) return nullptr;
	memcpy(_StrDestination, _Start, sourceLength);
	_StrDestination[sourceLength] = 0;
	return _StrDestination;
}
template<size_t size> char* oStrcpy(char (&_StrDestination)[size], const char* _Start, const char* _End) { return oStrcpy(_StrDestination, size, _Start, _End); }

char* oURIPercentEncode(char* _StrDestination, size_t _SizeofStrDestination, const char* _StrSource, const char* _StrReservedChars)
{
	*_StrDestination = 0;
	char* d = _StrDestination;
	char* end = d + _SizeofStrDestination - 1;
	*(end-1) = 0; // don't allow read outside the buffer even in failure cases

	oASSERT(_StrSource < _StrDestination || _StrSource >= (_StrDestination + _SizeofStrDestination), "Overlapping buffers not allowed");

	const char* s = _StrSource;
	while (*s)
	{
		if (strchr(_StrReservedChars, *s))
		{
			if ((d+3) > end)
			{
				oErrorSetLast(std::errc::no_buffer_space);
				return nullptr;
			}

			// @oooii-tony: This assumes ascii-in... no UTF-8 support here.
			*d++ = '%';
			snprintf(d, std::distance(d, end), "%02x", *s++); // use lower-case escaping http://www.textuality.com/tag/uri-comp-2.html
			d += 2;
		}

		else if (d >= end)
		{
			oErrorSetLast(std::errc::no_buffer_space);
			return nullptr;
		}

		else
			*d++ = *s++;
	}

	*d = 0;
	return _StrDestination;
}

// http://tools.ietf.org/html/rfc3986#appendix-B
static regex reURI("^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?", std::tr1::regex_constants::optimize); // @oooii-tony: ok static (duplication in DLLs won't affect correctness)
bool oURIDecompose(const char* _URIReference, char* _Scheme, size_t _SizeofScheme, char* _Authority, size_t _SizeofAuthority, char* _Path, size_t _SizeofPath, char* _Query, size_t _SizeofQuery, char* _Fragment, size_t _SizeofFragment)
{
	if (_URIReference)
	{
		cmatch matches;
		regex_search(_URIReference, matches, reURI);
		if (matches.empty())
			return false;

		#define COPY(part, index) do { if (_##part && !oStrcpy(_##part, _Sizeof##part, matches[index].first, matches[index].second)) return false; if (_##part && !percent_decode(_##part, _Sizeof##part, _##part)) return false; } while(false)
			COPY(Scheme, 2);
			if (_Scheme)
				tolower(_Scheme); // http://www.ietf.org/rfc/rfc2396.txt 3.1. Scheme Component
			COPY(Authority, 4);
			COPY(Path, 5);
			COPY(Query, 7);
			COPY(Fragment, 9);
		#undef COPY

		// Decode any percent-encoded stuff now. http://tools.ietf.org/html/rfc3986#page-14 section 2.4
		percent_decode(_Path, _SizeofPath, _Path);
		return true;
	}

	return false;
}

char* oURIRecompose(char* _URIReference, size_t _SizeofURIReference, const char* _Scheme, const char* _Authority, const char* _Path, const char* _Query, const char* _Fragment)
{
	/** <citation
		usage="Adaptation" 
		reason="This code seems to handle a lot of real-world compatibility issues, so rather than rediscover these issues, use the learnings from collada-dom." 
		author="Sony Computer Entertainment of America"
		description="http://sourceforge.net/projects/collada-dom"
		license="Shared Source License"
		licenseurl="http://research.scea.com/scea_shared_source_license.html"
	/>*/

	#define SAFECAT(str) do {	if (strlcat(_URIReference, str, _SizeofURIReference) >= _SizeofURIReference) return nullptr; } while(false)

	oASSERT(_Scheme < _URIReference || _Scheme >= (_URIReference + _SizeofURIReference), "Overlapping buffers not allowed");
	oASSERT(_Authority < _URIReference || _Authority >= (_URIReference + _SizeofURIReference), "Overlapping buffers not allowed");
	oASSERT(_Path < _URIReference || _Path >= (_URIReference + _SizeofURIReference), "Overlapping buffers not allowed");
	oASSERT(_Query < _URIReference || _Query >= (_URIReference + _SizeofURIReference), "Overlapping buffers not allowed");
	oASSERT(_Fragment < _URIReference || _Fragment >= (_URIReference + _SizeofURIReference), "Overlapping buffers not allowed");

	*_URIReference = 0;
	if (oSTRVALID(_Scheme))
	{
		SAFECAT(_Scheme);
		tolower(_URIReference); // http://www.ietf.org/rfc/rfc2396.txt 3.1. Scheme Component
		SAFECAT(":");
	}

	if (oSTRVALID(_Authority))
	{
		SAFECAT("//");
		SAFECAT(_Authority);
	}
	else if (0==_memicmp(_Scheme,"file",4))
	{
		SAFECAT("//");
		if (oSTRVALID(_Path) && _Path[0] != '/')
			SAFECAT("/");
	}

	path_string path;
	if (!clean_path(path, _Path))
		return nullptr;

	size_t URIReferenceLength = strlen(_URIReference);
	if (!oURIPercentEncode(_URIReference + URIReferenceLength, _SizeofURIReference - URIReferenceLength, path, " "))
		return nullptr;

	if (oSTRVALID(_Query))
	{
		SAFECAT("?");
		SAFECAT(_Query);
	}

	if (oSTRVALID(_Fragment))
	{
		SAFECAT("#");
		SAFECAT(_Fragment);
	}

	return _URIReference;
	#undef SAFECAT
}

char* oURIFromAbsolutePath(char* _URI, size_t _SizeofURI, const char* _AbsolutePath)
{
	if (!_AbsolutePath) return nullptr;
	if (*_AbsolutePath == 0)
	{
		*_URI = 0;
		return _URI;
	}

	uri_string Authority;
	uri_string Path;

	if (ouro::path(_AbsolutePath).is_windows_unc())
	{
		// Move to end of authority
		const char* end = _AbsolutePath + 2;
		while (*end && !oIsSeparator(*end))
			end++;

		strncpy_s(Authority, Authority.capacity(), _AbsolutePath + 2, end - (_AbsolutePath + 2));
		_AbsolutePath = end;
		Path = _AbsolutePath;
	}
	else
	{
		snprintf(Path, "/%s", _AbsolutePath);
	}

	return oURIRecompose(_URI, _SizeofURI, "file", Authority, Path, "", "");
}

char* oURIFromRelativePath(char* _URIReference, size_t _SizeofURIReference, const char* _RelativePath)
{
	path_string path;
	if (!clean_path(path, _RelativePath))
		return nullptr;

	if (!oURIPercentEncode(_URIReference, _SizeofURIReference, path, " "))
		return nullptr;
	
	return _URIReference;
}

char* oURIToPath(char* _Path, size_t _SizeofPath, const char* _URI)
{
	oURIParts parts;
	if (!oURIDecompose(_URI, &parts))
		return nullptr;

	return oURIPartsToPath(_Path, _SizeofPath, parts);
}

char* oURIPartsToPath(char* _Path, size_t _SizeofPath, const oURIParts& _URIParts)
{
	*_Path = 0;

	#define SAFECAT(str) do { if (strlcat(_Path, str, _SizeofPath) >= _SizeofPath) return false; } while(false)

	const bool isWindows = true;
	const char* p = _URIParts.Path.c_str();
	bool skipFirstTwo = false;

	if (isWindows)
	{
		if (!_URIParts.Authority.empty())
		{
			SAFECAT("\\\\");
			SAFECAT(_URIParts.Authority);
		}
		
		if (p[0] == '/' && p[2] == ':')
			p++;
	}

	SAFECAT(p);

	// convert forward to backslashes only for UNC since it won't work out of 
	// Non-URI aware interfaces (i.e. File Explorer, Run Menu, etc.) unless 
	// formatted this way.
	if (!_URIParts.Authority.empty())
	{
		char* p = _Path;
		if (p[0] == '/' && p[1] == '/')
		{
			p[0] = '\\';
			p[1] = '\\';
		}
	}

	return _Path;

	#undef SAFECAT
}

char* oURIRelativize(char* _URIReference, size_t _SizeofURIReference, const char* _URIBase, const char* _URI)
{
	/** <citation
		usage="Adaptation" 
		reason="This code seems to handle a lot of real-world compatibility issues, so rather than rediscover these issues, use the learnings from collada-dom." 
		author="Sony Computer Entertainment of America"
		description="http://sourceforge.net/projects/collada-dom"
		license="Shared Source License"
		licenseurl="http://research.scea.com/scea_shared_source_license.html"
	/>*/

	oURIParts base_d, d;
	
	if (!oURIDecompose(_URIBase, &base_d))
		return nullptr;

	if (!oURIDecompose(_URI, &d))
		return nullptr;

	// Can only do this function if both URIs have the same Scheme and Authority
	if (_stricmp(base_d.Scheme, d.Scheme) || _stricmp(base_d.Authority, d.Authority))
		return nullptr;

	// Since we're outputting a URI reference with a relative path, 
	// it shouldn't have a scheme. And because the authority is also assumed
	// to be the same, we can also clear that.
	d.Scheme.clear();
	d.Authority.clear();

	path_string new_Path;
	if (!relativize_path(new_Path, base_d.Path, d.Path))
		return nullptr;

	return oURIRecompose(_URIReference, _SizeofURIReference, d.Scheme, d.Authority, new_Path, d.Query, d.Fragment);
}

char* oURIResolve(char* _URIReference, size_t _SizeofURIReference, const char* _URIBase, const char* _URI)
{
	oURIParts ResultURIParts;
	oURIDecompose(_URIBase, &ResultURIParts);

	oURIParts URIParts;
	oURIDecompose(_URI, &URIParts);

	if (oSTRVALID(URIParts.Scheme))
	{
		ResultURIParts.Scheme = URIParts.Scheme;
		ResultURIParts.Authority = URIParts.Authority;
		ResultURIParts.Path = URIParts.Path;
		ResultURIParts.Query = URIParts.Query;
		ResultURIParts.Fragment = URIParts.Fragment;
	}
	else if (oSTRVALID(URIParts.Authority))
	{
		ResultURIParts.Authority = URIParts.Authority;
		ResultURIParts.Path = URIParts.Path;
		ResultURIParts.Query = URIParts.Query;
		ResultURIParts.Fragment = URIParts.Fragment;
	}
	else if (oSTRVALID(URIParts.Path))
	{
		if (URIParts.Path[0]=='/')
			ResultURIParts.Path = URIParts.Path;
		else
		{
			path p = ResultURIParts.Path;
			p.remove_filename();
			p /= URIParts.Path;
			ResultURIParts.Path = p;
		}

		ResultURIParts.Query = URIParts.Query;
		ResultURIParts.Fragment = URIParts.Fragment;
	}
	else if (oSTRVALID(URIParts.Query))
	{
		ResultURIParts.Query = URIParts.Query;
		ResultURIParts.Fragment = URIParts.Fragment;
	}
	else if (oSTRVALID(URIParts.Fragment))
	{
		ResultURIParts.Fragment = URIParts.Fragment;
	}

	return oURIRecompose(_URIReference, _SizeofURIReference, ResultURIParts);
}

char* oURIEnsureFileExtension(char* _URIReferenceWithExtension, size_t _SizeofURIReferenceWithExtension, const char* _SourceURIReference, const char* _Extension)
{
	if (!oSTRVALID(_Extension))
	{
		if (strlcpy(_URIReferenceWithExtension, _SourceURIReference, _SizeofURIReferenceWithExtension) >= _SizeofURIReferenceWithExtension)
			return nullptr;
	}

	else
	{
		oURIParts parts;
		if (!oURIDecompose(_SourceURIReference, &parts))
			return nullptr;
		parts.Path = ouro::path(parts.Path).replace_extension(_Extension).c_str();
		if (!oURIRecompose(_URIReferenceWithExtension, _SizeofURIReferenceWithExtension, parts))
			return nullptr;
	}

	return _URIReferenceWithExtension;
}

namespace ouro {

bool from_string(char (*_pStrDestination)[oMAX_URI], const char* _StrSource)
{
	return strlcpy(*_pStrDestination, _StrSource, oMAX_URI) < oMAX_URI;
}

bool from_string(oURIParts* _pURIParts, const char* _StrSource)
{
	return oURIDecompose(_StrSource, _pURIParts);
}

} // namespace ouro

static std::regex QueryRegex("(.+?)=(.+?)&", std::tr1::regex_constants::optimize); // @oooii-tony: ok static (duplication in DLLs won't affect correctness)
void oURIQueryEnumKeyValuePairs(const char* _URIQuery, oFUNCTION<void(const char* _Key, const char* _Value)> _Enumerator)
{
	const std::cregex_token_iterator end;
	int ArgsToCollect[] = {1,2};

	// FIXME: Copying the URI so we can append it with & so the regex matches
	uri_string copy = _URIQuery;
	sncatf(copy, "&");

	for (std::cregex_token_iterator Groups(copy, copy + copy.length(), QueryRegex, ArgsToCollect); Groups != end; ++Groups)
	{
		std::string Query = *Groups;
		std::string Value = *(++Groups);
		_Enumerator(Query.c_str(), Value.c_str());
	}
}

const oURI& oURI::operator=(const char* _That)
{
	// @oooii-tony: start with KISS... optimize if this shows up on benchmarks
	// basically ensure the memory pattern is the same so that we can blindly
	// run the hash through the whole mess.
	memset(this, 0, sizeof(*this));

	// apply good practices from http://www.textuality.com/tag/uri-comp-2.html
	if (!oURIDecompose(_That, &URIParts) || !clean_path(URIParts.Path, URIParts.Path, '/', true))
		Clear();
	URIParts.ToLower();

	uint128 h = murmur3(this, sizeof(*this));
	HashID = (size_t)h; // we want the most random bits of the 128, but it seems from the avalanching of murmur, it doesn't matter (http://blog.aggregateknowledge.com/tag/murmur-hash/)
	return *this;
}

bool oURI::operator==(const oURI& _That) const
{
	if (HashID != _That.HashID)
		return false;
		#ifdef _DEBUG
			if (memcmp(this, &_That, sizeof(*this)))
			{
				uri_string A, B;
				oURIRecompose(A, URIParts);
				oURIRecompose(B, _That.URIParts);
				oASSERT(false, "oURI hash collision between \"%s\" and \"%s\"", A.c_str(), B.c_str());
			}
		#endif
	return true;
}

bool oURI::operator<(const oURI& _That) const
{
	return memcmp(this, &_That, sizeof(*this)) < 0;
}

namespace ouro {

bool from_string(oURI* _pURI, const char* _StrSource)
{
	*_pURI = _StrSource;
	return true;
}

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const oURI& _URI)
{
	return oURIRecompose(_StrDestination, _SizeofStrDestination, _URI.GetParts());
}

} // namespace ouro

typedef bool (*s_oURI_FromString)(oURI*, const char*);
typedef char* (*s_oURI_ToString)(char*, size_t, const oURI&);

oRTTI_ATOM_DESCRIPTION(oURI,oURI,false,false,oURI, 1, \
	(oRTTIFromString)(s_oURI_FromString)from_string, \
	(oRTTIToString)(s_oURI_ToString)to_string, \
	nullptr, \
	nullptr, \
	nullptr, \
	nullptr, \
	nullptr, \
	nullptr, \
	oRTTI_CAPS_ARRAY)
