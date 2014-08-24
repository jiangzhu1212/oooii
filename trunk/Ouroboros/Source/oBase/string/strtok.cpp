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

char* strtok(const char* _strToken, const char* _strDelim, char** _Context)
{
	struct context { char* Context; char strToken[1]; };
	context* ctx = (context*)*_Context;
	char* s = nullptr;
	if (_strToken)
	{
		size_t size = strlen(_strToken) + 1;
		*_Context = new char[size + sizeof(context)];
		ctx = (context*)*_Context;
		strlcpy(ctx->strToken, _strToken, size);
		s = ctx->strToken;
	}
	char* tok = strtok_r(s, _strDelim, &ctx->Context);
	if (!tok) delete [] *_Context, *_Context = nullptr;
	return tok;
}

void end_strtok(char** _Context)
{
	if (_Context && *_Context) delete [] *_Context;
}

}
