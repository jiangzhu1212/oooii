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
#include <oBasis/oURI.h>
#include <oBasis/oAssert.h>
#include <oBasis/oString.h>
#include <oBasis/oPath.h>
#include <cerrno>
#include <cstdlib>
#include <regex>

using namespace std;
using namespace std::tr1;

oURIParts::oURIParts(const char* _Scheme, const char* _Authority, const char* _Path, const char* _Query, const char* _Fragment)
{
	strcpy_s(Scheme, oSAFESTR(_Scheme));
	strcpy_s(Authority, oSAFESTR(_Authority));
	strcpy_s(Path, oSAFESTR(_Path));
	strcpy_s(Query, oSAFESTR(_Query));
	strcpy_s(Fragment, oSAFESTR(_Fragment));
}

// Copies a begin/end iterator range into the specified destination as a string
static errno_t strcpy_s(char* _StrDestination, size_t _SizeofStrDestination, const char* _Start, const char* _End)
{
	if (!_StrDestination || !_Start || !_End) return EINVAL;
	const size_t sourceLength = std::distance(_Start, _End);
	if (_SizeofStrDestination < (1+sourceLength)) return ERANGE;
	memcpy(_StrDestination, _Start, sourceLength);
	_StrDestination[sourceLength] = 0;
	return 0;
}
template<size_t size> errno_t strcpy_s(char (&_StrDestination)[size], const char* _Start, const char* _End) { return strcpy_s(_StrDestination, size, _Start, _End); }

