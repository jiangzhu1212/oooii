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
#pragma once
#ifndef oSwizzle_h
#define oSwizzle_h

struct oByteSwizzle16
{
	union
	{
		short AsShort;
		unsigned short AsUnsignedShort;
		char AsChar[2];
		unsigned char AsUnsignedChar[2];
	};
};

struct oByteSwizzle32
{
	union
	{
		float AsFloat;
		int AsInt;
		unsigned int AsUnsignedInt;
		long AsLong;
		unsigned long AsUnsignedLong;
		short AsShort[2];
		unsigned short AsUnsignedShort[2];
		char AsChar[4];
		unsigned char AsUnsignedChar[4];
	};
};

struct oByteSwizzle64
{
	union
	{
		double AsDouble;
		long long AsLongLong;
		unsigned long long AsUnsignedLongLong;
		float AsFloat[2];
		int AsInt[2];
		unsigned int AsUnsignedInt[2];
		long AsLong[2];
		unsigned long AsUnsignedLong[2];
		short AsShort[4];
		unsigned short AsUnsignedShort[4];
		char AsChar[8];
		unsigned char AsUnsignedChar[8];
	};
};

#endif
