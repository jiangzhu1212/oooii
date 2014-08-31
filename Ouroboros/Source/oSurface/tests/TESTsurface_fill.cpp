// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oSurface/fill.h>
#include <oSurface/codec.h>
#include <oBase/color.h>
#include <oBase/colors.h>
#include <oBase/throw.h>
#include <oBase/timer.h>
#include <vector>

#include "../../test_services.h"

namespace ouro { 

void core_fill_grid_numbers(surface::image* _pBuffer, const int2& _GridDimensions, color _NumberColor);

	namespace tests {

#define SETUP_AND_MAKE() \
	surface::info si; \
	si.format = surface::format::b8g8r8a8_unorm; \
	si.mip_layout = surface::mip_layout::none; \
	si.dimensions = uint3(_Dimensions, 1); \
	surface::image s(si);

surface::image make_numbered_grid(
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
	
	core_fill_grid_numbers(&s, _GridDimensions, _NumberColor);
	return s;
}

surface::image make_checkerboard(
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

surface::image make_solid(const int2& _Dimensions, color _Color)
{
	SETUP_AND_MAKE();
	surface::lock_guard lock(s);
	surface::fill_solid((color*)lock.mapped.data, lock.mapped.row_pitch, si.dimensions.xy(), _Color);
	return s;
}

void TESTsurface_fill(test_services& _Services)
{
	static const color gradiantColors0[4] = { blue, purple, lime, orange };
	static const color gradiantColors1[4] = { midnight_blue, dark_slate_blue, green, chocolate };
	surface::image s;
	s = make_numbered_grid(int2(256,256), int2(64,64), black, black, gradiantColors0);
	_Services.check(s, 0);
	s = make_numbered_grid(int2(512,512), int2(32,32), gray, white, gradiantColors1);
	_Services.check(s, 1);
	s = make_checkerboard(int2(256,256), int2(32,32), cyan, pink);
	_Services.check(s, 2);
	s = make_solid(int2(256,256), tangent_space_normal_blue);
	_Services.check(s, 3);
}

	} // namespace tests
} // namespace ouro
