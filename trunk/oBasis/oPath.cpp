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
#include <oBasis/oPath.h>
#include <oBasis/oAssert.h>
#include <oBasis/oMacros.h>
#include <cstring>
#include <cctype>

template<typename T> T* oGetFileExtensionT(T* _Path)
{
	size_t len = strlen(_Path);
	T* cur = _Path + len-1;
	while (cur >= _Path && !(oIsSeparator(*cur) || *cur == ':'))
	{
		if (*cur == '.')
			return cur;
		cur--;
	}

	return _Path + len;
}

char* oGetFileExtension(char* _Path)
{
	return oGetFileExtensionT(_Path);
}

const char* oGetFileExtension(const char* _Path)
{
	return oGetFileExtensionT(_Path);
}

template<typename T> T* oGetFileBaseT(T* _Path)
{
	T* cur = _Path + strlen(_Path)-1;
	while (cur >= _Path && !(oIsSeparator(*cur) || *cur == ':'))
		cur--;
	cur++;
	return cur;
}

const char* oGetFilebase(const char* _Path)
{
	return oGetFileBaseT(_Path);
}

char* oGetFilebase(char* _Path)
{
	return oGetFileBaseT(_Path);
}

char* oGetFilebase(char* _StrDestination, size_t _SizeofStrDestination, const char* _Path)
{
	if (_StrDestination)
		*_StrDestination = 0;

	if (oSTRVALID(_Path))
	{
		const char* start = oGetFilebase(_Path);
		const char* end = oGetFileExtension(_Path);
		size_t len = end - start;
		oASSERT(len < _SizeofStrDestination, "The buffer specified to receive a filebase is not large enough (%d bytes) for the base contained in path %s", _SizeofStrDestination, oSAFESTRN(_Path));
		memcpy(_StrDestination, start, len);
		_StrDestination[len] = 0;
	}

	return _StrDestination;
}

char* oReplaceFileExtension(char* _Path, size_t _SizeofPath, const char* _Extension)
{
	char* start = oGetFileExtension(_Path);
	if (start)
		*start = 0;
	else
	{
		errno_t err = strcat_s(_Path, _SizeofPath, ".");
		oASSERT(!err, "Error concatenating '.' to path string %s", oSAFESTRN(_Path));
	}
	
	errno_t err = strcat_s(_Path, _SizeofPath, _Extension);
	oASSERT(!err, "Error concatenating %s to path string %s", oSAFESTRN(_Extension), oSAFESTRN(_Path));
	return _Path;
}

char* oTrimFilename(char* _Path, bool _IgnoreTrailingSeparator)
{
	char* cur = _Path + strlen(_Path);
	// Handle the case where a path ends in a separator, indicating that it is
	// a directory path, so trim back another step.
	if (_IgnoreTrailingSeparator && oIsSeparator(*(cur-1)))
		cur -= 2;
	while (cur >= _Path && !(oIsSeparator(*cur) || *cur == ':'))
		cur--;
	cur++;
	*cur = 0;
	return _Path;
}

char* oTrimFileExtension(char* _Path)
{
	char* cur = _Path + strlen(_Path);
	while (cur >= _Path && *cur != '.')
		--cur;
	*cur = 0;
	return _Path;
}

char* oEnsureSeparator(char* _Path, size_t _SizeofPath, char _FileSeparator)
{
	size_t len = strlen(_Path);
	char* cur = _Path + len-1;
	if (!oIsSeparator(*cur) && _SizeofPath)
	{
		oASSERT((len+1) < _SizeofPath, "Path string does not have the capacity to have a separate appended (%s)", oSAFESTRN(_Path));
		*(++cur) = _FileSeparator;
		*(++cur) = 0;
	}

	return 0;
}

