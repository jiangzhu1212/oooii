/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
// Encapsulation of a FourCC code mostly for better self-documenting code
// http://en.wikipedia.org/wiki/FourCC
#ifndef oFourCC_h
#define oFourCC_h

#include <oBasis/oOperators.h>

struct oFourCC : oCompareable<oFourCC>
{
	oFourCC() {}

	oFourCC(unsigned int _FourCC)
		: FourCC(_FourCC)
	{}

	operator int() const { return *(int*)&FourCC; }
	operator unsigned int() const { return FourCC; }

	bool operator==(const oFourCC& _That) const { return FourCC == _That.FourCC; }
	bool operator<(const oFourCC& _That) const { return FourCC < _That.FourCC; }

protected:
	unsigned int FourCC;
};

#endif
