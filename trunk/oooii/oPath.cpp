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
#include "pch.h"
#include <oooii/oWindows.h>
#include <oooii/oPath.h>
#include <oooii/oAssert.h>
#include <oooii/oErrno.h>
#include <oooii/oP4.h>
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

errno_t oGetExePath(char* _ExePath, size_t _SizeofExePath)
{
	errno_t err = 0;

	#ifdef UNICODE
		wchar_t path[_MAX_PATH];
		DWORD length = GetModuleFileName(GetModuleHandle(0), path, oCOUNTOF(path));
		if (length == oCOUNTOF(path) && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			err = STRUNCATE;
		oStrConvert(_ExePath, _SizeofExePath, path);
	#else
		DWORD length = GetModuleFileName(GetModuleHandle(0), _ExePath, static_cast<DWORD>(_SizeofExePath));
		if (length == (DWORD)_SizeofExePath && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			err = STRUNCATE;
	#endif

	return err;
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
	ne += sprintf_s(ne, oCOUNTOF(newExtension) - std::distance(newExtension, ne), "-%i", oTimer() );
	strcpy_s(ne, oCOUNTOF(newExtension) - std::distance(newExtension, ne), ".txt");

	errno_t err = oGetExePath(_StrDestination, _SizeofStrDestination);
	if (err) return err;

	err = oReplaceFileExtension(_StrDestination, _SizeofStrDestination, newExtension);
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

	return false;
}

bool GetSVNRoot(char* _RootPath, size_t _SizeofRootPath)
{
	return false;
}

bool oGetSysPath(char* _StrSysPath, size_t _SizeofStrSysPath, oSYSPATH _SysPath)
{
	bool success = true;
	DWORD nElements = static_cast<DWORD>(_SizeofStrSysPath);

	switch (_SysPath)
	{
		case oSYSPATH_APP: GetModuleFileNameA(GetModuleHandle(0), _StrSysPath, nElements); *oGetFilebase(_StrSysPath) = 0; break;
		case oSYSPATH_CWD: GetCurrentDirectoryA(nElements, _StrSysPath); break;
		case oSYSPATH_SYS: GetSystemDirectoryA(_StrSysPath, nElements); break;
		case oSYSPATH_OS: GetWindowsDirectoryA(_StrSysPath, nElements); break;
		case oSYSPATH_P4ROOT:
		{
			oP4::CLIENT_SPEC cspec;
			success = oP4::GetClientSpec(&cspec);
			if (success)
				strcpy_s(_StrSysPath, _SizeofStrSysPath, cspec.Root);
			break;
		}

		case oSYSPATH_DEV:
		{
			// fixme: find a better way to do this...
			GetModuleFileNameA(GetModuleHandle(0), _StrSysPath, nElements);
			success = S_OK == GetLastError();
			*oGetFilebase(_StrSysPath) = 0;
			success = 0 == strcat_s(_StrSysPath, _SizeofStrSysPath, "../../../"); // assumes $DEV/bin/$PLATFORM/$BUILDTYPE
			break;
		}

		case oSYSPATH_COMPILER_INCLUDES:
		{
			// @oooii-tony: Yes, sorta hard-coded but better than trying to get at this
			// directory elsewhere in user code.
			success = oGetEnvironmentVariable(_StrSysPath, _SizeofStrSysPath, "VS90COMNTOOLS");
			if (success)
			{
				oEnsureFileSeparator(_StrSysPath, _SizeofStrSysPath);
				strcat_s(_StrSysPath, _SizeofStrSysPath, "../../VC/include/");
				oCleanPath(_StrSysPath, _SizeofStrSysPath, _StrSysPath);
			}

			else
				oSetLastError(ENOENT, "Failed to find compiler include path becayse env var VS90COMNTOOLS does not exist");

			break;
		}

		case oSYSPATH_TMP:
		{
			DWORD len = GetTempPathA(nElements, _StrSysPath);
			if (len > 0 && len <= MAX_PATH)
				break; // otherwise use the desktop (pass through to next case)
		}

		case oSYSPATH_DESKTOP_ALLUSERS:
		case oSYSPATH_DESKTOP:
		{
			int folder = _SysPath == oSYSPATH_DESKTOP ? CSIDL_DESKTOPDIRECTORY : CSIDL_COMMON_DESKTOPDIRECTORY;
			if (nElements < MAX_PATH)
				oTRACE("WARNING: Getting desktop as a system path might fail because the specified buffer is smaller than the platform assumes.");
			if (!SHGetSpecialFolderPathA(0, _StrSysPath, folder, FALSE))
				success = false;
			break;
		}

		default: oASSUME(0);
	}

	if (success)
	{
		oEnsureFileSeparator(_StrSysPath, _SizeofStrSysPath);
		success = 0 == oCleanPath(_StrSysPath, _SizeofStrSysPath, _StrSysPath);
	}
	return success;
}

bool oSetCWD(const char* _StrCWD)
{
	return !!SetCurrentDirectoryA(_StrCWD);
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
	bool success = true;

	if (oIsFullPath(_ResultingFullPath))
		success = 0 == strcpy_s(_ResultingFullPath, _SizeofResultingFullPath, _RelativePath);
	else
	{
		success = oFindInSysPath(_ResultingFullPath, _SizeofResultingFullPath, oSYSPATH_APP, _RelativePath, _DotPath, _PathExists);
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

		if (!success)
			success = 0 == strcpy_s(_ResultingFullPath, _SizeofResultingFullPath, _RelativePath);
	}

	return success;
}

void oCreateTempPath( char* tempPath, size_t tempPathLength )
{
	oASSERT( tempPathLength < (size_t)std::numeric_limits<DWORD>::max(), "Path is too long!" );
	GetTempPath( (DWORD)tempPathLength, tempPath );
	size_t pathLength = strlen( tempPath );

	bool bFoundUniqueDir = false;
	while( !bFoundUniqueDir )
	{
		sprintf_s( tempPath + pathLength, tempPathLength - pathLength, "%i", rand() );
		bFoundUniqueDir = ( GetFileAttributes(tempPath) == INVALID_FILE_ATTRIBUTES );   
	}

	CreateDirectory( tempPath, NULL );
}