char* oCleanPath(char* _CleanedPath, size_t _SizeofCleanedPath, const char* _SourcePath, char _FileSeparator)
{
	if (!_CleanedPath)
		return nullptr;

	char* w = _CleanedPath;
	const char* r = _SourcePath;

	if (!r)
	{
		*_CleanedPath = 0;
		return _CleanedPath;
	}

	while (*r && size_t(w - _CleanedPath) < _SizeofCleanedPath)
	{
		// skip all separators that are right in a row, except for the first ones that would constitute a valid UNC path
		if (oIsSeparator(*r) && r != _SourcePath)
		{
			while (oIsSeparator(*r)) r++; // move past any separators right in a row
			*w++ = _FileSeparator; // write the separator we found above
		}

		else if (*r == '.' && oIsSeparator(*(r+1))) // is dot, skip reading, it's a noop
			r++;

		else if (*r == '.' && *(r+1) == '.' && oIsSeparator(*(r+2))) // is dot dot, so rewind the writing to overwrite last dir
		{
			if (r > _SourcePath)
			{
				r += 2; // move past reading
				while (!oIsSeparator(*w) && w != _CleanedPath) w--; // find the end of the last dir
				if( w != _CleanedPath ) w--;// move past it
				while (!oIsSeparator(*w) && w != _CleanedPath) w--; // move to start of last dir
			}

			else // leave as a relative path, so copy it over
			{
				*w++ = *r++;
				*w++ = *r++;
				*w++ = *r++;
			}
		}

		else
			*w++ = *r++;
	}

	*w = 0;
	return _CleanedPath;
}

template<typename T> static bool oXOr(const T& _A, const T& _B)
{
	return (_A && !_B) || (!_A && _B);
}

template<typename T, typename Pr> static bool oXOr(const T& _A, const T& _B, Pr _Pred)
{
	return (_Pred(_A) && !_Pred(_B)) || (!_Pred(_A) && _Pred(_B));
}

size_t oGetCommonBaseLength(const char* _Path1, const char* _Path2, bool _CaseInsensitive)
{
	size_t lastSeparatorLen = 0;
	size_t len = 0;
	if (oSTRVALID(_Path1) && oSTRVALID(_Path2))
	{
		while (_Path1[len] && _Path2[len])
		{
			int a, b;
			if (_CaseInsensitive)
			{
				a = toupper(_Path1[len]);
				b = toupper(_Path2[len]);
			}

			else
			{
				a = _Path1[len];
				b = _Path2[len];
			}

			if (a != b || oXOr(a, b, oIsSeparator))
				break;

			if (oIsSeparator(a) || oIsSeparator(b))
				lastSeparatorLen = len;
			
			len++;
		}
	}

	return lastSeparatorLen;
}

char* oMakeRelativePath(char* _StrDestination, size_t _SizeofStrDestination, const char* _FullPath, const char* _RootPath, char _FileSeparator)
{
	if (!_StrDestination)
		return nullptr;
	*_StrDestination = 0;

	size_t rootLength = oGetCommonBaseLength(_FullPath, _RootPath);
	if (rootLength)
	{
		size_t nSeperators = 0;
		size_t index = rootLength - 1;
		while (_RootPath[index])
		{
			if (oIsSeparator(_RootPath[index]) && 0 != _RootPath[index + 1] )
				nSeperators++;
			index++;
		}

		for (size_t i = 0; i < nSeperators; i++)
		{
			errno_t err = strcat_s(_StrDestination + 3*i, _SizeofStrDestination - 3*i, _FileSeparator == '/' ? "../" : "..\\");
			oASSERT(!err, "Error concatenating ..'s to relative path from %s", oSAFESTRN(_FullPath));
		}

		errno_t err = strcat_s(_StrDestination+3*nSeperators, _SizeofStrDestination-3*nSeperators, _FullPath + rootLength);
		oASSERT(!err, "Error concatenating relative path extracted from %s", oSAFESTRN(_FullPath));
	}

	return _StrDestination;
}

