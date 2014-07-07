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
#include <oGPU/oGPU.h>

#define STR_SUPPORT(_T, _NumTs) \
	oDEFINE_TO_STRING(_T) \
	oDEFINE_FROM_STRING(_T, _NumTs)

namespace ouro {

static_assert(sizeof(surface::format) == 1, "unexpected size");

const char* as_string(const gpu::debug_level::value& _Value)
{
	switch (_Value)
	{
		case gpu::debug_level::none: return "none";
		case gpu::debug_level::normal: return "normal";
		case gpu::debug_level::unfiltered: return "unfiltered";

		default: break;
	}
	return "?";
}

STR_SUPPORT(gpu::debug_level::value, gpu::debug_level::count);

const char* as_string(const gpu::cube_face::value& _Value)
{
	switch (_Value)
	{
		case gpu::cube_face::posx: return "posx";
		case gpu::cube_face::negx: return "negx";
		case gpu::cube_face::posy: return "posy";
		case gpu::cube_face::negy: return "negy";
		case gpu::cube_face::posz: return "posz";
		case gpu::cube_face::negz: return "negz";

		default: break;
	}
	return "?";
}

STR_SUPPORT(gpu::cube_face::value, gpu::cube_face::count);

const char* as_string(const gpu::resource_type::value& _Value)
{
	switch (_Value)
	{
		case gpu::resource_type::buffer: return "buffer";
		case gpu::resource_type::texture1: return "texture";

		default: break;
	}
	return "?";
}

STR_SUPPORT(gpu::resource_type::value, gpu::resource_type::count);

const char* as_string(const gpu::buffer_type::value& _Value)
{
	switch (_Value)
	{
		case gpu::buffer_type::constant: return "constant";
		case gpu::buffer_type::readback: return "readback";
		case gpu::buffer_type::unordered_raw: return "unordered_raw";
		case gpu::buffer_type::unordered_unstructured: return "unordered_unstructured";
		case gpu::buffer_type::unordered_structured: return "unordered_structured";
		case gpu::buffer_type::unordered_structured_append: return "unordered_structured_append"; 
		case gpu::buffer_type::unordered_structured_counter: return "unordered_structured_counter";

		default: break;
	}
	return "?";
}

STR_SUPPORT(gpu::buffer_type::value, gpu::buffer_type::count);

const char* as_string(const gpu::texture_type::value& _Value)
{
	switch (_Value)
	{
		case gpu::texture_type::default_1d: return "default_1d";
		case gpu::texture_type::mipped_1d: return "mipped_1d";
		case gpu::texture_type::array_1d: return "array_1d";
		case gpu::texture_type::mipped_array_1d: return "mipped_array_1d";
		case gpu::texture_type::render_target_1d: return "render_target_1d";
		case gpu::texture_type::mipped_render_target_1d: return "mipped_render_target_1d";
		case gpu::texture_type::readback_1d: return "readback_1d";
		case gpu::texture_type::mipped_readback_1d: return "mipped_readback_1d";
		case gpu::texture_type::readback_array_1d: return "readback_array_1d";
		case gpu::texture_type::mipped_readback_array_1d: return "mipped_readback_array_1d";
		case gpu::texture_type::default_2d: return "default_2d";
		case gpu::texture_type::mipped_2d: return "mipped_2d";
		case gpu::texture_type::array_2d: return "array_2d";
		case gpu::texture_type::mipped_array_2d: return "mipped_array_2d";
		case gpu::texture_type::render_target_2d: return "render_target_2d";
		case gpu::texture_type::mipped_render_target_2d: return "mipped_render_target_2d";
		case gpu::texture_type::readback_2d: return "readback_2d";
		case gpu::texture_type::mipped_readback_2d: return "mipped_readback_2d";
		case gpu::texture_type::readback_array_2d: return "readback_array_2d";
		case gpu::texture_type::mipped_readback_array_2d: return "mipped_readback_array_2d";
		case gpu::texture_type::unordered_2d: return "unordered_2d";
		case gpu::texture_type::default_cube: return "default_cube";
		case gpu::texture_type::mipped_cube: return "mipped_cube";
		case gpu::texture_type::array_cube: return "array_cube";
		case gpu::texture_type::mipped_array_cube: return "mipped_array_cube";
		case gpu::texture_type::render_target_cube: return "render_target_cube";
		case gpu::texture_type::mipped_render_target_cube: return "mipped_render_target_cube";
		case gpu::texture_type::readback_cube: return "readback_cube";
		case gpu::texture_type::mipped_readback_cube: return "mipped_readback_cube";
		case gpu::texture_type::default_3d: return "default_3d";
		case gpu::texture_type::mipped_3d: return "mipped_3d";
		case gpu::texture_type::array_3d: return "array_3d";
		case gpu::texture_type::mipped_array_3d: return "mipped_array_3d";
		case gpu::texture_type::render_target_3d: return "render_target_3d";
		case gpu::texture_type::mipped_render_target_3d: return "mipped_render_target_3d";
		case gpu::texture_type::readback_3d: return "readback_3d";
		case gpu::texture_type::mipped_readback_3d: return "mipped_readback_3d";

		default: break;
	}
	return "?";
}

oDEFINE_TO_STRING(gpu::texture_type::value);

bool from_string(gpu::texture_type::value* _pValue, const char* _StrSource)
{
	sstring copy(_StrSource);
	char* ctx = nullptr;
	char* tok = strtok_r(copy, "_", &ctx);
	int traits = 0;
	while (tok)
	{
		int traits = 0;
		if (!_stricmp("1d", tok)) traits |= gpu::texture_type::type_1d;
		else if (!_stricmp("2d", tok)) traits |= gpu::texture_type::type_2d;
		else if (!_stricmp("3d", tok)) traits |= gpu::texture_type::type_3d;
		else if (!_stricmp("cube", tok)) traits |= gpu::texture_type::type_cube;
		else if (!_stricmp("default", tok)) traits |= gpu::texture_type::usage_default;
		else if (!_stricmp("readback", tok)) traits |= gpu::texture_type::usage_readback;
		else if (!_stricmp("render_target", tok)) traits |= gpu::texture_type::usage_render_target;
		else if (!_stricmp("unordered", tok)) traits |= gpu::texture_type::usage_unordered;
		else if (!_stricmp("mipped", tok)) traits |= gpu::texture_type::flag_mipped;
		else if (!_stricmp("array", tok)) traits |= gpu::texture_type::flag_array;

		tok = strtok_r(nullptr, "_", &ctx);
	}

	*_pValue = (gpu::texture_type::value)traits;
	return !!traits;
}

const char* as_string(const gpu::query_type::value& _Value)
{
	switch (_Value)
	{
		case gpu::query_type::timer: return "timer";

		default: break;
	}
	return "?";
}

STR_SUPPORT(gpu::query_type::value, gpu::query_type::count);

const char* as_string(const gpu::clear_type::value& _Value)
{
	switch (_Value)
	{
		case gpu::clear_type::depth: return "depth";
		case gpu::clear_type::stencil: return "stencil";
		case gpu::clear_type::depth_stencil: return "depth_stencil";
		case gpu::clear_type::color: return "color";
		case gpu::clear_type::color_depth: return "color_depth";
		case gpu::clear_type::color_stencil: return "color_stencil";
		case gpu::clear_type::color_depth_stencil: return "color_depth_stencil";

		default: break;
	}
	return "?";
}

STR_SUPPORT(gpu::clear_type::value, gpu::clear_type::count);

} // namespace ouro
