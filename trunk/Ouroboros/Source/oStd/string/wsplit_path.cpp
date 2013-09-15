/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
 *                                                                        *
 * Permission is hereby granted, free of wchar_tge, to any person obtaining  *
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
#include <string.h>

namespace oStd {

static bool is_sep(wchar_t c, bool _Posix) { return c == L'/' || (!_Posix && c == L'\\'); }
static bool is_unc(const wchar_t* _Path, bool _Posix) { return _Path && is_sep(_Path[0], _Posix) && is_sep(_Path[1], _Posix) && !is_sep(_Path[2], _Posix) && _Path[0] == _Path[1]; }
static bool has_vol(const wchar_t* _Path, bool _Posix) { return !_Posix && _Path && _Path[0] && _Path[1] == L':';  }

static const wchar_t* find_parent_end(const wchar_t* _Path, bool _Posix, const wchar_t* _Basename, bool _UNC)
{
	const wchar_t* end = _Basename;
	while (end >= _Path && is_sep(*end, _Posix)) end--;
	if (!_UNC && end > (_Path+1)) end--;
	while (end > (_Path+1) && is_sep(*end, _Posix) && is_sep(*(end-1), _Posix)) end--;
	return end;
}

size_t wsplit_path(const wchar_t* _Path
	, bool _Posix
	, const wchar_t** _ppRoot
	, const wchar_t** _ppPath
	, const wchar_t** _ppParentPathEnd
	, const wchar_t** _ppBasename
	, const wchar_t** _ppExt)
{
	*_ppRoot = *_ppPath = *_ppParentPathEnd = *_ppBasename = *_ppExt = nullptr;
	const size_t len = wcslen(_Path);
	const bool unc = is_unc(_Path, _Posix);
	const wchar_t* NonUNCPath = _Path;
	if (unc)
	{
		*_ppRoot = _Path;
		NonUNCPath = _Path + 2;
	}

	else if (has_vol(_Path, _Posix))
		*_ppRoot = _Path;

	const wchar_t* cur = _Path + len - 1;
	while (cur >= NonUNCPath)
	{
		if (is_sep(*cur, _Posix))
		{
			// *(cur+1) fails with foo/ because it's a valid null basename
			// cur != _Path fails with "/" because there is no parent
			if (!*_ppBasename)
			{
				if (is_sep(NonUNCPath[0], _Posix) && !NonUNCPath[1])
					*_ppBasename = NonUNCPath;
				else
					*_ppBasename = cur + 1;
				if (*_ppExt)
				{
					*_ppParentPathEnd = find_parent_end(_Path, _Posix, *_ppBasename, unc);
					return len;
				}
			}
		}

		else if (*cur == L'.' && !*_ppExt)
			*_ppExt = cur;

		cur--;
	}

	if (*_ppExt && !*_ppBasename && (!(*_ppExt)[1] || (*_ppExt)[1] == L'.')) // "." or ".."
		*_ppExt = nullptr;

	if (*_ppBasename)
	{
		*_ppParentPathEnd = find_parent_end(_Path, _Posix, *_ppBasename, unc);
		if (!**_ppBasename)
			*_ppBasename = nullptr;
	}

	return len;
}

} // namespace oStd