// $(CitedCodeBegin)
static inline bool JONATHAN_MILES_WildcardMatch(const char *String1, const char *String2)
{
	/** $(Citation)
		<citation>
			<usage type="Implementation" />
			<author name="Jonathan Miles" />
			<description url="http://lists.eggheads.org/pipermail/eggdev/2000-August.txt.gz" />
			<license type="*** Assumed Public Domain ***" url="http://lists.eggheads.org/pipermail/eggdev/2000-August.txt.gz" />
		</citation>
	*/

/*	Case insensitive wildcard match... the string with wildcards is the
	first arg. The string to test against is the second arg. This
	implentation was written by Jon Miles <jon@zetnet.net>. I've yet to see
	another wild-match implementation as simple as this... there's gotta be
	a bug or flaw in it somewhere?
*/

  bool    bStar = false;
  /* Set to true when processing a wildcard * in String1 */
  char   *StarPos = 0;	
  /* Set this to the text just after the
  last star, so we can resurrect String1
  when we find that String2 isnt matching
  after a * earlier on.
  */

  /*	  Loop through each character in the string sorting out the
  wildcards. If a ? is found then just increment both strings. If
  a * is found then increment the second string until the first
  character matches, then continue as normal. This is where the
  algorithm gets a little more complicated. As matching
  *zetnet.co.uk to zdialup.zetnet.co.uk would incorrectly return
  false. To solve this i'm keeping a pointer to the string just
  after the last * so that when the two next characters from each
  string dont match, we reset String1 back to the string we
  should be looking for.
  */
  while(true)
  {
    switch(*String1)
    {
    case '*':
      {
        String1++;
        bStar = true;
        StarPos = (char *)String1;
        break;
      }

    case '?':
      {
        String1++;
        String2++;
        break;
      }

    case 0:	//	NULL terminator
      {
        if(!String2[0])
        {
          /* End of both strings, so it matches. */
          return true;
        }
        if(*(String1-1)=='*')
        {
          /* The last character in String1 was a '*', so it matches. */
          return true;
        }

        /* End of one string, but not the other, fails match. */
        return false;
        break;
      }

    default:
      {
        if(toupper(*String1)!=toupper(*String2))
        {
          if(!String2[0])
          {
            /* End of String2 but not String1, doesnt match. */
            return false;
          }
          if(bStar)
          {
            String2++;
            if(!String2[0])
              return false;

            /* Reset String1 to just after the last '*'. */
            String1 = StarPos;
          }
          else
            return false;
        }
        else
        {
          String1++;
          String2++;
        }
        break;
      }
    }
  }

  //	shouldn't get here
  return false;
}
// $(CitedCodeEnd)

bool oMatchesWildcard(const char* _Wildcard, const char* _Path)
{
	// You can get the implementation I use internally here:
	// http://lists.eggheads.org/pipermail/eggdev/2000-August.txt.gz
	return JONATHAN_MILES_WildcardMatch(_Wildcard, _Path);
}

char* oFindInPath(char* _StrDestination, size_t _SizeofStrDestination, const char* _SearchPaths, const char* _RelativePath, const char* _DotPath, oFUNCTION<bool(const char* _Path)> _PathExists)
{
	if (!_StrDestination || !_SearchPaths) return nullptr;
	const char* cur = _SearchPaths;

	while (*cur)
	{
		cur += strspn(cur, oWHITESPACE);
		if (*cur == ';')
		{
			cur++;
			continue;
		}

		char* dst = _StrDestination;
		char* end = dst + _SizeofStrDestination - 1;
		if (*cur == '.' && oSTRVALID(_DotPath))
		{
			if (0 != strcpy_s(_StrDestination, _SizeofStrDestination, _DotPath))
				return nullptr;
			oEnsureSeparator(_StrDestination, _SizeofStrDestination);
			dst = _StrDestination + strlen(_StrDestination);
		}

		while (dst < end && *cur && *cur != ';')
			*dst++ = *cur++;
		while (isspace(*(--dst))); // empty
		if (!oIsSeparator(*dst))
			*(++dst) = '/';
		oASSERT(dst < end, "");
		*(++dst) = 0;
		if (0 != strcat_s(_StrDestination, _SizeofStrDestination, _RelativePath))
			return nullptr;
		if (_PathExists(_StrDestination))
			return _StrDestination;
		if (*cur == 0)
			break;
		cur++;
	}

	*_StrDestination = 0;
	return nullptr;
}

