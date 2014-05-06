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
#include <oGPU/state.h>

#define STR_SUPPORT(_T, _NumTs) \
	oDEFINE_TO_STRING(_T) \
	oDEFINE_FROM_STRING(_T, _NumTs)

namespace ouro {

const char* as_string(const gpu::sampler_state::value& _Value)
{
	switch (_Value)
	{
		case gpu::sampler_state::point_clamp: return "point_clamp"; 
		case gpu::sampler_state::point_wrap: return "point_wrap";
		case gpu::sampler_state::linear_clamp: return "linear_clamp";
		case gpu::sampler_state::linear_wrap: return "linear_wrap";
		case gpu::sampler_state::aniso_clamp: return "aniso_clamp";
		case gpu::sampler_state::aniso_wrap: return "aniso_wrap";
		default: break;
	}
	return "?";
}
STR_SUPPORT(gpu::sampler_state::value, gpu::sampler_state::count);

const char* as_string(const gpu::rasterizer_state::value& _Value)
{
	switch (_Value)
	{
		case gpu::rasterizer_state::front_face: return "front_face";
		case gpu::rasterizer_state::back_face: return "back_face"; 
		case gpu::rasterizer_state::two_sided: return "two_sided";
		case gpu::rasterizer_state::front_wireframe: return "front_wireframe";
		case gpu::rasterizer_state::back_wireframe: return "back_wireframe"; 
		case gpu::rasterizer_state::two_sided_wireframe: return "two_sided_wireframe";
		default: break;
	}
	return "?";
}
STR_SUPPORT(gpu::rasterizer_state::value, gpu::rasterizer_state::count);

const char* as_string(const gpu::depth_stencil_state::value& _Value)
{
	switch (_Value)
	{
		case gpu::depth_stencil_state::none: return "none";
		case gpu::depth_stencil_state::test_and_write: return "test_and_write";
		case gpu::depth_stencil_state::test: return "test";
		default: break;
	}
	return "?";
}
STR_SUPPORT(gpu::depth_stencil_state::value, gpu::depth_stencil_state::count);

const char* as_string(const gpu::blend_state::value& _Value)
{
	switch (_Value)
	{
		case gpu::blend_state::opaque: return "opaque";
		case gpu::blend_state::alpha_test: return "alpha_test";
		case gpu::blend_state::accumulate: return "accumulate";
		case gpu::blend_state::additive: return "additive";
		case gpu::blend_state::multiply: return "multiply";
		case gpu::blend_state::screen: return "screen";
		case gpu::blend_state::translucent: return "translucent";
		case gpu::blend_state::min_: return "min";
		case gpu::blend_state::max_: return "max";
		default: break;
	}
	return "?";
}
STR_SUPPORT(gpu::blend_state::value, gpu::blend_state::count);

} // namespace ouro
