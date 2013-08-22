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
// Simple utilities for filling a 2D surface with color in various patterns. 
// This is useful in generating debug images/textures for infrastructure 
// bringup.
#ifndef oSurfaceFill_h
#define oSurfaceFill_h

#include <oStd/color.h>
#include <oStd/function.h>
#include <oBasis/oMathTypes.h>

// Fills the specified 32-bit BGRA (oStd::color) buffer with the specified solid 
// color.
void oSurfaceFillSolid(oStd::color* _pColors, size_t _RowPitch, const int2& _Dimensions, const oStd::color& _Color);

// Fills the specified 32-bit BGRA (oStd::color) buffer with the specified solid 
// color, but doesn't touch the bits that are masked out.
void oSurfaceFillSolidMasked(oStd::color* _pColors, size_t _RowPitch, const int2& _Dimensions, const oStd::color& _Color, uint _Mask);

// Fills the specified 32-bit BGRA (oStd::color buffer) with a checkerboard pattern
// of the specified dimensions and two colors.
void oSurfaceFillCheckerboard(oStd::color* _pColors, size_t _RowPitch, const int2& _Dimensions, const int2& _GridDimensions, const oStd::color& _Color0, const oStd::color& _Color1);

// Fills the specified 32-bit BGRA buffer with a gradient that goes between the
// 4 specified colors at the corners of the image.
void oSurfaceFillGradient(oStd::color* _pColors, size_t _RowPitch, const int2& _Dimensions, oStd::color _CornerColors[4]);

// This draws a rectangle at [(0,0),_GridDim-int2(1,1)] in the specified color,
// thus producing a grid pattern.
void oSurfaceFillGridLines(oStd::color* _pColors, size_t _RowPitch, const int2& _Dimensions, const int2& _GridDimensions, const oStd::color& _GridColor);

// This iterates through each grid box and calls the _DrawText function
bool oSurfaceFillGridNumbers(const int2& _Dimensions, const int2& _GridDimensions, oFUNCTION<bool(const int2& _DrawBoxPosition, const int2& _DrawBoxSize, const char* _Text)> _DrawText);

#endif
