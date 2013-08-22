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
#pragma once
#ifndef oColor_h
#define oColor_h

#include <oStd/byte.h>
#include <oStd/color.h>
#include <oStd/operators.h>

void oColorRGBToYUV(int _R, int _G, int _B, int* _pY, int* _pU, int* _pV);
void oColorYUVToRGB(int _Y, int _U, int _V, int* _pR, int* _pG, int* _pB);
// see http://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=1&ved=0CE8QFjAA&url=http%3A%2F%2Fwftp3.itu.int%2Fav-arch%2Fjvt-site%2F2003_09_SanDiego%2FJVT-I014r1.doc&ei=gegRUJCwB-eSiQLdjYHIBA&usg=AFQjCNG6399B3vKVP_cohT2Av7vmMcj6aQ
//	for the paper on yCoCg color space. its better than yuv. going to be used in H.265. there are 2 variants. YCoCg and YCoCg-R. H.265 draft is using the YCoCg variant, so only including that one.
void oColorRGBToYCoCg(int _R, int _G, int _B, int* _pY, int* _pCo, int* _pCg);
void oColorYCoCgToRGB(int _Y, int _Co, int _Cg, int* _pR, int* _pG, int* _pB);

inline void oColorDecomposeToYUV(oStd::color _Color, int* _pY, int* _pU, int* _pV, int* _pA = nullptr) { int r,g,b,a; _Color.decompose(&r, &g, &b, &a); oColorRGBToYUV(r, g, b, _pY, _pU, _pV); if(_pA) *_pA = a;}
inline oStd::color oColorComposeFromYUV(int _Y, int _U, int _V, int _A = 255) { int r,g,b; oColorYUVToRGB(_Y, _U, _V, &r, &g, &b); return oStd::color(r, g, b, _A); }
inline void oColorDecomposeToYCoCg(oStd::color _Color, int* _pY, int* _pCo, int* _pCg, int* _pA = nullptr) { int r,g,b,a; _Color.decompose(&r, &g, &b, &a); oColorRGBToYCoCg(r, g, b, _pY, _pCo, _pCg); if(_pA) *_pA = a; }
inline oStd::color oColorComposeFromYCoCg(int _Y, int _Co, int _Cg, int _A = 255) { int r,g,b; oColorYCoCgToRGB(_Y, _Co, _Cg, &r, &g, &b); return oStd::color(r, g, b, _A); }

// Hue is in degrees, saturation and value are [0,1]
void oColorDecomposeToHSV(oStd::color _Color, float* _pH, float* _pS, float* _pV);

#endif
