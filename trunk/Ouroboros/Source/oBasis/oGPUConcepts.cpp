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
#include <oBasis/oGPUConcepts.h>

#define STR_SUPPORT(_T, _NumTs) \
	oDEFINE_TO_STRING(_T) \
	oDEFINE_FROM_STRING(_T, _NumTs) \

namespace ouro {

const char* as_string(const gpu::api::value& _Value)
{
	switch (_Value)
	{
		case gpu::api::unknown: return "unknown";
		case gpu::api::d3d11: return "d3d11";
		case gpu::api::ogl: return "ogl";
		case gpu::api::ogles: return "ogles";
		case gpu::api::webgl: return "webgl";
		case gpu::api::custom: return "custom";

		default: break;
	}
	return "?";
}

STR_SUPPORT(gpu::api::value, gpu::api::count);

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

const char* as_string(const gpu::primitive_type::value& _Value)
{
	switch (_Value)
	{
		case gpu::primitive_type::unknown: return "unknown";
		case gpu::primitive_type::points: return "points";
		case gpu::primitive_type::lines: return "lines";
		case gpu::primitive_type::line_strips: return "line_strips";
		case gpu::primitive_type::triangles: return "triangles";
		case gpu::primitive_type::triangle_strips: return "triangle_strips";
		case gpu::primitive_type::lines_adjacency: return "lines_adjacency";
		case gpu::primitive_type::line_strips_adjacency: return "line_strips_adjacency";
		case gpu::primitive_type::triangles_adjacency: return "triangles_adjacency";
		case gpu::primitive_type::triangle_strips_adjacency: return "triangle_strips_adjacency";
		case gpu::primitive_type::patches1: return "patches1";
		case gpu::primitive_type::patches2: return "patches2";
		case gpu::primitive_type::patches3: return "patches3";
		case gpu::primitive_type::patches4: return "patches4";
		case gpu::primitive_type::patches5: return "patches5";
		case gpu::primitive_type::patches6: return "patches6";
		case gpu::primitive_type::patches7: return "patches7";
		case gpu::primitive_type::patches8: return "patches8";
		case gpu::primitive_type::patches9: return "patches9";
		case gpu::primitive_type::patches10: return "patches10";
		case gpu::primitive_type::patches11: return "patches11";
		case gpu::primitive_type::patches12: return "patches12";
		case gpu::primitive_type::patches13: return "patches13";
		case gpu::primitive_type::patches14: return "patches14";
		case gpu::primitive_type::patches15: return "patches15";
		case gpu::primitive_type::patches16: return "patches16";
		case gpu::primitive_type::patches17: return "patches17";
		case gpu::primitive_type::patches18: return "patches18";
		case gpu::primitive_type::patches19: return "patches19";
		case gpu::primitive_type::patches20: return "patches20";
		case gpu::primitive_type::patches21: return "patches21";
		case gpu::primitive_type::patches22: return "patches22";
		case gpu::primitive_type::patches23: return "patches23";
		case gpu::primitive_type::patches24: return "patches24";
		case gpu::primitive_type::patches25: return "patches25";
		case gpu::primitive_type::patches26: return "patches26";
		case gpu::primitive_type::patches27: return "patches27";
		case gpu::primitive_type::patches28: return "patches28";
		case gpu::primitive_type::patches29: return "patches29";
		case gpu::primitive_type::patches30: return "patches30";
		case gpu::primitive_type::patches31: return "patches31";
		case gpu::primitive_type::patches32: return "patches32";

		default: break;
	}
	return "?";
}

STR_SUPPORT(gpu::primitive_type::value, gpu::primitive_type::count);

const char* as_string(const gpu::resource_type::value& _Value)
{
	switch (_Value)
	{
		case gpu::resource_type::buffer: return "buffer";
		case gpu::resource_type::texture: return "texture";

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
		case gpu::buffer_type::index: return "index";
		case gpu::buffer_type::index_readback: return "index_readback";
		case gpu::buffer_type::vertex: return "vertex";
		case gpu::buffer_type::vertex_readback: return "vertex_readback";
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

const char* as_string(const gpu::texture_trait::value& _Value)
{
	switch (_Value)
	{
		case gpu::texture_trait::cube: return "cube";
		case gpu::texture_trait::_1d: return "_1d";
		case gpu::texture_trait::_2d: return "_2d";
		case gpu::texture_trait::_3d: return "_3d";
		case gpu::texture_trait::mipped: return "mipped";
		case gpu::texture_trait::array: return "array";
		case gpu::texture_trait::readback: return "readback";
		case gpu::texture_trait::unordered: return "unordered";
		case gpu::texture_trait::render_target: return "render_target";

		default: break;
	}
	return "?";
}

oDEFINE_TO_STRING(gpu::texture_trait::value);

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
		if (!_stricmp("1d", tok)) traits |= gpu::texture_trait::_1d;
		else if (!_stricmp("2d", tok)) traits |= gpu::texture_trait::_2d;
		else if (!_stricmp("3d", tok)) traits |= gpu::texture_trait::_3d;
		else if (!_stricmp("cube", tok)) traits |= gpu::texture_trait::cube;
		else if (!_stricmp("mipped", tok)) traits |= gpu::texture_trait::mipped;
		else if (!_stricmp("array", tok)) traits |= gpu::texture_trait::array;
		else if (!_stricmp("readback", tok)) traits |= gpu::texture_trait::readback;
		else if (!_stricmp("unordered", tok)) traits |= gpu::texture_trait::unordered;
		else if (!_stricmp("render_target", tok)) traits |= gpu::texture_trait::render_target;

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

const char* as_string(const gpu::surface_state::value& _Value)
{
	switch (_Value)
	{
		case gpu::surface_state::front_face: return "front_face";
		case gpu::surface_state::back_face: return "back_face"; 
		case gpu::surface_state::two_sided: return "two_sided";
		case gpu::surface_state::front_wireframe: return "front_wireframe";
		case gpu::surface_state::back_wireframe: return "back_wireframe"; 
		case gpu::surface_state::two_sided_wireframe: return "two_sided_wireframe";

		default: break;
	}
	return "?";
}

STR_SUPPORT(gpu::surface_state::value, gpu::surface_state::count);

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

const char* as_string(const gpu::sampler_type::value& _Value)
{
	switch (_Value)
	{
		case gpu::sampler_type::point_clamp: return "point_clamp"; 
		case gpu::sampler_type::point_wrap: return "point_wrap";
		case gpu::sampler_type::linear_clamp: return "linear_clamp";
		case gpu::sampler_type::linear_wrap: return "linear_wrap";
		case gpu::sampler_type::aniso_clamp: return "aniso_clamp";
		case gpu::sampler_type::aniso_wrap: return "aniso_wrap";
		case gpu::sampler_type::point_clamp_bias_up1: return "point_clamp_bias_up1";
		case gpu::sampler_type::point_wrap_bias_up1: return "point_wrap_bias_up1";
		case gpu::sampler_type::linear_clamp_bias_up1: return "linear_clamp_bias_up1";
		case gpu::sampler_type::linear_wrap_bias_up1: return "linear_wrap_bias_up1";
		case gpu::sampler_type::aniso_clamp_bias_up1: return "aniso_clamp_bias_up1";
		case gpu::sampler_type::aniso_wrap_bias_up1: return "aniso_wrap_bias_up1";
		case gpu::sampler_type::point_clamp_bias_down1: return "point_clamp_bias_down1";
		case gpu::sampler_type::point_wrap_bias_down1: return "point_wrap_bias_down1";
		case gpu::sampler_type::linear_clamp_bias_down1: return "linear_clamp_bias_down1";
		case gpu::sampler_type::linear_wrap_bias_down1: return "linear_wrap_bias_down1";
		case gpu::sampler_type::aniso_clamp_bias_down1: return "aniso_clamp_bias_down1";
		case gpu::sampler_type::aniso_wrap_bias_down1: return "aniso_wrap_bias_down1";
		case gpu::sampler_type::point_clamp_bias_up2: return "point_clamp_bias_up2";
		case gpu::sampler_type::point_wrap_bias_up2: return "point_wrap_bias_up2";
		case gpu::sampler_type::linear_clamp_bias_up2: return "linear_clamp_bias_up2";
		case gpu::sampler_type::linear_wrap_bias_up2: return "linear_wrap_bias_up2";
		case gpu::sampler_type::aniso_clamp_bias_up2: return "aniso_clamp_bias_up2";
		case gpu::sampler_type::aniso_wrap_bias_up2: return "aniso_wrap_bias_up2";
		case gpu::sampler_type::point_clamp_bias_down2: return "point_clamp_bias_down2";
		case gpu::sampler_type::point_wrap_bias_down2: return "point_wrap_bias_down2";
		case gpu::sampler_type::linear_clamp_bias_down2: return "linear_clamp_bias_down2";
		case gpu::sampler_type::linear_wrap_bias_down2: return "linear_wrap_bias_down2";
		case gpu::sampler_type::aniso_clamp_bias_down2: return "aniso_clamp_bias_down2";
		case gpu::sampler_type::aniso_wrap_bias_down2: return "aniso_wrap_bias_down2";

		default: break;
	}
	return "?";
}

STR_SUPPORT(gpu::sampler_type::value, gpu::sampler_type::count);

const char* as_string(const gpu::pipeline_stage::value& _Value)
{
	switch (_Value)
	{
		case gpu::pipeline_stage::vertex: return "vertex";
		case gpu::pipeline_stage::hull: return "hull";
		case gpu::pipeline_stage::domain: return "domain";
		case gpu::pipeline_stage::geometry: return "geometry";
		case gpu::pipeline_stage::pixel: return "pixel";
		default: break;
	}
	return "?";
}

STR_SUPPORT(gpu::pipeline_stage::value, gpu::pipeline_stage::count);

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

const char* as_string(const gpu::vertex_trait::value& _Value)
{
	switch (_Value)
	{
		//case gpu::vertex_trait::none: return "none";
		case gpu::vertex_trait::position32: return "position32";
		case gpu::vertex_trait::position16: return "position16";
		case gpu::vertex_trait::normal32: return "normal32";
		case gpu::vertex_trait::normal16: return "normal16";
		case gpu::vertex_trait::normal10: return "normal10";
		case gpu::vertex_trait::tangent32: return "tangent32";
		case gpu::vertex_trait::tangent16: return "tangent16";
		case gpu::vertex_trait::tangent10: return "tangent10";
		case gpu::vertex_trait::uv132: return "uv132";
		case gpu::vertex_trait::uvw132: return "uvw132";
		case gpu::vertex_trait::uvwx132: return "uvwx132";
		case gpu::vertex_trait::uv116: return "uv116";
		case gpu::vertex_trait::uvw116: return "uvw116";
		case gpu::vertex_trait::uvwx116: return "uvwx116";
		case gpu::vertex_trait::uv232: return "uv232";
		case gpu::vertex_trait::uvw232: return "uvw232";
		case gpu::vertex_trait::uvwx232: return "uvwx232";
		case gpu::vertex_trait::uv216: return "uv216";
		case gpu::vertex_trait::uvw216: return "uvw216";
		case gpu::vertex_trait::uvwx216: return "uvwx216";
		case gpu::vertex_trait::color132: return "color132";
		case gpu::vertex_trait::color18: return "color18";
		case gpu::vertex_trait::color232: return "color232";
		case gpu::vertex_trait::color28: return "color28";
		
		default: break;
	}
	return "?";
}

oDEFINE_TO_STRING(gpu::vertex_trait::value);

bool from_string(gpu::vertex_trait::value* _pValue, const char* _StrSource)
{
	if (!_stricmp("none", _StrSource)) *_pValue = gpu::vertex_trait::none;
	else if (!_stricmp("position32", _StrSource)) *_pValue = gpu::vertex_trait::position32;
	else if (!_stricmp("position16", _StrSource)) *_pValue = gpu::vertex_trait::position16;
	else if (!_stricmp("normal32", _StrSource)) *_pValue = gpu::vertex_trait::normal32;
	else if (!_stricmp("normal16", _StrSource)) *_pValue = gpu::vertex_trait::normal16;
	else if (!_stricmp("normal10", _StrSource)) *_pValue = gpu::vertex_trait::normal10;
	else if (!_stricmp("tangent32", _StrSource)) *_pValue = gpu::vertex_trait::tangent32;
	else if (!_stricmp("tangent16", _StrSource)) *_pValue = gpu::vertex_trait::tangent16;
	else if (!_stricmp("tangent10", _StrSource)) *_pValue = gpu::vertex_trait::tangent10;
	else if (!_stricmp("uv132", _StrSource)) *_pValue = gpu::vertex_trait::uv132;
	else if (!_stricmp("uvw132", _StrSource)) *_pValue = gpu::vertex_trait::uvw132;
	else if (!_stricmp("uvwx132", _StrSource)) *_pValue = gpu::vertex_trait::uvwx132;
	else if (!_stricmp("uv116", _StrSource)) *_pValue = gpu::vertex_trait::uv116;
	else if (!_stricmp("uvw116", _StrSource)) *_pValue = gpu::vertex_trait::uvw116;
	else if (!_stricmp("uvwx116", _StrSource)) *_pValue = gpu::vertex_trait::uvwx116;
	else if (!_stricmp("uv232", _StrSource)) *_pValue = gpu::vertex_trait::uv232;
	else if (!_stricmp("uvw232", _StrSource)) *_pValue = gpu::vertex_trait::uvw232;
	else if (!_stricmp("uvwx232", _StrSource)) *_pValue = gpu::vertex_trait::uvwx232;
	else if (!_stricmp("uv216", _StrSource)) *_pValue = gpu::vertex_trait::uv216;
	else if (!_stricmp("uvw216", _StrSource)) *_pValue = gpu::vertex_trait::uvw216;
	else if (!_stricmp("uvwx216", _StrSource)) *_pValue = gpu::vertex_trait::uvwx216;
	else if (!_stricmp("color132", _StrSource)) *_pValue = gpu::vertex_trait::color132;
	else if (!_stricmp("color18", _StrSource)) *_pValue = gpu::vertex_trait::color18;
	else if (!_stricmp("color232", _StrSource)) *_pValue = gpu::vertex_trait::color232;
	else if (!_stricmp("color28", _StrSource)) *_pValue = gpu::vertex_trait::color28;
	else return false;
	return true;
}

} // namespace ouro

using namespace ouro;

static_assert(sizeof(ouro::gpu::vertex_range) == 16, "unexpected struct packing for vertex_range");
static_assert(sizeof(fourcc) == 4, "unexpected struct packing for fourcc");
static_assert(sizeof(ouro::resized_type<ouro::surface::format, short>) == 2, "unexpected struct packing for oResizedType<ouro::surface::format, short>");
static_assert(sizeof(oGPU_VERTEX_ELEMENT) == 8, "unexpected struct packing for oGPU_VERTEX_ELEMENT");

bool oGPUParseSemantic(const fourcc& _FourCC, char _Name[5], uint* _pIndex)
{
	*_pIndex = 0;
	*_Name = 0;

	char fcc[5];
	to_string(fcc, _FourCC);
	char* i = &fcc[3];
	while (isspace(*i)) i--;
	if (i == fcc)
		return false;

	while (isdigit(*i)) i--;
	if (i == fcc)
		return false;

	i++;
	*_pIndex = atoi(i);

	// there can be no numbers or space before the semantic index
	char* n = _Name;
	char* f = fcc;
	while (f < i)
	{
		if (isspace(*f) || isdigit(*f))
			return false;
		*n++ = *f++;
	}

	*n = 0;
	return true;
}

uint oGPUCalcVertexSize(const oGPU_VERTEX_ELEMENT* _pElements, uint _NumElements, uint _InputSlot)
{
	bool IsFirstRun = true;
	bool IsInstanceList = false;

	uint size = 0;
	for (uint i = 0; i < _NumElements; i++)
	{
		if (_InputSlot == _pElements[i].InputSlot)
		{
			if (IsFirstRun)
			{
				IsInstanceList = _pElements[i].Instanced;
				IsFirstRun = false;
			}

			else
				oASSERT(IsInstanceList == _pElements[i].Instanced, "Elements in the same slot must either be all instanced or all not instanced.");

			size += ouro::surface::element_size(_pElements[i].Format);
		}
	}

	return size;
}

uint oGPUCalcNumInputSlots(const oGPU_VERTEX_ELEMENT* _pElements, uint _NumElements)
{
	uchar nSlots = 0;

	#ifdef _DEBUG
		uint lastSlot = ouro::invalid;
	#endif

	for (uint i = 0; i < _NumElements; i++)
	{
		nSlots = __max(nSlots, _pElements[i].InputSlot+1);

		#ifdef _DEBUG
			oASSERT(lastSlot == ouro::invalid 
				|| lastSlot == _pElements[i].InputSlot 
				|| lastSlot == static_cast<unsigned int>(_pElements[i].InputSlot - 1), "Non-packed elements");
			lastSlot = _pElements[i].InputSlot;
		#endif
	}

	return nSlots;
}
