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
#include <oBase/quat.h>
#include <oString/stringize.h>

namespace ouro {

bool from_string(quatf* _pValue, const char* _StrSource)
{
	return from_string_float_array((float*)_pValue, 4, _StrSource);
}

bool from_string(quatd* _pValue, const char* _StrSource)
{
	return from_string_double_array((double*)_pValue, 4, _StrSource);
}

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const quatf& value) { return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%f %f %f %f", value.x, value.y, value.z, value.w) ? _StrDestination : nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const quatd& value) { return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%f %f %f %f", value.x, value.y, value.z, value.w) ? _StrDestination : nullptr; }

}
