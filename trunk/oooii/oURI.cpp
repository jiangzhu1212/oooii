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
#include <oooii/oURI.h>
#include <oooii/oErrno.h>
#include <oooii/oPath.h>
#include <oooii/oSTL.h>
#include <oooii/oString.h>
#include <stdlib.h>

using namespace std;
using namespace std::tr1;

namespace oURI {

Decomposition::Decomposition() { *Scheme = *Authority = *Path = *Query = *Fragment = 0; }
Decomposition::Decomposition(const Decomposition& _Other) { *this = _Other; }
Decomposition::Decomposition(const char* _Scheme, const char* _Authority, const char* _Path, const char* _Query, const char* _Fragment)
{
	*Scheme = *Authority = *Path = *Query = *Fragment = 0;
	if (_Scheme) strcpy_s(Scheme, _Scheme);
	if (_Authority) strcpy_s(Authority, _Authority);
	if (_Path) strcpy_s(Path, _Path);
	if (_Query) strcpy_s(Query, _Query);
	if (_Fragment) strcpy_s(Fragment, _Fragment);
}

const Decomposition& Decomposition::operator=(const Decomposition& _Other)
{
	strcpy_s(Scheme, _Other.Scheme);
	strcpy_s(Authority, _Other.Authority);
	strcpy_s(Path, _Other.Path);
	strcpy_s(Query, _Other.Query);
	strcpy_s(Fragment, _Other.Fragment);
	return *this;
}

bool Decomposition::operator==(const Decomposition& _Other)
{
	#define SAME(x) !_stricmp(x, _Other.x)
	return SAME(Scheme) && SAME(Authority) && SAME(Path) && SAME(Query) && SAME(Fragment);
	#undef SAME
}

// http://tools.ietf.org/html/rfc3986#appendix-B
static regex reURI("^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?", std::tr1::regex_constants::optimize); // @oooii-tony: ok static (duplication won't affect correctness)
errno_t Decompose( const char* _pURIReference, std::basic_string<char>* _pScheme /*= NULL*/, std::basic_string<char>* _pAuthority /*= NULL*/, std::basic_string<char>* _pPath /*= NULL*/, std::basic_string<char>* _pQuery /*= NULL*/, std::basic_string<char>* _pFragment /*= NULL*/ )
{
	cmatch matches;
	regex_search(_pURIReference, matches, reURI);
	if (matches.empty())
	{
		oSetLastError(EINVAL, "failed to match specified URI against URI pattern");
		return EINVAL;
	}

	#define COPY(part, index) if (_p##part) { *_p##part = std::basic_string<char>( matches[index].first, matches[index].second ); }
	COPY( Scheme, 2 );
	COPY( Authority, 4 );
	COPY( Query, 7);
	COPY( Fragment, 9 );

	if (_pPath)
	{
		cmatch::reference ref =  matches[5];
		int skipLeadingSlash = (ref.first && *ref.first == '/') ? 1 : 0;
		*_pPath = std::basic_string<char>( ref.first + skipLeadingSlash, ref.second );
	}
	
	#undef COPY

	return 0;
}


errno_t Decompose(const char* _URIReference, char* _pScheme, size_t _SizeofScheme, char* _pAuthority, size_t _SizeofAuthority, char* _pPath, size_t _SizeofPath, char* _pQuery, size_t _SizeofQuery, char* _pFragment, size_t _SizeofFragment)
{
	std::basic_string<char> Scheme;
	std::basic_string<char> Authority;
	std::basic_string<char> Path;
	std::basic_string<char> Query;
	std::basic_string<char> Fragment;

	errno_t err = Decompose(_URIReference, _pScheme ? &Scheme : NULL, _pAuthority ? &Authority : NULL, _pPath ? &Path : NULL, _pQuery ? &Query : NULL, _pFragment ? &Fragment : NULL );
	if( 0 != err )
		return err;

	#define COPY(part) if( _p##part ) { strcpy_s( _p##part, _Sizeof##part, oSAFESTR( ##part.c_str() ) ); }
	
	COPY( Scheme );
	COPY( Authority );
	COPY( Path );
	COPY( Query );
	COPY( Fragment );

	#undef COPY

	return 0;
}


errno_t Normalize(char* _NormalizedURI, size_t _SizeofNormalizedURI, const char* _SourceURI)
{
	Decomposition d;
	errno_t err = Decompose(_SourceURI, d);
	if (err) return err;
	return Recompose(_NormalizedURI, _SizeofNormalizedURI, d);
}

errno_t NormalizeBase(char* _NormalizedURIBase, size_t _SizeofNormalizedURIBase, const char* _SourceURIBase)
{
	char u[MAX_URI];
	errno_t err = oCleanPath(u, _SourceURIBase);
	if (err) return err;
	err = Normalize(_NormalizedURIBase, _SizeofNormalizedURIBase, u);
	if (!err)
		err = oEnsureFileSeparator(_NormalizedURIBase, _SizeofNormalizedURIBase);

	size_t len = strlen(_NormalizedURIBase);

	if (!err && len > 1 && oIsFileSeparator(_NormalizedURIBase[len-1]) && ':' == _NormalizedURIBase[len-2])
		err = strcat_s(_NormalizedURIBase, _SizeofNormalizedURIBase, "//");
	return err;
}

errno_t GetScheme(char* _Scheme, size_t _SizeofScheme, const char* _SourceURI)
{
	size_t SchemeLen = strcspn(_SourceURI, ":");
	if (SchemeLen == strlen(_SourceURI))
		return EINVAL;
	errno_t err = memcpy_s(_Scheme, _SizeofScheme-1, _SourceURI, SchemeLen);
	_Scheme[__min(SchemeLen, _SizeofScheme-1)] = 0;
	return err;
}

errno_t Recompose(char* _URIReference, size_t _SizeofURIReference, const char* _Scheme, const char* _Authority, const char* _Path, const char* _Query, const char* _Fragment)
{
	/** <citation
		usage="Adaptation" 
		reason="This code seems to handle a lot of real-world compatibility issues, so rather than rediscover these issues, use the learnings from collada-dom." 
		author="Sony Computer Entertainment of America"
		description="http://sourceforge.net/projects/collada-dom"
		license="Shared Source License"
		licenseurl="http://research.scea.com/scea_shared_source_license.html"
	/>*/

	#define SAFECAT(str) do \
	{	errno_t ERR = strcat_s(_URIReference, _SizeofURIReference, str); \
		if (ERR) return ERR; \
	} while(false)

	*_URIReference = 0;
	if (_Scheme && *_Scheme)
	{
		SAFECAT(_Scheme);
		SAFECAT(":");
	}

	const bool makeLibxmlCompatible = true;
	bool libxml = makeLibxmlCompatible && !_memicmp("file", _Scheme, 4);
	bool uncSlashesAdded = false;
	if (libxml || (_Authority && *_Authority) || oIsUNCPath(_Path))
		SAFECAT("//");

	if (_Authority && *_Authority)
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
		if (_Path && oIsFileSeparator(_Path[0]) && !oIsFileSeparator(_Path[1]) && _Path[2] != ':')
			SAFECAT("/");
	}