// http://tools.ietf.org/html/rfc3986#appendix-B
static regex reURI("^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?", std::tr1::regex_constants::optimize); // @oooii-tony: ok static (duplication won't affect correctness)
bool oURIDecompose(const char* _URIReference, char* _Scheme, size_t _SizeofScheme, char* _Authority, size_t _SizeofAuthority, char* _Path, size_t _SizeOfPath, char* _Query, size_t _SizeofQuery, char* _Fragment, size_t _SizeofFragment)
{
	if (_URIReference)
	{
		cmatch matches;
		regex_search(_URIReference, matches, reURI);
		if (matches.empty())
			return false;

		#define COPY(part, index) do { if (_##part && strcpy_s(_##part, _Sizeof##part, matches[index].first, matches[index].second)) return false; } while(false)
			COPY(Scheme, 2);
			COPY(Authority, 4);
			COPY(Query, 7);
			COPY(Fragment, 9);
		#undef COPY

		cmatch::reference ref =  matches[5];
		int skipLeadingSlash = (ref.first && *ref.first == '/') ? 1 : 0;
		if (_Path && strcpy_s(_Path, _SizeOfPath, ref.first+skipLeadingSlash, ref.second)) return false;
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

	#define SAFECAT(str) do {	errno_t ERR = strcat_s(_URIReference, _SizeofURIReference, str); if (ERR) return nullptr; } while(false)

	oASSERT(_Scheme < _URIReference || _Scheme >= (_URIReference + _SizeofURIReference), "Overlapping buffers not allowed");
	oASSERT(_Authority < _URIReference || _Authority >= (_URIReference + _SizeofURIReference), "Overlapping buffers not allowed");
	oASSERT(_Path < _URIReference || _Path >= (_URIReference + _SizeofURIReference), "Overlapping buffers not allowed");
	oASSERT(_Query < _URIReference || _Query >= (_URIReference + _SizeofURIReference), "Overlapping buffers not allowed");
	oASSERT(_Fragment < _URIReference || _Fragment >= (_URIReference + _SizeofURIReference), "Overlapping buffers not allowed");

	*_URIReference = 0;
	if (oSTRVALID(_Scheme))
	{
		SAFECAT(_Scheme);
		SAFECAT(":");
	}

	const bool makeLibxmlCompatible = true;
	bool libxml = makeLibxmlCompatible && !_memicmp("file", _Scheme, 4);
	bool uncSlashesAdded = false;
	if (libxml || (_Authority && *_Authority) || oIsUNCPath(_Path))
		SAFECAT("//");

	if (oSTRVALID(_Authority))
	{
		if (libxml)
		{
			SAFECAT("///");
			SAFECAT(_Authority);
			uncSlashesAdded = true;
		}

		else
			SAFECAT(_Authority);
	}

	const bool isWindows = true;
	if (!uncSlashesAdded && libxml && isWindows)
	{
		// if an absolute Path with no drive letter, add an extra slash
		if (_Path && oIsSeparator(_Path[0]) && !oIsSeparator(_Path[1]) && _Path[2] != ':')
			SAFECAT("/");
	}

	if (libxml && _Path && !oIsSeparator(_Path[0]))
		SAFECAT("/");

	size_t URIReferenceLength = strlen(_URIReference);
	if (!oCleanPath(_URIReference + URIReferenceLength, _SizeofURIReference - URIReferenceLength, _Path))
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

char* oURINormalize(char* _NormalizedURI, size_t _SizeofNormalizedURI, const char* _SourceURI)
{
	oURIParts parts;
	return oURIDecompose(_SourceURI, &parts) && oURIRecompose(_NormalizedURI, _SizeofNormalizedURI, parts) ? _NormalizedURI : nullptr;
}

// Ensure there's a leading slash, that all slashes are forward, and that all
// spaces are escaped (%20)
static char* oURIMakeCompatible(char* _CompatiblePath, size_t _SizeofCompatiblePath, const char* _Path)
{
	const bool isWindows = true;
	if (isWindows && _Path && isalpha(_Path[0]) && _Path[1] == ':')
	{
		*_CompatiblePath++ = '/';
		_SizeofCompatiblePath--;
	}

	char tmp[oMAX_URI];
	const char* src = _Path;

	if (isWindows)
	{
		if (oReplace(tmp, _Path, "\\", "/"))
			return nullptr;
		src = tmp;
	}

	return 0 == oReplace(_CompatiblePath, _SizeofCompatiblePath, src, " ", "%20") ? _CompatiblePath : nullptr;
}

template<size_t size> static char* oURIMakeCompatible(char (&_CompatiblePath)[size], const char* _Path) { return oURIMakeCompatible(_CompatiblePath, size, _Path); }

char* oURIFromAbsolutePath(char* _URI, size_t _SizeofURI, const char* _AbsolutePath)
{
	if (!_AbsolutePath) return false;
	if (*_AbsolutePath == 0)
	{
		*_URI = 0;
		return _URI;
	}

	char compatiblePath[_MAX_PATH];
	return oURIRecompose(_URI, _SizeofURI, "file", "", oURIMakeCompatible(compatiblePath, _AbsolutePath), "", "");
}

char* oURIFromRelativePath(char* _URIReference, size_t _SizeofURIReference, const char* _RelativePath)
{
	return oURIMakeCompatible(_URIReference, _SizeofURIReference, _RelativePath);
}

char* oURIToPath(char* _Path, size_t _SizeofPath, const char* _URI)
{
	/** <citation
		usage="Adaptation" 
		reason="This code seems to handle a lot of real-world compatibility issues, so rather than rediscover these issues, use the learnings from collada-dom." 
		author="Sony Computer Entertainment of America"
		description="http://sourceforge.net/projects/collada-dom"
		license="Shared Source License"
		licenseurl="http://research.scea.com/scea_shared_source_license.html"
	/>*/

	#define SAFECAT(str) do { errno_t ERR = strcat_s(_Path, _SizeofPath, str); if (ERR) return false; } while(false)

	*_Path = 0;
	if (!oSTRVALID(_URI))
		return _Path;
	oURIParts parts;
	if (!oURIDecompose(_URI, &parts))
		return nullptr;

	// If not a file Scheme, then it can't be converted
	if (!parts.Scheme || _stricmp(parts.Scheme, "file"))
		return nullptr;

	const bool isWindows = true;
	char* p = parts.Path;

	if (isWindows)
	{
		if (oSTRVALID( parts.Authority ))
		{
			SAFECAT("\\\\");
			SAFECAT(parts.Authority);
		}

		if (oIsUNCPath(_Path))
			p++;

		if (_Path[0] == '/' && _Path[2] == ':')
			p++;

		char* p2 = p;
		
		// convert forward to backslashes
		while (*p2)
		{
			if (*p2 == '/')
				*p2 = '\\';

			p2++;
		}
	}

	SAFECAT(p);
	if (oReplace(_Path, _SizeofPath, _Path, "%20", " "))
		return nullptr;

	if (*_URI && !*_Path)
		return nullptr;

	return _Path;

	#undef SAFECAT
}

char* oURIMakeRelativeToBase(char* _URIReference, size_t _SizeofURIReference, const char* _URIBase, const char* _URI)
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

	// advance till we find a segment that doesn't match
	const char *this_Path = d.Path;
	const char *relativeTo_Path = base_d.Path;
	const char *this_slash = this_Path;
	const char *relativeTo_slash = relativeTo_Path;

	while((*this_Path == *relativeTo_Path) && *this_Path)
	{
		if(*this_Path == '/')
		{
			this_slash = this_Path;
			relativeTo_slash = relativeTo_Path;
		}
		this_Path++;
		relativeTo_Path++;
	}

	if(this_slash == this_Path) // @oooii-kevin: If nothing is relative you can't make the path relative
		return nullptr;

	// Decide how many ../ segments are needed (FilePath should always end in a /)
	size_t segment_count = 0;
	relativeTo_slash++;
	while(*relativeTo_slash != 0)
	{
		if(*relativeTo_slash == '/')
			segment_count++;
		relativeTo_slash++;
	}

	char new_Path[_MAX_PATH];
	*new_Path = 0;

	for (size_t i = 0; i < segment_count; i++)
		if (strcat_s(new_Path, "../"))
			return nullptr;

	if (strcat_s(new_Path, this_slash))
		return nullptr;

	return oURIRecompose(_URIReference, _SizeofURIReference, d.Scheme, d.Authority, new_Path, d.Query, d.Fragment);
}

char* oURIMakeAbsoluteFromBase(char* _URIReference, size_t _SizeofURIReference, const char* _URIBase, const char* _URI)
{
	char* pFileBase;
	if (strcpy_s(_URIReference, _SizeofURIReference, _URIBase))
		return nullptr;
	pFileBase = oGetFilebase(_URIReference);

	if (!oURIToPath(pFileBase, oMAX_URI - strlen(_URIBase), _URI))
		return nullptr;

	char fullPath[oMAX_URI];
	return oURIFromAbsolutePath(_URIReference, _SizeofURIReference, oURIToPath(fullPath, _URIReference));
}

char* oURIEnsureFileExtension(char* _URIReferenceWithExtension, size_t _SizeofURIReferenceWithExtension, const char* _SourceURIReference, const char* _Extension)
{
	if (!oSTRVALID(_Extension))
	{
		if (strcpy_s(_URIReferenceWithExtension, _SizeofURIReferenceWithExtension, _SourceURIReference))
			return nullptr;
	}

	else
	{
		oURIParts parts;
		if (!oURIDecompose(_SourceURIReference, &parts))
			return nullptr;
		oReplaceFileExtension(parts.Path, _Extension);
		if (!oURIRecompose(_URIReferenceWithExtension, _SizeofURIReferenceWithExtension, parts))
			return nullptr;
	}

	return _URIReferenceWithExtension;
}

bool oFromString(char (*_pStrDestination)[oMAX_URI], const char* _StrSource)
{
	return 0 == strcpy_s(*_pStrDestination, oMAX_URI, _StrSource);
}

bool oFromString(oURIParts* _pURIParts, const char* _StrSource)
{
	return oURIDecompose(_StrSource, _pURIParts);
}

