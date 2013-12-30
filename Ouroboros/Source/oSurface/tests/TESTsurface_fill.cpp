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
#include <oSurface/fill.h>
#include <oSurface/codec.h>
#include <oBase/color.h>
#include <oBase/throw.h>
#include <oBase/timer.h>
#include <vector>

#include "../../test_services.h"

void PlatformFillGridNumbers(ouro::surface::buffer* _pBuffer
	, const int2& _GridDimensions, ouro::color _NumberColor);

namespace ouro { 
	namespace tests {

#define SETUP_AND_MAKE() \
	surface::info si; \
	si.format = surface::b8g8r8a8_unorm; \
	si.layout = surface::image; \
	si.dimensions = int3(_Dimensions, 1); \
	std::shared_ptr<surface::buffer> s = surface::buffer::make(si);

std::shared_ptr<surface::buffer> make_numbered_grid(
	const int2& _Dimensions
	, const int2& _GridDimensions
	, color _GridColor
	, color _NumberColor
	, const color _CornerColors[4])
{
	SETUP_AND_MAKE();

	{
		surface::lock_guard lock(s);
		color* c = (color*)lock.mapped.data;
		surface::fill_gradient(c, lock.mapped.row_pitch, si.dimensions.xy(), _CornerColors);
		surface::fill_grid_lines(c, lock.mapped.row_pitch, si.dimensions.xy(), _GridDimensions, _GridColor);
	}
	
	PlatformFillGridNumbers(s.get(), _GridDimensions, _NumberColor);
	return s;
}

static std::shared_ptr<surface::buffer> make_checkerboard(
	const int2& _Dimensions
	, const int2& _GridDimensions
	, color _Color0
	, color _Color1)
{
	SETUP_AND_MAKE();
	surface::lock_guard lock(s);
	surface::fill_checkerboard((color*)lock.mapped.data, lock.mapped.row_pitch
		, si.dimensions.xy(), int2(64, 64), _Color0, _Color1);
	return s;
}

static std::shared_ptr<surface::buffer> make_solid(const int2& _Dimensions, color _Color)
{
	SETUP_AND_MAKE();
	surface::lock_guard lock(s);
	surface::fill_solid((color*)lock.mapped.data, lock.mapped.row_pitch, si.dimensions.xy(), _Color);
	return s;
}

void TESTsurface_fill(test_services& _Services)
{
	static const color gradiantColors0[4] = { Blue, Purple, Lime, Orange};
	static const color gradiantColors1[4] = { MidnightBlue, DarkSlateBlue, Green, Chocolate };
	std::shared_ptr<surface::buffer> s;
	s = make_numbered_grid(int2(256,256), int2(64,64), Black, Black, gradiantColors0);
	_Services.check(s, 0);
	s = make_numbered_grid(int2(512,512), int2(32,32), Gray, White, gradiantColors1);
	_Services.check(s, 1);
	s = make_checkerboard(int2(256,256), int2(32,32), Cyan, Pink);
	_Services.check(s, 2);
	s = make_solid(int2(256,256), TangentSpaceNormalBlue);
	_Services.check(s, 3);
}

	} // namespace tests
} // namespace ouro