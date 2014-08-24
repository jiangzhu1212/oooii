/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#include <obase/string.h>
#include <functional>

namespace ouro {

char* search_path(char* _StrDestination
	, size_t _SizeofStrDestination
	, const char* _SearchPaths
	, const char* _RelativePath
	, const char* _DotPath
	, const std::function<bool(const char* _Path)>& _PathExists)
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
		if (*cur == '.' && _DotPath && *_DotPath)
		{
			size_t len = strlcpy(_StrDestination, _DotPath, _SizeofStrDestination);
			if (len >= _SizeofStrDestination)
				return nullptr;
			if (_StrDestination[len-1] != '/' && _StrDestination[len-1] != '\\')
			{
				if ((len+1) >= _SizeofStrDestination)
					return nullptr;
				_StrDestination[len-1] = '/';
				_StrDestination[len++] = '\0';
			}
				
			dst = _StrDestination + len;
		}

		while (dst < end && *cur && *cur != ';')
			*dst++ = *cur++;
		while (isspace(*(--dst))); // empty
		if (*dst == '/' || *dst == '\\')
			*(++dst) = '/';
		assert(dst < end);
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

}
