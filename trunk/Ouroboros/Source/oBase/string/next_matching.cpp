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

#include <oBase/string.h>
#include <memory.h>

namespace ouro {

const char* next_matching(const char* _pPointingAtOpenBrace, char _CloseBrace)
{
	int open = 1;
	char open_brace = *_pPointingAtOpenBrace;
	const char* cur = _pPointingAtOpenBrace + 1;
	while (*cur && open > 0)
	{
		if (*cur == _CloseBrace)
			open--;
		else if (*cur == open_brace)
			open++;
		cur++;
	}

	if (open > 0)
		return nullptr;
	return cur - 1;
}

char* next_matching(char* _pPointingAtOpenBrace, char _CloseBrace)
{
	return const_cast<char*>(next_matching(static_cast<const char*>(_pPointingAtOpenBrace), _CloseBrace));
}

const char* next_matching(const char* _pPointingAtOpenBrace, const char* _OpenBrace, const char* _CloseBrace)
{
	int open = 1;
	size_t lOpen = strlen(_OpenBrace);
	size_t lClose = strlen(_CloseBrace);
	const char* cur = _pPointingAtOpenBrace + lOpen;
	while (*cur && open > 0)
	{
		if (!memcmp(cur, _CloseBrace, lClose))
			open--;
		else if (!memcmp(cur, _OpenBrace, lOpen))
			open++;
		cur++;
	}

	if (open > 0)
		return 0;
	return cur - 1;
}

char* next_matching(char* _pPointingAtOpenBrace, const char* _OpenBrace, const char* _CloseBrace)
{
	return const_cast<char*>(next_matching(static_cast<const char*>(_pPointingAtOpenBrace), _OpenBrace, _CloseBrace));
}

} // namespace ouro
