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
#include <oBase/compiler_config.h>
#include <oBase/string.h>

namespace ouro {

errno_t replace(char* oRESTRICT _StrResult, size_t _SizeofStrResult, const char* oRESTRICT _StrSource, char _ChrFind, char _ChrReplace)
{
	const char* oRESTRICT r = _StrSource;
	char* oRESTRICT w = _StrResult;
	while (1)
	{
		*w++ = *r == _ChrFind ? _ChrReplace : *r;
		if (!*r)
			break;
		r++;
	}
	return 0;
}

errno_t replace(char* oRESTRICT _StrResult, size_t _SizeofStrResult, const char* oRESTRICT _StrSource, const char* _StrFind, const char* _StrReplace)
{
	if (!_StrResult || !_StrSource) return EINVAL;
	if (_StrResult == _StrSource) return EINVAL;
	if (!_StrFind)
		return strlcpy(_StrResult, _StrSource, _SizeofStrResult) < _SizeofStrResult ? 0 : ENOBUFS;
	if (!_StrReplace)
		_StrReplace = "";

	size_t findLen = strlen(_StrFind);
	size_t replaceLen = strlen(_StrReplace);
	const char* s = strstr(_StrSource, _StrFind);

	while (s)
	{
		size_t len = s - _StrSource;
		#ifdef _MSC_VER
			errno_t e = strncpy_s(_StrResult, _SizeofStrResult, _StrSource, len);
			if (e)
				return e;
		#else
			strncpy(_StrResult, _StrSource, len);
		#endif
		_StrResult += len;
		_SizeofStrResult -= len;
		if (strlcpy(_StrResult, _StrReplace, _SizeofStrResult) >= _SizeofStrResult)
			return ENOBUFS;
		_StrResult += replaceLen;
		_SizeofStrResult -= replaceLen;
		_StrSource += len + findLen;
		s = strstr(_StrSource, _StrFind);
	}

	// copy the rest
	return strlcpy(_StrResult, _StrSource, _SizeofStrResult) ? 0 : EINVAL;
}

} // namespace ouro
