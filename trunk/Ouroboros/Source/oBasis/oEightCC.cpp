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
#include <oBasis/oByte.h>
#include <oBasis/oByteSwizzle.h>
#include <oBasis/oError.h>
#include <oBasis/oMacros.h>

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oEightCC& _Value)
{
	if (_SizeofStrDestination < 9)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER, "String buffer not large enough");
		return nullptr;
	}

	#ifdef oLITTLEENDIAN
		oByteSwizzle64 sw; sw.AsUnsignedLongLong = (unsigned long long)_Value;
		unsigned long long fcc = ((unsigned long long)oByteSwap(sw.AsUnsignedInt[0]) << 32) | oByteSwap(sw.AsUnsignedInt[1]);
	#else
		unsigned long long fcc = _Value;
	#endif

	memcpy(_StrDestination, &fcc, sizeof(unsigned long long));
	_StrDestination[8] = 0;
	return _StrDestination;
}

bool oFromString(oEightCC* _pValue, const char* _StrSource)
{
	if (!oSTRVALID(_StrSource))
		return oErrorSetLast(oERROR_INVALID_PARAMETER);
	#ifdef oLITTLEENDIAN
		*_pValue = oByteSwap(*(unsigned long long*)_StrSource);
	#else
		*_pValue = *(unsigned long long *)_StrSource;
	#endif
	return true;
}
