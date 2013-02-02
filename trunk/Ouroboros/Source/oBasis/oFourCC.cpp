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
#include <oBasis/oFourCC.h>
#include <oBasis/oByte.h>
#include <oBasis/oError.h>
#include <oBasis/oMacros.h>

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oFourCC& _Value)
{
	if (_SizeofStrDestination < 5)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER, "String buffer not large enough");
		return nullptr;
	}

	#ifdef oLITTLEENDIAN
		unsigned int fcc = oByteSwap((unsigned int)_Value);
	#else
		unsigned int fcc = _Value;
	#endif

	memcpy(_StrDestination, &fcc, sizeof(unsigned int));
	_StrDestination[4] = 0;
	return _StrDestination;
}

bool oFromString(oFourCC* _pValue, const char* _StrSource)
{
	if (!oSTRVALID(_StrSource))
		return oErrorSetLast(oERROR_INVALID_PARAMETER);
	#ifdef oLITTLEENDIAN
		*_pValue = oByteSwap(*(unsigned int*)_StrSource);
	#else
		*_pValue = *(unsigned int *)_StrSource;
	#endif
	return true;
}

oRTTI_ATOM_DEFAULT_DESCRIPTION(oFourCC,oFourCC)
