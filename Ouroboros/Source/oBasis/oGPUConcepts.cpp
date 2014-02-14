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
	oDEFINE_FROM_STRING(_T, _NumTs)

namespace ouro {

static_assert(sizeof(gpu::vertex_range) == 16, "unexpected struct packing for vertex_range");
static_assert(sizeof(fourcc) == 4, "unexpected struct packing for fourcc");
static_assert(sizeof(surface::format) == 1, "unexpected size");

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

const char* as_string(const gpu::face_type::value& _Value)
{
	switch (_Value)
	{
		case gpu::face_type::unknown: return "unknown";
		case gpu::face_type::front_ccw: return "front_ccw";
		case gpu::face_type::front_cw: return "front_cw";
		case gpu::face_type::outline: return "outline";
		default: break;
	}
	return "?";
}
		
STR_SUPPORT(gpu::face_type::value, gpu::face_type::count);
		
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

const char* as_string(const gpu::vertex_semantic::value& _Value)
{
	switch (_Value)
	{
		case gpu::vertex_semantic::position: return "position";
		case gpu::vertex_semantic::normal: return "normal";
		case gpu::vertex_semantic::tangent: return "tangent";
		case gpu::vertex_semantic::texcoord: return "texcoord";
		case gpu::vertex_semantic::color: return "color";
		default: break;
	}
	return "?";
}
	
STR_SUPPORT(gpu::vertex_semantic::value, gpu::vertex_semantic::count);

const char* as_string(const gpu::vertex_layout::value& _Value)
{
	switch (_Value)
	{
		case gpu::vertex_layout::none: return "none";
		case gpu::vertex_layout::pos: return "pos";
		case gpu::vertex_layout::color: return "color";
		case gpu::vertex_layout::pos_color: return "pos_color";
		case gpu::vertex_layout::pos_nrm: return "pos_nrm";
		case gpu::vertex_layout::pos_nrm_tan: return "pos_nrm_tan";
		case gpu::vertex_layout::pos_nrm_tan_uv0: return "pos_nrm_tan_uv0";
		case gpu::vertex_layout::pos_nrm_tan_uvwx0: return "pos_nrm_tan_uvwx0";
		case gpu::vertex_layout::pos_uv0: return "pos_uv0";
		case gpu::vertex_layout::pos_uvwx0: return "pos_uvwx0";
		case gpu::vertex_layout::uv0: return "uv0";
		case gpu::vertex_layout::uvwx0: return "uvwx0";
		case gpu::vertex_layout::uv0_color: return "uv0_color";
		case gpu::vertex_layout::uvwx0_color: return "uvwx0_color";
		default: break;
	}
	return "?";
}

STR_SUPPORT(gpu::vertex_layout::value, gpu::vertex_layout::count);

const char* as_string(const gpu::vertex_usage::value& _Value)
{
	switch (_Value)
	{
		case gpu::vertex_usage::dynamic_vertices: return "dynamic_vertices";
		case gpu::vertex_usage::static_vertices: return "static_vertices";
		case gpu::vertex_usage::per_instance_vertices: return "per_instance_vertices";
		default: break;
	}
	return "?";
}

oDEFINE_TO_STRING(gpu::vertex_usage::value);

namespace gpu {

uint num_primitives(const primitive_type::value& _PrimitiveType, uint _NumIndices, uint _NumVertices)
{
	uint n = _NumIndices;
	switch (_PrimitiveType)
	{
		case primitive_type::points: n = _NumVertices; break;
		case primitive_type::lines: n /= 2; break;
		case primitive_type::line_strips: n--; break;
		case primitive_type::triangles: n /= 3; break;
		case primitive_type::triangle_strips: n -= 2; break;
		default: oTHROW_INVARG("unsupported primitive type");
	}
	
	return n;
}

uint vertex_size(const vertex_layout::value& _Layout)
{
	static uchar sSizes[] =
	{
		0,
		sizeof(float3),
		sizeof(color),
		sizeof(float3) + sizeof(color),
		sizeof(float3) + sizeof(dec3n),
		sizeof(float3) + sizeof(dec3n) + sizeof(dec3n),
		sizeof(float3) + sizeof(dec3n) + sizeof(dec3n) + sizeof(half2),
		sizeof(float3) + sizeof(dec3n) + sizeof(dec3n) + sizeof(half4),
		sizeof(float3) + sizeof(half2),
		sizeof(float3) + sizeof(half4),
		sizeof(half2),
		sizeof(half4),
		sizeof(half2) + sizeof(color),
		sizeof(half4) + sizeof(color),
	};
	static_assert(oCOUNTOF(sSizes) == vertex_layout::count, "array mismatch");
	return sSizes[_Layout];
}

buffer_info make_index_buffer_info(uint _NumIndices, uint _NumVertices)
{
	buffer_info i;
	i.type = buffer_type::index;
	i.format = has_16bit_indices(_NumVertices) ? surface::r16_uint : surface::r32_uint;
	i.array_size = _NumIndices;
	i.struct_byte_size = static_cast<ushort>(surface::element_size(i.format));
	return i;
}

buffer_info make_vertex_buffer_info(uint _NumVertices, const vertex_layout::value& _Layout)
{
	buffer_info i;
	i.type = buffer_type::vertex;
	i.array_size = _NumVertices;
	i.struct_byte_size = static_cast<ushort>(vertex_size(_Layout));
	i.format = surface::unknown;
	return i;
}

void copy_indices(surface::mapped_subresource& _Destination, const surface::const_mapped_subresource& _Source, uint _NumIndices)
{
	if (_Destination.row_pitch == _Source.row_pitch)
		memcpy(_Destination.data, _Source.data, _NumIndices * _Destination.row_pitch);
	else if (_Destination.row_pitch == sizeof(uint) && _Source.row_pitch == sizeof(ushort))
		memcpyustoui((uint*)_Destination.data, (const ushort*)_Source.data, _NumIndices);
	else if (_Destination.row_pitch == sizeof(ushort) && _Source.row_pitch == sizeof(uint))
		memcpyuitous((ushort*)_Destination.data, (const uint*)_Source.data, _NumIndices);
	else
		oTHROW_INVARG("bad strides");
}

template<typename DstT, typename SrcT>
void copy_vertex_element(DstT* oRESTRICT _pDestination, uint _DestinationPitch, const SrcT* oRESTRICT _pSource, uint _SourcePitch, uint _NumVertices)
{
	if (std::is_same<DstT, SrcT>::value)
		memcpy2d(_pDestination, _DestinationPitch, _pSource, _SourcePitch, _SourcePitch, _NumVertices);
	else
	{
		const DstT* oRESTRICT end = byte_add(_pDestination, _DestinationPitch * _NumVertices);
		while (_pDestination < end)
		{
			*_pDestination = *_pSource;
			_pDestination = byte_add(_pDestination, _DestinationPitch);
			_pSource = byte_add(_pSource, _SourcePitch);
		}
	}
}

template<>
void copy_vertex_element<half2, float3>(half2* oRESTRICT _pDestination, uint _DestinationPitch, const float3* oRESTRICT _pSource, uint _SourcePitch, uint _NumVertices)
{
	const half2* oRESTRICT end = byte_add(_pDestination, _DestinationPitch * _NumVertices);
	while (_pDestination < end)
	{
		_pDestination->x = _pSource->x;
		_pDestination->y = _pSource->y;
		_pDestination = byte_add(_pDestination, _DestinationPitch);
		_pSource = byte_add(_pSource, _SourcePitch);
	}
}

#if 0
template<>
void copy_vertex_element<ushort4, float3>(ushort4* oRESTRICT _pDestination, uint _DestinationPitch, const float3* oRESTRICT _pSource, uint _SourcePitch, uint _NumVertices)
{
	const ushort4* oRESTRICT end = byte_add(_pDestination, _DestinationPitch * _NumVertices);
	while (_pDestination < end)
	{
		_pDestination->x = f32ton16(_pSource->x);
		_pDestination->y = f32ton16(_pSource->y); 
		_pDestination->z = f32ton16(_pSource->z);
		_pDestination->w = 65535;//f32ton16(1.0f);
		_pDestination = byte_add(_pDestination, _DestinationPitch);
		_pSource = byte_add(_pSource, _SourcePitch);
	}
}

template<>
void copy_vertex_element<float3, ushort4>(float3* oRESTRICT _pDestination, uint _DestinationPitch, const ushort4* oRESTRICT _pSource, uint _SourcePitch, uint _NumVertices)
{
	const float3* oRESTRICT end = byte_add(_pDestination, _DestinationPitch * _NumVertices);
	while (_pDestination < end)
	{
		_pDestination->x = n16tof32(_pSource->x);
		_pDestination->y = n16tof32(_pSource->y); 
		_pDestination->z = n16tof32(_pSource->z);
		_pDestination = byte_add(_pDestination, _DestinationPitch);
		_pSource = byte_add(_pSource, _SourcePitch);
	}
}
#endif
template<>
void copy_vertex_element<half4, float3>(half4* oRESTRICT _pDestination, uint _DestinationPitch, const float3* oRESTRICT _pSource, uint _SourcePitch, uint _NumVertices)
{
	const half4* oRESTRICT end = byte_add(_pDestination, _DestinationPitch * _NumVertices);
	while (_pDestination < end)
	{
		_pDestination->x = _pSource->x;
		_pDestination->y = _pSource->y;
		_pDestination->z = _pSource->z;
		_pDestination->w = 0.0f;
		_pDestination = byte_add(_pDestination, _DestinationPitch);
		_pSource = byte_add(_pSource, _SourcePitch);
	}
}

template<>
void copy_vertex_element<float3, half4>(float3* oRESTRICT _pDestination, uint _DestinationPitch, const half4* oRESTRICT _pSource, uint _SourcePitch, uint _NumVertices)
{
	const float3* oRESTRICT end = byte_add(_pDestination, _DestinationPitch * _NumVertices);
	while (_pDestination < end)
	{
		_pDestination->x = _pSource->x;
		_pDestination->y = _pSource->y;
		_pDestination->z = _pSource->z;
		_pDestination = byte_add(_pDestination, _DestinationPitch);
		_pSource = byte_add(_pSource, _SourcePitch);
	}
}

template<>
void copy_vertex_element<half4, float4>(half4* oRESTRICT _pDestination, uint _DestinationPitch, const float4* oRESTRICT _pSource, uint _SourcePitch, uint _NumVertices)
{
	const half4* oRESTRICT end = byte_add(_pDestination, _DestinationPitch * _NumVertices);
	while (_pDestination < end)
	{
		_pDestination->x = _pSource->x;
		_pDestination->y = _pSource->y;
		_pDestination->z = _pSource->z;
		_pDestination->w = _pSource->w;
		_pDestination = byte_add(_pDestination, _DestinationPitch);
		_pSource = byte_add(_pSource, _SourcePitch);
	}
}

template<>
void copy_vertex_element<float4, half4>(float4* oRESTRICT _pDestination, uint _DestinationPitch, const half4* oRESTRICT _pSource, uint _SourcePitch, uint _NumVertices)
{
	const float4* oRESTRICT end = byte_add(_pDestination, _DestinationPitch * _NumVertices);
	while (_pDestination < end)
	{
		_pDestination->x = _pSource->x;
		_pDestination->y = _pSource->y;
		_pDestination->z = _pSource->z;
		_pDestination->w = _pSource->w;
		_pDestination = byte_add(_pDestination, _DestinationPitch);
		_pSource = byte_add(_pSource, _SourcePitch);
	}
}

template<typename DstT, typename SrcT>
static void copy_semantic(void* oRESTRICT & _pDestination, uint _DestinationPitch, const vertex_layout::value& _DestinationLayout
	, const SrcT* oRESTRICT _pSource, uint _SourcePitch, uint _NumVertices
	, bool (*has_element)(const vertex_layout::value& _Layout))
{
	if (has_element(_DestinationLayout))
	{
		if (_pSource)
			copy_vertex_element<DstT, SrcT>((DstT*)_pDestination, _DestinationPitch, (SrcT*)_pSource, _SourcePitch, _NumVertices);
		else
			memset2d4(_pDestination, _DestinationPitch, 0, sizeof(DstT), _NumVertices);

		_pDestination = byte_add(_pDestination, sizeof(DstT));
	}
}

void copy_vertices(void* oRESTRICT _pDestination, const vertex_layout::value& _DestinationLayout, const vertex_source& _Source, uint _NumVertices)
{
	const uint DestinationPitch = vertex_size(_DestinationLayout);

	copy_semantic<float3, float3>(_pDestination, DestinationPitch, _DestinationLayout, _Source.positionsf, _Source.positionf_pitch, _NumVertices, has_positions);
	
	if (_Source.normals)
		copy_semantic<dec3n, dec3n>(_pDestination, DestinationPitch, _DestinationLayout, _Source.normals, _Source.normal_pitch, _NumVertices, has_normals);
	else
		copy_semantic<dec3n, float3>(_pDestination, DestinationPitch, _DestinationLayout, _Source.normalsf, _Source.normalf_pitch, _NumVertices, has_normals);
	
	if (_Source.tangents)
		copy_semantic<dec3n, dec3n>(_pDestination, DestinationPitch, _DestinationLayout, _Source.tangents, _Source.tangent_pitch, _NumVertices, has_tangents);
	else
		copy_semantic<dec3n, float4>(_pDestination, DestinationPitch, _DestinationLayout, _Source.tangentsf, _Source.tangentf_pitch, _NumVertices, has_tangents);
	
	const int src_uv0s = (_Source.uv0s || _Source.uv0sf) ? 1 : 0;
	const int src_uvw0s = (_Source.uvw0sf) ? 1 : 0;
	const int src_uvwx0s = (_Source.uvwx0s || _Source.uvwx0sf) ? 1 : 0;

	oCHECK((src_uv0s + src_uvw0s + src_uvwx0s) == 1, "only one of uv0 uvw0 or uvwx0's can be specified");

	if (_Source.uv0s)
		copy_semantic<half2, half2>(_pDestination, DestinationPitch, _DestinationLayout, _Source.uv0s, _Source.uv0_pitch, _NumVertices, has_uv0s);
	else if (!has_uvwx0s(_DestinationLayout) && _Source.uvw0sf)
		copy_semantic<half2, float3>(_pDestination, DestinationPitch, _DestinationLayout, _Source.uvw0sf, _Source.uvw0f_pitch, _NumVertices, has_uv0s);
	else 
		copy_semantic<half2, float2>(_pDestination, DestinationPitch, _DestinationLayout, _Source.uv0sf, _Source.uv0f_pitch, _NumVertices, has_uv0s);

	if (_Source.uvwx0s)
		copy_semantic<half4, half4>(_pDestination, DestinationPitch, _DestinationLayout, _Source.uvwx0s, _Source.uvwx0_pitch, _NumVertices, has_uvwx0s);
	else if (_Source.uvwx0sf)
		copy_semantic<half4, float4>(_pDestination, DestinationPitch, _DestinationLayout, _Source.uvwx0sf, _Source.uvwx0f_pitch, _NumVertices, has_uvwx0s);
	else
		copy_semantic<half4, float3>(_pDestination, DestinationPitch, _DestinationLayout, _Source.uvw0sf, _Source.uvw0f_pitch, _NumVertices, has_uvwx0s);

	if (_Source.colors)
		copy_semantic<color, color>(_pDestination, DestinationPitch, _DestinationLayout, _Source.colors, _Source.color_pitch, _NumVertices, has_colors);
}

	} // namespace gpu
} // namespace ouro