	if (libxml && _Path && !oIsFileSeparator(_Path[0]))
		SAFECAT("/");

	SAFECAT(_Path);

	if (_Query && *_Query)
	{
		SAFECAT("?");
		SAFECAT(_Query);
	}

	if (_Fragment && *_Fragment)
	{
		SAFECAT("#");
		SAFECAT(_Fragment);
	}

	return 0;
	#undef SAFECAT
}

errno_t EnsureFileExtension(char* _URIReferenceWithExtension, size_t _SizeofURIReferenceWithExtension, const char* _SourceURIReference, const char* _Extension)
{
	errno_t err = 0;
	if (!_Extension || !*_Extension)
		err = strcpy_s(_URIReferenceWithExtension, _SizeofURIReferenceWithExtension, _SourceURIReference);
	else
	{
		Decomposition d;
		err = Decompose(_SourceURIReference, d);
		if (!err) err = oReplaceFileExtension(d.Path, _Extension);
		if (!err) err = Recompose(_URIReferenceWithExtension, _SizeofURIReferenceWithExtension, d);
	}

	return err;
}

static errno_t uriCompatiblePath(char* _CompatiblePath, size_t _SizeofCompatiblePath, const char* _Path)
{
	const bool isWindows = true;
	if (isWindows && _Path && isalpha(_Path[0]) && _Path[1] == ':')
	{
		*_CompatiblePath++ = '/';
		_SizeofCompatiblePath--;
	}

	char tmp[MAX_URI];
	const char* src = _Path;

	if (isWindows)
	{
		errno_t err = oReplace(tmp, _Path, "\\", "/");
		if (err) return err;
		src = tmp;
	}

	return oReplace(_CompatiblePath, _SizeofCompatiblePath, src, " ", "%20");
}

template<size_t size> inline errno_t uriCompatiblePath(char (&_CompatiblePath)[size], const char* _Path) { return uriCompatiblePath(_CompatiblePath, size, _Path); }

errno_t URIFromAbsolutePath(char* _URI, size_t _SizeofURI, const char* _AbsolutePath)
{
	if (*_AbsolutePath == 0)
	{
		*_URI = 0;
		return 0;
	}
	errno_t err = uriCompatiblePath(_URI, _SizeofURI, _AbsolutePath);
	if (err) return err;

	// Absolute paths are cleaned on import
	char path[MAX_URI];
	oCleanPath(path, _URI);
	return Recompose(_URI, _SizeofURI, "file", "", path, "", "");
}

