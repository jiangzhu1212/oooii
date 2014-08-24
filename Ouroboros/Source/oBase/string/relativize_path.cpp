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

#include <oBase/string.h>

namespace ouro {

	char* relativize_path(char* _StrDestination, size_t _SizeofStrDestination, const char* _BasePath, const char* _FullPath)
	{
		// note: might needs traits for sep, .. or ../

		// advance till we find a segment that doesn't match
		const char* fullpath = _FullPath;
		const char* basepath = _BasePath;
		const char* fullslash = fullpath;
		const char* baseslash = basepath;

		while ((*fullpath == *basepath) && *fullpath)
		{
			if (*fullpath == '/')
			{
				fullslash = fullpath;
				baseslash = basepath;
			}
			fullpath++;
			basepath++;
		}

		if (fullslash == fullpath) // nothing is relative
			return nullptr;

		// Decide how many ../ segments are needed
		size_t segments = 0;
		baseslash++;
		while (*baseslash != 0)
		{
			if (*baseslash == '/')
				segments++;
			baseslash++;
		}
		fullslash++;

		if (_SizeofStrDestination < (segments * 3 + 1)) // strlen("../") + nul
			return nullptr;

		char* dst = _StrDestination;
		size_t size = _SizeofStrDestination;
		for (size_t i = 0; i < segments; i++)
		{
			dst += strlcpy(dst, "../", size);
			size -= 3;
		}

		return strlcpy(dst, fullslash, size) < _SizeofStrDestination ? _StrDestination : nullptr;
 	}

}
