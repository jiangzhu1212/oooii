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
#include "pch.h"
#include <oooii/oColor.h>
#include <oooii/oMath.h>

void oDecomposeToHSV(oColor _Color, float* _pH, float* _pS, float* _pV)
{
	float r,g,b,a;
	oDecomposeColor(_Color, &r, &g, &b, &a);
	float Max = __max(r, __max(g,b));
	float Min = __min(r, __min(g,b));
	float diff = Max - Min;
	*_pV = Max;

	if (oEqual(diff, 0.0f))
	{
		*_pH = 0.0f;
		*_pS = 0.0f;
	}

	else
	{
		*_pS = diff / Max;
		float R = 60.0f * (Max - r) / diff + 180.0f;
		float G = 60.0f * (Max - g) / diff + 180.0f;
		float B = 60.0f * (Max - b) / diff + 180.0f;
		if (r == Max)
			*_pH = B - G;
		else if (oEqual(g, Max))
			*_pH = 120.0f + R - B;
		else
			*_pH = 240.0f + G - R;
	}

	if (*_pH < 0.0f) *_pH += 360.0f;
	if (*_pH >= 360.0f) *_pH -= 360.0f;
}