errno_t URIFromRelativePath(char* _URIReference, size_t _SizeofURIReference, const char* _RelativePath)
{
	return uriCompatiblePath(_URIReference, _SizeofURIReference, _RelativePath);
}

errno_t URIToPath(char* _Path, size_t _SizeofPath, const char* _URI)
{
	/** <citation
		usage="Adaptation" 
		reason="This code seems to handle a lot of real-world compatibility issues, so rather than rediscover these issues, use the learnings from collada-dom." 
		author="Sony Computer Entertainment of America"
		description="http://sourceforge.net/projects/collada-dom"
		license="Shared Source License"
		licenseurl="http://research.scea.com/scea_shared_source_license.html"
	/>*/

	#define SAFECAT(str) do \
	{	errno_t ERR = strcat_s(_Path, _SizeofPath, str); \
		if (ERR) return ERR; \
	} while(false)

	*_Path = 0;
	Decomposition d;
	errno_t err = Decompose(_URI, d);
	if (err) return err;

	// If not a file Scheme, then it can't be converted
	if (!d.Scheme || _stricmp(d.Scheme, "file"))
		return EINVAL;

	const bool isWindows = true;
	char* p = d.Path;

	if (isWindows)
	{
		if (oSTRNEXISTS( d.Authority ))
		{
			SAFECAT("\\\\");
			SAFECAT(d.Authority);
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
	err = oReplace(_Path, _SizeofPath, _Path, "%20", " ");
	if (err) return err;

	if (*_URI && !*_Path)
		return EINVAL;

	return 0;

	#undef SAFECAT
}

errno_t MakeURIRelativeToURIBase(char* _URIReference, size_t _SizeofURIReference, const char* _URIBase, const char* _URI)
{
	/** <citation
		usage="Adaptation" 
		reason="This code seems to handle a lot of real-world compatibility issues, so rather than rediscover these issues, use the learnings from collada-dom." 
		author="Sony Computer Entertainment of America"
		description="http://sourceforge.net/projects/collada-dom"
		license="Shared Source License"
		licenseurl="http://research.scea.com/scea_shared_source_license.html"
	/>*/

	Decomposition base_d, d;
	errno_t err = Decompose(_URIBase, base_d);
	if (!err)
		err = Decompose(_URI, d);

	// Can only do this function if both URIs have the same Scheme and Authority
	if (_stricmp(base_d.Scheme, d.Scheme) || _stricmp(base_d.Authority, d.Authority))
		return EINVAL;

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

	// Decide how many ../ segments are needed (FilePath should always end in a /)
	size_t segment_count = 0;
	relativeTo_slash++;
	while(*relativeTo_slash != 0)
	{
		if(*relativeTo_slash == '/')
			segment_count++;
		relativeTo_slash++;
	}
	this_slash++;

	char new_Path[_MAX_PATH];
	*new_Path = 0;

	for (size_t i = 0; i < segment_count; i++)
	{
		errno_t e = strcat_s(new_Path, "../");
		if (e)
		{
			err = e;
			break;
		}
	}

	if (!err)
		err = strcat_s(new_Path, this_slash);

	if (!err)
		err = Recompose(_URIReference, _SizeofURIReference, d.Scheme, d.Authority, new_Path, d.Query, d.Fragment);

	return err;
}

errno_t MakeURIAbsoluteFromURIBase(char* _URIReference, size_t _SizeofURIReference, const char* _URIBase, const char* _URI)
{
	errno_t err = 0;

	char* pFileBase;
	strcpy_s(_URIReference, _SizeofURIReference, _URIBase);
	pFileBase = oGetFilebase( _URIReference );

	err = oURI::URIToPath( pFileBase, oURI::MAX_URI - strlen(_URIBase), _URI );
	char fullPath[oURI::MAX_URI];
	if(!err)
		err = oURI::URIToPath( fullPath, _URIReference );
	if(!err)
		err = oURI::URIFromAbsolutePath(_URIReference, _SizeofURIReference, fullPath); //mostly to clean the path.
	return err;
}

} // namespace oURI

// uri case
template<> errno_t oFromString( char(* _pValue )[oURI::MAX_URI], const char* _StrSource)
{
	strcpy_s( *_pValue, oURI::MAX_URI, _StrSource );
	return 0;
}

template<> errno_t oFromString( oURI::Decomposition* _pDecomposition, const char* _StrSource)
{
	return oURI::Decompose(_StrSource, (*_pDecomposition));
}
