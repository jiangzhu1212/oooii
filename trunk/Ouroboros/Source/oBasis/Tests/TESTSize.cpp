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
#include "oBasisTestCommon.h"
#include <oBasis/oInt.h>

template<typename T, typename SIZE_T>
bool TestIntBase()
{
	T Value = 5;
	SIZE_T SZValue = 7;

	oTESTB(Value < SZValue, "Less than failed");
	oTESTB(Value <= SZValue, "Less than or equal failed");
	oTESTB(!(Value >= SZValue), "Greater than or equal failed");
	oTESTB(!(Value > SZValue), "Greater than failed");

	return true;
};

template<typename T, typename SIZE_T>
bool TestIntIntegral()
{
	if (!TestIntBase<SIZE_T, SIZE_T>())
		return false;

	if (!TestIntBase<T, SIZE_T>())
		return false;

	if (!TestIntBase<SIZE_T, T>())
		return false;

	return true;
}

template<typename SIZE_T>
bool TestInt()
{
	if (!TestIntIntegral<unsigned char, SIZE_T>())
		return false;

	if (!TestIntIntegral<unsigned short, SIZE_T>())
		return false;

	if (!TestIntIntegral<unsigned int, SIZE_T>())
		return false;

	if (!TestIntIntegral<unsigned long long, SIZE_T>())
		return false;

	return true;
}

bool oBasisTest_oInt()
{
	if (!TestInt<oULLong>())
		return false;

	if (!TestInt<oUInt>())
		return false;

	if (!TestInt<oUShort>())
		return false;

	if (!TestInt<oUChar>())
		return false;

	oErrorSetLast(oERROR_NONE);
	return true;
}