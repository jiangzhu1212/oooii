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
#include <string.h>
#include <malloc.h>

namespace oStd {

char* insert(char* _StrSource, size_t _SizeofStrSource, char* _InsertionPoint, size_t _ReplacementLength, const char* _Insertion)
{
	size_t insertionLength = strlen(_Insertion);
	size_t afterInsertionLength = strlen(_InsertionPoint) - _ReplacementLength;
	size_t newLen = static_cast<size_t>(_InsertionPoint - _StrSource) + afterInsertionLength;
	if (newLen >= _SizeofStrSource)
		return nullptr;
	// to avoid the overwrite of a direct memcpy, copy the remainder
	// of the string out of the way and then copy it back in.
	char* tmp = (char*)alloca(afterInsertionLength);
	memcpy(tmp, _InsertionPoint + _ReplacementLength, afterInsertionLength);
	memcpy(_InsertionPoint, _Insertion, insertionLength);
	char* p = _InsertionPoint + insertionLength;
	memcpy(p, tmp, afterInsertionLength);
	p[afterInsertionLength] = '\0';
	return p;
}

} // namespace oStd
