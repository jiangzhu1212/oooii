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
#ifndef oEightCC_h
#define oEightCC_h

// An EightCC is much like a FourCC ( http://en.wikipedia.org/wiki/FourCC ) but twice as long to avoid collisions
struct oEightCC
{
	oEightCC()
	{}
	oEightCC(long _FourCCA, long _FourCCB)
	{
		FourCC[0] = _FourCCA;
		FourCC[1] = _FourCCB;
	}

	operator long long() const
	{
		return EightCC;
	}

	union
	{
		long FourCC[2];
		long long EightCC;
	};

	bool operator!=(const oEightCC& _other) const { return _other.EightCC != EightCC; }
	bool operator==(const oEightCC& _other) const { return _other.EightCC == EightCC; }
};

#endif // oEightCC_h