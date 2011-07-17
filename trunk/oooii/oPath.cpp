// $(header)
#include <oooii/oPath.h>
#include <oooii/oAssert.h>
#include <oooii/oErrno.h>
#include <oooii/oFile.h>
#include <oooii/oRef.h>
#include <oooii/oStdio.h>
#include <oooii/oString.h>
#include <oooii/oThreading.h>
#include <time.h>

template<typename T> T* GetFileBaseT(T* _Path)
{
	T* cur = _Path + strlen(_Path)-1;
	while (cur >= _Path && !(oIsFileSeparator(*cur) || *cur == ':'))
		cur--;
	cur++;
	return cur;
}

const char* oGetFilebase(const char* _Path){ return GetFileBaseT(_Path); }
char* oGetFilebase(char* _Path) { return GetFileBaseT(_Path); }
errno_t oGetFilebase(char* _Filebase, size_t _SizeofFilebase, const char* _Path)
{
	errno_t err = 0;
	const char* start = oGetFilebase(_Path);
	const char* end = oGetFileExtension(_Path);

	size_t len = end - start;
	if (len > _SizeofFilebase-1)
	{
		len = _SizeofFilebase-1;
		err = ERANGE;
	}

	memcpy(_Filebase, start, len);
	_Filebase[len] = 0;
	return 0;
}

template<typename T> T* GetFileExtension(T* _Path)
{
	size_t len = strlen(_Path);
	T* cur = _Path + len-1;
	while (cur >= _Path && !(oIsFileSeparator(*cur) || *cur == ':'))
	{
		if (*cur == '.')
			return cur;
		cur--;
	}

	return _Path + len;
}

char* oGetFileExtension(char* _Path) { return GetFileExtension(_Path); }
const char* oGetFileExtension(const char* _Path) { return GetFileExtension(_Path); }

errno_t oReplaceFileExtension(char* _Path, size_t _SizeofPath, const char* _Extension)
{
	errno_t err = 0;
	char* start = oGetFileExtension(_Path);
	if (start)
		*start = 0;
	else
		err = strcat_s(_Path, _SizeofPath, ".");
	return strcat_s(_Path, _SizeofPath, _Extension);
}

