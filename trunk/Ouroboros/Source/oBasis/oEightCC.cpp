/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#include <oBasis/oEightCC.h>
#include <oBasis/oError.h>
#include <oStd/macros.h>

namespace oStd {

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const oEightCC& _Value)
{
	if (_SizeofStrDestination < 9)
	{
		oErrorSetLast(std::errc::invalid_argument, "String buffer not large enough");
		return nullptr;
	}

	#ifdef oLITTLEENDIAN
		oStd::byte_swizzle64 sw; sw.as_unsigned_long_long = (unsigned long long)_Value;
		unsigned long long fcc = ((unsigned long long)oStd::endian_swap(sw.as_unsigned_int[0]) << 32) | oStd::endian_swap(sw.as_unsigned_int[1]);
	#else
		unsigned long long fcc = _Value;
	#endif

	memcpy(_StrDestination, &fcc, sizeof(unsigned long long));
	_StrDestination[8] = 0;
	return _StrDestination;
}

bool from_string(oEightCC* _pValue, const char* _StrSource)
{
	if (!oSTRVALID(_StrSource))
		return oErrorSetLast(std::errc::invalid_argument);
	*_pValue = oStd::to_big_endian(*(unsigned long long*)_StrSource);
	return true;
}

} // namespace oStd