// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#ifndef oSurface_fill_h
#define oSurface_fill_h

// Simple utilities for filling a 2D surface with color in various patterns. 
// This is useful in generating debug images/textures for infrastructure 
// bringup.

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

// Fills a 3D texture with colors that go from black to white, red along U, green along V, blue along W
void fill_color_cube(color* _pColors, size_t _RowPitch, size_t _SlicePitch, const int3& _Dimensions);

	} // namespace surface
} // namespace ouro

#endif