char* oTrimFilename(char* _Path)
{
	char* cur = _Path + strlen(_Path);
	while (cur >= _Path && !(oIsFileSeparator(*cur) || *cur == ':'))
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

errno_t oEnsureFileSeparator(char* _Path, size_t _SizeofPath)
{
	size_t len = strlen(_Path);
	char* cur = _Path + len-1;
	if (!oIsFileSeparator(*cur) && _SizeofPath)
	{
		if (len >= _SizeofPath-1)
			return ERANGE;
		*(++cur) = '/';
		*(++cur) = 0;
	}

	return 0;
}

errno_t oCleanPath(char* _CleanedPath, size_t _SizeofCleanedPath, const char* _SourcePath, char _FileSeparator)
{
	char* w = _CleanedPath;
	const char* r = _SourcePath;

	while (*r && (size_t)std::distance(_CleanedPath, w) < _SizeofCleanedPath)
	{
		// skip all separators that are right in a row, except for the first ones that would constitute a valid UNC path
		if (oIsFileSeparator(*r) && r != _SourcePath)
		{
			while (oIsFileSeparator(*r)) r++; // move past any separators right in a row
			*w++ = _FileSeparator; // write the separator we found above
		}

		else if (*r == '.' && oIsFileSeparator(*(r+1))) // is dot, skip reading, it's a noop
			r++;

		else if (*r == '.' && *(r+1) == '.' && oIsFileSeparator(*(r+2))) // is dot dot, so rewind the writing to overwrite last dir
		{
			r += 2; // move past reading
			while (!oIsFileSeparator(*w)) w--; // find the end of the last dir
			w--;// move past it
			while (!oIsFileSeparator(*w)) w--; // move to start of last dir
		}

		else
			*w++ = *r++;
	}

	*w = 0;
	return 0;
}

errno_t oGetLogFilePath(char* _StrDestination, size_t _SizeofStrDestination, const char* _ExeSuffix)
{
	char newExtension[128];
	time_t theTime;
	time(&theTime);
	tm t;
	localtime_s(&t, &theTime);

	char* ne = newExtension;
	ne += sprintf_s(ne, oCOUNTOF(newExtension) - std::distance(newExtension, ne), "%s%s_", _ExeSuffix ? "_" : "", oSAFESTR(_ExeSuffix));
	ne += strftime(ne, oCOUNTOF(newExtension) - std::distance(newExtension, ne), "%Y-%m-%d-%H-%M-%S", &t);
	ne += sprintf_s(ne, oCOUNTOF(newExtension) - std::distance(newExtension, ne), "-%i", oGetCurrentProcessID() );
	strcpy_s(ne, oCOUNTOF(newExtension) - std::distance(newExtension, ne), ".txt");

	if (!oGetExePath(_StrDestination, _SizeofStrDestination))
		return oGetLastError();

	errno_t err = oReplaceFileExtension(_StrDestination, _SizeofStrDestination, newExtension);
	if (err) return err;
	return 0;
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

bool oFindInPath(char* _ResultingFullPath, size_t _SizeofResultingFullPath, const char* _SearchPath, const char* _RelativePath, const char* _DotPath, oPATH_EXISTS_FUNCTION _PathExists)
{
	if (!_ResultingFullPath || !_SearchPath) return false;
	const char* cur = _SearchPath;

	while (*cur)
	{
		cur += strspn(cur, oWHITESPACE);

		if(*cur == ';')
		{
			cur++;
			continue;
		}

		char* dst = _ResultingFullPath;
		char* end = dst + _SizeofResultingFullPath - 1;
		if (*cur == '.' && _DotPath && *_DotPath) // relative path, use cwd
		{
			if (0 != strcpy_s(_ResultingFullPath, _SizeofResultingFullPath, _DotPath))
				return false;
			oEnsureFileSeparator(_ResultingFullPath, _SizeofResultingFullPath);
			dst = _ResultingFullPath + strlen(_ResultingFullPath);
		}

		while (dst < end && *cur && *cur != ';')
			*dst++ = *cur++;
		while (isspace(*(--dst))); // empty
		if (!oIsFileSeparator(*dst))
			*(++dst) = '/';
		oASSERT(dst < end, "");
		*(++dst) = 0;
		if (0 != strcat_s(_ResultingFullPath, _SizeofResultingFullPath, _RelativePath))
			return false;
		if (_PathExists(_ResultingFullPath))
			return true;
		if (*cur == 0)
			break;
		cur++;
	}

	*_ResultingFullPath = 0;
	oSetLastError(ENOENT, "Cannot find %s in search path %s", _RelativePath, _SearchPath);
	return false;
}

bool oFindInSysPath(char* _ResultingFullPath, size_t _SizeofResultingFullPath, oSYSPATH _SysPath, const char* _RelativePath, const char* _DotPath, oPATH_EXISTS_FUNCTION _PathExists)
{
	bool success = false;
	if (oGetSysPath(_ResultingFullPath, _SizeofResultingFullPath, _SysPath) && *_ResultingFullPath)
	{
		size_t len = strlen(_ResultingFullPath);
		success = 0 == strcpy_s(_ResultingFullPath + len, _SizeofResultingFullPath - len, _RelativePath);
		if (success)
			success = _PathExists(_ResultingFullPath);
	}

	return success;
}

bool oFindPath(char* _ResultingFullPath, size_t _SizeofResultingFullPath, const char* _RelativePath, const char* _DotPath, const char* _ExtraSearchPath, oPATH_EXISTS_FUNCTION _PathExists)
{
	if (oIsFullPath(_RelativePath) && _PathExists(_RelativePath) )
		return 0 == strcpy_s(_ResultingFullPath, _SizeofResultingFullPath, _RelativePath);

	bool success = oFindInSysPath(_ResultingFullPath, _SizeofResultingFullPath, oSYSPATH_APP, _RelativePath, _DotPath, _PathExists);
	if (!success) success = oFindInSysPath(_ResultingFullPath, _SizeofResultingFullPath, oSYSPATH_CWD, _RelativePath, _DotPath, _PathExists);
	if (!success) success = oFindInSysPath(_ResultingFullPath, _SizeofResultingFullPath, oSYSPATH_SYS, _RelativePath, _DotPath, _PathExists);
	if (!success) success = oFindInSysPath(_ResultingFullPath, _SizeofResultingFullPath, oSYSPATH_OS, _RelativePath, _DotPath, _PathExists);
	if (!success)
	{
		char appPath[_MAX_PATH];
		if (!oFindInSysPath(appPath, oSYSPATH_CWD, _RelativePath, _DotPath, _PathExists))
		{
			char* envPath;
			size_t envPathSize;
			success = 0 == _dupenv_s(&envPath, &envPathSize, "PATH");
			if (success) success = oFindInPath(_ResultingFullPath, _SizeofResultingFullPath, envPath, _RelativePath, appPath, _PathExists);
			free(envPath);
		}

		if (!success) success = oFindInPath(_ResultingFullPath, _SizeofResultingFullPath, _ExtraSearchPath, _RelativePath, appPath, _PathExists);
	}

	return success;
}

size_t oExtractCommonPath(const char* _path1, const char* _path2)
{
	size_t lastSeperatorIndex = 0;
	size_t index = 0;
	while(_path1[index] !=0 && _path2[index] != 0 && index < MAX_PATH)
	{
		if((_path1[index] != _path2[index]) && !(oIsFileSeparator(_path1[index]) && oIsFileSeparator(_path2[index])))
			break;
		if(oIsFileSeparator(_path1[index]))
			lastSeperatorIndex = index;
		index++;
	}
	++lastSeperatorIndex; //include the final trailing path seperator in the result;
	return lastSeperatorIndex;
}

void oMakeRelativePath(char* _relativePath, const char* _fullPath, const char* _referencePath)
{
	size_t pathIndex = oExtractCommonPath(_fullPath, _referencePath);

	size_t numPathSeperators = 0;

	size_t index = pathIndex - 1;
	while(_referencePath[index] !=0 && index < MAX_PATH)
	{
		if(oIsFileSeparator(_referencePath[index]) && 0 != _referencePath[index + 1] )
			numPathSeperators++;
		index++;
	}
	_relativePath[0] = '\0';
	for (size_t i = 0;i < numPathSeperators;++i)
	{
		strcat_s(_relativePath+3*i, MAX_PATH - 3*i, "..\\");
	}
	strcat_s(_relativePath+3*numPathSeperators, MAX_PATH-3*numPathSeperators, _fullPath+pathIndex);
}

size_t oCommonPath( const char* _path1, const char* _path2 )
{
	size_t count = __min(strlen(_path1), strlen(_path2) );
	for( size_t i = 0; i < count; ++i )
	{
		char a = _path1[i];
		char b = _path2[i];
		if( 0 != _strnicmp( &a, &b, 1 ) )
		{
			if( !oIsFileSeparator(a) || !oIsFileSeparator(b) )
			{
				// Paths were the same till we reached this, so backup
				size_t j = i;
				for(; j > 0; --j )
				{
					char c = _path1[j];
					if( oIsFileSeparator(c) || c == ':' )
						return j + 1;
				}
				return 0;
			}
		}
	}

	return count;
}
