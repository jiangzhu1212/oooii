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
#include <oBasis/oPath.h>
#include <oBase/assert.h>
#include <oBase/macros.h>
#include <oBasis/oStrTok.h>
#include <cstring>
#include <cctype>
#include <algorithm>

using namespace ouro;

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

	return _Path;
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
			size_t sz = _SizeofStrDestination - 3*i;
			if (strlcat(_StrDestination + 3*i, _FileSeparator == '/' ? "../" : "..\\", sz) >= sz)
				return nullptr;
		}

		size_t sz = _SizeofStrDestination-3*nSeperators;
		if (strlcat(_StrDestination+3*nSeperators, _FullPath + rootLength, sz) >= sz)
			return nullptr;
	}

	return _StrDestination;
}

char* oFindInPath(char* _StrDestination, size_t _SizeofStrDestination, const char* _SearchPaths, const char* _RelativePath, const char* _DotPath, const oFUNCTION<bool(const char* _Path)>& _PathExists)
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
			if (!strlcpy(_StrDestination, _DotPath, _SizeofStrDestination))
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
		if (strlcat(_StrDestination, _RelativePath, _SizeofStrDestination) >= _SizeofStrDestination)
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

char* oStrTokToSwitches(char* _StrDestination, size_t _SizeofStrDestination, const char* _Switch, const char* _Tokens, const char* _Separator)
{
	size_t len = strlen(_StrDestination);
	_StrDestination += len;
	_SizeofStrDestination -= len;

	char* ctx = nullptr;
	const char* tok = oStrTok(_Tokens, _Separator, &ctx);
	while (tok)
	{
		strlcpy(_StrDestination, _Switch, _SizeofStrDestination);
		size_t len = strlen(_StrDestination);
		_StrDestination += len;
		_SizeofStrDestination -= len;

		clean_path(_StrDestination, _SizeofStrDestination, tok);

		len = strlen(_StrDestination);
		_StrDestination += len;
		_SizeofStrDestination -= len;

		tok = oStrTok(nullptr, ";", &ctx);
	}

	return _StrDestination;
}

