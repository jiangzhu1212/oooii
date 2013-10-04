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
#ifndef oSurface_fill_h
#define oSurface_fill_h

#include <oBase/color.h>
#include <oHLSL/oHLSLTypes.h>
#include <functional>

namespace ouro {
	namespace surface {

// Fills the specified 32-bit BGRA (color) buffer with the specified solid 
// color.
void fill_solid(color* _pColors, size_t _RowPitch, const int2& _Dimensions, const color& _Color);

// Fills the specified 32-bit BGRA (color) buffer with the specified solid 
// color, but doesn't touch the bits that are masked out.
void fill_solid_masked(color* _pColors, size_t _RowPitch, const int2& _Dimensions, const color& _Color, uint _Mask);

// Fills the specified 32-bit BGRA (color buffer) with a checkerboard pattern
// of the specified dimensions and two colors.
void fill_checkerboard(color* _pColors, size_t _RowPitch, const int2& _Dimensions, const int2& _GridDimensions, const color& _Color0, const color& _Color1);

// Fills the specified 32-bit BGRA buffer with a gradient that goes between the
// 4 specified colors at the corners of the image.
void fill_gradient(color* _pColors, size_t _RowPitch, const int2& _Dimensions, const color _CornerColors[4]);

// This draws a rectangle at [(0,0),_GridDim-int2(1,1)] in the specified color,
// thus producing a grid pattern.
void fill_grid_lines(color* _pColors, size_t _RowPitch, const int2& _Dimensions, const int2& _GridDimensions, const color& _GridColor);

// This iterates through each grid box and calls the _DrawText function
bool fill_grid_numbers(const int2& _Dimensions, const int2& _GridDimensions, std::function<bool(const int2& _DrawBoxPosition, const int2& _DrawBoxSize, const char* _Text)> _DrawText);

	} // namespace surface
} // namespace ouro

#endif
