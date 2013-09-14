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

namespace oStd {

static inline bool issep(int c) { return c == '\\' || c == '/'; }
static inline bool isdotdir(const char* s) { return s[0] == '.' && issep(s[1]); }
static inline bool isdotend(const char* s) { return s[0] == '.' && !s[1]; }
static inline bool isdotdotdir(const char* s) { return s[0] == '.' && s[1] == '.' && issep(s[2]); }
static inline bool isdotdotend(const char* s) { return s[0] == '.' && s[1] == '.' && !s[2]; }
static inline bool isunc(const char* s) { return issep(s[0]) && issep(s[1]); }

// back up write head to last dir
#define BACK_UP() do \
{	while (w != _StrDestination && issep(*(w-1))) w--; \
	while (w != _StrDestination && !issep(*(w-1))) w--; \
	if (w == _StrDestination && issep(*w)) w++; \
} while(false)

char* clean_path(char* _StrDestination, size_t _SizeofStrDestination, const char* _SourcePath, char _FileSeparator, bool _ZeroBuffer)
{
	if (!_StrDestination || !_SourcePath)
		return nullptr;

	char* w = _StrDestination;
	char* wend = w + _SizeofStrDestination;
	char* wendm1 = wend - 1; // save room for nul
	*wendm1 = 0; // be safe
	const char* r = _SourcePath;

	// preserve UNC
	if (isunc(r))
	{
		if (_SizeofStrDestination <= 2)
			return nullptr;
		*w++ = *r++;
		*w++ = *r++;
	}

	// preserve leading dots
	while (*r)
	{
		if (isdotdir(r))
		{
			if (_SizeofStrDestination <= 2)
				return nullptr;
			*w++ = *r++;
			*w++ = *r++;
			_SizeofStrDestination -= 2;
		}
		else if (isdotdotdir(r))
		{
			if (_SizeofStrDestination <= 3)
				return nullptr;
			*w++ = *r++;
			*w++ = *r++;
			*w++ = *r++;
			_SizeofStrDestination -= 3;
		}
		else
			break;
	}

	while (*r && w < wendm1)
	{
		if (isdotdir(r))
			r += 2;

		else if (isdotend(r))
			break;

		else if (isdotdotdir(r))
		{
			BACK_UP();
			// skip the read head
			r += 3;
		}

		else if (isdotdotend(r))
		{
			BACK_UP();
			break;
		}

		else if (issep(*r))
		{
			*w++ = '/';
			do { r++; } while (*r && issep(*r));
		}

		else
			*w++ = *r++;
	}

	*w = 0;
	if (_ZeroBuffer)
		while (w < wend)
			*w++ = 0;
	return _StrDestination;
}

} // namespace oStd
