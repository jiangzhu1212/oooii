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
#include <oMesh/mesh.h>
#include <oBase/throw.h>
#include "mesh_template.h"

#define STR_SUPPORT(_T, _NumTs) \
	oDEFINE_TO_STRING(_T) \
	oDEFINE_FROM_STRING(_T, _NumTs)

namespace ouro {

const char* as_string(const mesh::primitive_type::value& _Value)
{
	switch (_Value)
	{
		case mesh::primitive_type::unknown: return "unknown";
		case mesh::primitive_type::points: return "points";
		case mesh::primitive_type::lines: return "lines";
		case mesh::primitive_type::line_strips: return "line_strips";
		case mesh::primitive_type::triangles: return "triangles";
		case mesh::primitive_type::triangle_strips: return "triangle_strips";
		case mesh::primitive_type::lines_adjacency: return "lines_adjacency";
		case mesh::primitive_type::line_strips_adjacency: return "line_strips_adjacency";
		case mesh::primitive_type::triangles_adjacency: return "triangles_adjacency";
		case mesh::primitive_type::triangle_strips_adjacency: return "triangle_strips_adjacency";
		case mesh::primitive_type::patches1: return "patches1";
		case mesh::primitive_type::patches2: return "patches2";
		case mesh::primitive_type::patches3: return "patches3";
		case mesh::primitive_type::patches4: return "patches4";
		case mesh::primitive_type::patches5: return "patches5";
		case mesh::primitive_type::patches6: return "patches6";
		case mesh::primitive_type::patches7: return "patches7";
		case mesh::primitive_type::patches8: return "patches8";
		case mesh::primitive_type::patches9: return "patches9";
		case mesh::primitive_type::patches10: return "patches10";
		case mesh::primitive_type::patches11: return "patches11";
		case mesh::primitive_type::patches12: return "patches12";
		case mesh::primitive_type::patches13: return "patches13";
		case mesh::primitive_type::patches14: return "patches14";
		case mesh::primitive_type::patches15: return "patches15";
		case mesh::primitive_type::patches16: return "patches16";
		case mesh::primitive_type::patches17: return "patches17";
		case mesh::primitive_type::patches18: return "patches18";
		case mesh::primitive_type::patches19: return "patches19";
		case mesh::primitive_type::patches20: return "patches20";
		case mesh::primitive_type::patches21: return "patches21";
		case mesh::primitive_type::patches22: return "patches22";
		case mesh::primitive_type::patches23: return "patches23";
		case mesh::primitive_type::patches24: return "patches24";
		case mesh::primitive_type::patches25: return "patches25";
		case mesh::primitive_type::patches26: return "patches26";
		case mesh::primitive_type::patches27: return "patches27";
		case mesh::primitive_type::patches28: return "patches28";
		case mesh::primitive_type::patches29: return "patches29";
		case mesh::primitive_type::patches30: return "patches30";
		case mesh::primitive_type::patches31: return "patches31";
		case mesh::primitive_type::patches32: return "patches32";

		default: break;
	}
	return "?";
}

STR_SUPPORT(mesh::primitive_type::value, mesh::primitive_type::count);

const char* as_string(const mesh::face_type::value& _Value)
{
	switch (_Value)
	{
		case mesh::face_type::unknown: return "unknown";
		case mesh::face_type::front_ccw: return "front_ccw";
		case mesh::face_type::front_cw: return "front_cw";
		case mesh::face_type::outline: return "outline";
		default: break;
	}
	return "?";
}
		
STR_SUPPORT(mesh::face_type::value, mesh::face_type::count);

const char* as_string(const mesh::semantic::value& _Value)
{
	switch (_Value)
	{
		case mesh::semantic::position: return "position";
		case mesh::semantic::normal: return "normal";
		case mesh::semantic::tangent: return "tangent";
		case mesh::semantic::texcoord: return "texcoord";
		case mesh::semantic::color: return "color";
		default: break;
	}
	return "?";
}
	
STR_SUPPORT(mesh::semantic::value, mesh::semantic::count);

const char* as_string(const mesh::layout::value& _Value)
{
	switch (_Value)
	{
		case mesh::layout::none: return "none";
		case mesh::layout::pos: return "pos";
		case mesh::layout::color: return "color";
		case mesh::layout::pos_color: return "pos_color";
		case mesh::layout::pos_nrm: return "pos_nrm";
		case mesh::layout::pos_nrm_tan: return "pos_nrm_tan";
		case mesh::layout::pos_nrm_tan_uv0: return "pos_nrm_tan_uv0";
		case mesh::layout::pos_nrm_tan_uvwx0: return "pos_nrm_tan_uvwx0";
		case mesh::layout::pos_uv0: return "pos_uv0";
		case mesh::layout::pos_uvwx0: return "pos_uvwx0";
		case mesh::layout::uv0: return "uv0";
		case mesh::layout::uvwx0: return "uvwx0";
		case mesh::layout::uv0_color: return "uv0_color";
		case mesh::layout::uvwx0_color: return "uvwx0_color";
		default: break;
	}
	return "?";
}

STR_SUPPORT(mesh::layout::value, mesh::layout::count);

const char* as_string(const mesh::usage::value& _Value)
{
	switch (_Value)
	{
		case mesh::usage::per_mesh_static: return "per_mesh_static";
		case mesh::usage::per_mesh_dynamic: return "per_mesh_dynamic";
		case mesh::usage::per_instance_static: return "per_instance_static";
		case mesh::usage::per_instance_dynamic: return "per_instance_dynamic";
		default: break;
	}
	return "?";
}

oDEFINE_TO_STRING(mesh::usage::value);

namespace mesh {

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

uint vertex_size(const layout::value& _Layout)
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
	static_assert(oCOUNTOF(sSizes) == layout::count, "array mismatch");
	return sSizes[_Layout];
}

void flip_winding_order(uint _BaseIndexIndex, ushort* _pIndices, uint _NumIndices)
{
	oCHECK((_BaseIndexIndex % 3) == 0, "Indices is not divisible by 3, so thus is not triangles and cannot be re-winded");
	oCHECK((_NumIndices % 3) == 0, "Indices is not divisible by 3, so thus is not triangles and cannot be re-winded");
	for (uint i = _BaseIndexIndex; i < _NumIndices; i += 3)
		std::swap(_pIndices[i+1], _pIndices[i+2]);
}

void flip_winding_order(uint _BaseIndexIndex, uint* _pIndices, uint _NumIndices)
{
	oCHECK((_BaseIndexIndex % 3) == 0, "Indices is not divisible by 3, so thus is not triangles and cannot be re-winded");
	oCHECK((_NumIndices % 3) == 0, "Indices is not divisible by 3, so thus is not triangles and cannot be re-winded");
	for (uint i = _BaseIndexIndex; i < _NumIndices; i += 3)
		std::swap(_pIndices[i+1], _pIndices[i+2]);
}

void copy_indices(void* oRESTRICT _pDestination, uint _DestinationPitch, const void* oRESTRICT _pSource, uint _SourcePitch, uint _NumIndices)
{
	if (_DestinationPitch == sizeof(uint) && _SourcePitch == sizeof(ushort))
		copy_indices((uint*)_pDestination, (const ushort*)_pSource, _NumIndices);
	else if (_DestinationPitch == sizeof(ushort) && _SourcePitch == sizeof(uint))
		copy_indices((ushort*)_pDestination, (const uint*)_pSource, _NumIndices);

	oTHROW_INVARG("unsupported index pitches (src=%d, dst=%d)", _SourcePitch, _DestinationPitch);
}

void copy_indices(ushort* oRESTRICT _pDestination, const uint* oRESTRICT _pSource, uint _NumIndices)
{
	const uint* end = &_pSource[_NumIndices];
	while (_pSource < end)
	{
		if (*_pSource > 65535) oTHROW_INVARG("truncating a uint (%d) to a ushort in a way that will change its value.", *_pSource);
		*_pDestination++ = (*_pSource++) & 0xffff;
	}
}

void copy_indices(uint* oRESTRICT _pDestination, const ushort* oRESTRICT _pSource, uint _NumIndices)
{
	const ushort* end = &_pSource[_NumIndices];
	while (_pSource < end)
		*_pDestination++ = *_pSource++;
}

template<typename DstT, typename SrcT>
static void copy_vertex_element(DstT* oRESTRICT _pDestination, uint _DestinationPitch, const SrcT* oRESTRICT _pSource, uint _SourcePitch, uint _NumVertices)
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
static void copy_vertex_element<half2, float3>(half2* oRESTRICT _pDestination, uint _DestinationPitch, const float3* oRESTRICT _pSource, uint _SourcePitch, uint _NumVertices)
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

//template<>
//static void copy_vertex_element<ushort4, float3>(ushort4* oRESTRICT _pDestination, uint _DestinationPitch, const float3* oRESTRICT _pSource, uint _SourcePitch, uint _NumVertices)
//{
//	const ushort4* oRESTRICT end = byte_add(_pDestination, _DestinationPitch * _NumVertices);
//	while (_pDestination < end)
//	{
//		_pDestination->x = f32ton16(_pSource->x);
//		_pDestination->y = f32ton16(_pSource->y); 
//		_pDestination->z = f32ton16(_pSource->z);
//		_pDestination->w = 65535;//f32ton16(1.0f);
//		_pDestination = byte_add(_pDestination, _DestinationPitch);
//		_pSource = byte_add(_pSource, _SourcePitch);
//	}
//}

//template<>
//static void copy_vertex_element<float3, ushort4>(float3* oRESTRICT _pDestination, uint _DestinationPitch, const ushort4* oRESTRICT _pSource, uint _SourcePitch, uint _NumVertices)
//{
//	const float3* oRESTRICT end = byte_add(_pDestination, _DestinationPitch * _NumVertices);
//	while (_pDestination < end)
//	{
//		_pDestination->x = n16tof32(_pSource->x);
//		_pDestination->y = n16tof32(_pSource->y); 
//		_pDestination->z = n16tof32(_pSource->z);
//		_pDestination = byte_add(_pDestination, _DestinationPitch);
//		_pSource = byte_add(_pSource, _SourcePitch);
//	}
//}

template<>
static void copy_vertex_element<half4, float3>(half4* oRESTRICT _pDestination, uint _DestinationPitch, const float3* oRESTRICT _pSource, uint _SourcePitch, uint _NumVertices)
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

//template<>
//static void copy_vertex_element<float3, half4>(float3* oRESTRICT _pDestination, uint _DestinationPitch, const half4* oRESTRICT _pSource, uint _SourcePitch, uint _NumVertices)
//{
//	const float3* oRESTRICT end = byte_add(_pDestination, _DestinationPitch * _NumVertices);
//	while (_pDestination < end)
//	{
//		_pDestination->x = _pSource->x;
//		_pDestination->y = _pSource->y;
//		_pDestination->z = _pSource->z;
//		_pDestination = byte_add(_pDestination, _DestinationPitch);
//		_pSource = byte_add(_pSource, _SourcePitch);
//	}
//}

template<>
static void copy_vertex_element<half4, float4>(half4* oRESTRICT _pDestination, uint _DestinationPitch, const float4* oRESTRICT _pSource, uint _SourcePitch, uint _NumVertices)
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

//template<>
//static void copy_vertex_element<float4, half4>(float4* oRESTRICT _pDestination, uint _DestinationPitch, const half4* oRESTRICT _pSource, uint _SourcePitch, uint _NumVertices)
//{
//	const float4* oRESTRICT end = byte_add(_pDestination, _DestinationPitch * _NumVertices);
//	while (_pDestination < end)
//	{
//		_pDestination->x = _pSource->x;
//		_pDestination->y = _pSource->y;
//		_pDestination->z = _pSource->z;
//		_pDestination->w = _pSource->w;
//		_pDestination = byte_add(_pDestination, _DestinationPitch);
//		_pSource = byte_add(_pSource, _SourcePitch);
//	}
//}

template<typename DstT, typename SrcT>
static void copy_semantic(void* oRESTRICT & _pDestination, uint _DestinationPitch, const layout::value& _DestinationLayout
	, const SrcT* oRESTRICT _pSource, uint _SourcePitch, uint _NumVertices
	, bool (*has_element)(const layout::value& _Layout))
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

void copy_vertices(void* oRESTRICT _pDestination, const layout::value& _DestinationLayout, const vertex_soa& _Source, uint _NumVertices)
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

void calc_min_max_indices(const uint* oRESTRICT _pIndices, uint _StartIndex, uint _NumIndices, uint _NumVertices, uint* oRESTRICT _pMinVertex, uint* oRESTRICT _pMaxVertex)
{
	detail::calc_min_max_indices(_pIndices, _StartIndex, _NumIndices, _NumVertices, _pMinVertex, _pMaxVertex);
}

void calc_min_max_indices(const ushort* oRESTRICT _pIndices, uint _StartIndex, uint _NumIndices, uint _NumVertices, uint* oRESTRICT _pMinVertex, uint* oRESTRICT _pMaxVertex)
{
	detail::calc_min_max_indices(_pIndices, _StartIndex, _NumIndices, _NumVertices, _pMinVertex, _pMaxVertex);
}

boundf calc_bound(const float3* _pVertices, uint _VertexStride, uint _NumVertices)
{
	return detail::calc_bound(_pVertices, _VertexStride, _NumVertices);
}

void transform_points(const float4x4& _Matrix, float3* oRESTRICT _pDestination, uint _DestinationStride, const float3* oRESTRICT _pSource, uint _SourceStride, uint _NumPoints)
{
	detail::transform_points(_Matrix, _pDestination, _DestinationStride, _pSource, _SourceStride, _NumPoints);
}

void transform_vectors(const float4x4& _Matrix, float3* oRESTRICT _pDestination, uint _DestinationStride, const float3* oRESTRICT _pSource, uint _SourceStride, uint _NumVectors)
{
	detail::transform_vectors(_Matrix, _pDestination, _DestinationStride, _pSource, _SourceStride, _NumVectors);
}

void remove_degenerates(const float3* oRESTRICT _pPositions, uint _NumPositions, uint* oRESTRICT _pIndices, uint _NumIndices, uint* oRESTRICT _pNewNumIndices)
{
	detail::remove_degenerates(_pPositions, _NumPositions, _pIndices, _NumIndices, _pNewNumIndices);
}

void remove_degenerates(const float3* oRESTRICT _pPositions, uint _NumPositions, ushort* oRESTRICT _pIndices, uint _NumIndices, uint* oRESTRICT _pNewNumIndices)
{
	detail::remove_degenerates(_pPositions, _NumPositions, _pIndices, _NumIndices, _pNewNumIndices);
}

void calc_face_normals(float3* oRESTRICT _pFaceNormals, const uint* oRESTRICT _pIndices, uint _NumIndices, const float3* oRESTRICT _pPositions, uint _NumPositions, bool _CCW)
{
	detail::calc_face_normals(_pFaceNormals, _pIndices, _NumIndices, _pPositions, _NumPositions, _CCW);
}

void calc_face_normals(float3* oRESTRICT _pFaceNormals, const ushort* oRESTRICT _pIndices, uint _NumIndices, const float3* oRESTRICT _pPositions, uint _NumPositions, bool _CCW)
{
	detail::calc_face_normals(_pFaceNormals, _pIndices, _NumIndices, _pPositions, _NumPositions, _CCW);
}

void calc_vertex_normals(float3* _pVertexNormals, const uint* _pIndices, uint _NumIndices, const float3* _pPositions, uint _NumPositions, bool _CCW, bool _OverwriteAll)
{
	detail::calc_vertex_normals(_pVertexNormals, _pIndices, _NumIndices, _pPositions, _NumPositions, _CCW, _OverwriteAll);
}

void calc_vertex_normals(float3* _pVertexNormals, const ushort* _pIndices, uint _NumIndices, const float3* _pPositions, uint _NumPositions, bool _CCW, bool _OverwriteAll)
{
	detail::calc_vertex_normals(_pVertexNormals, _pIndices, _NumIndices, _pPositions, _NumPositions, _CCW, _OverwriteAll);
}

void calc_vertex_tangents(float4* _pTangents, const uint* _pIndices, uint _NumIndices, const float3* _pPositions, const float3* _pNormals, const float3* _pTexcoords, uint _NumVertices)
{
	detail::calc_vertex_tangents(_pTangents, _pIndices, _NumIndices, _pPositions, _pNormals, _pTexcoords, _NumVertices);
}

void calc_vertex_tangents(float4* _pTangents, const ushort* _pIndices, uint _NumIndices, const float3* _pPositions, const float3* _pNormals, const float3* _pTexcoords, uint _NumVertices)
{
	detail::calc_vertex_tangents(_pTangents, _pIndices, _NumIndices, _pPositions, _pNormals, _pTexcoords, _NumVertices);
}

void calc_texcoords(const boundf& _Bound, const uint* _pIndices, uint _NumIndices, const float3* _pPositions, float2* _pOutTexcoords, uint _NumVertices, double* _pSolveTime)
{
	detail::calc_texcoords(_Bound, _pIndices, _NumIndices, _pPositions, _pOutTexcoords, _NumVertices, _pSolveTime);
}

void calc_texcoords(const boundf& _Bound, const uint* _pIndices, uint _NumIndices, const float3* _pPositions, float3* _pOutTexcoords, uint _NumVertices, double* _pSolveTime)
{
	detail::calc_texcoords(_Bound, _pIndices, _NumIndices, _pPositions, _pOutTexcoords, _NumVertices, _pSolveTime);
}

void calc_texcoords(const boundf& _Bound, const ushort* _pIndices, uint _NumIndices, const float3* _pPositions, float2* _pOutTexcoords, uint _NumVertices, double* _pSolveTime)
{
	detail::calc_texcoords(_Bound, _pIndices, _NumIndices, _pPositions, _pOutTexcoords, _NumVertices, _pSolveTime);
}

void calc_texcoords(const boundf& _Bound, const ushort* _pIndices, uint _NumIndices, const float3* _pPositions, float3* _pOutTexcoords, uint _NumVertices, double* _pSolveTime)
{
	detail::calc_texcoords(_Bound, _pIndices, _NumIndices, _pPositions, _pOutTexcoords, _NumVertices, _pSolveTime);
}

namespace TerathonEdges {

	/** <citation
	usage="Implementation" 
	reason="tangents can be derived, and this is how to do it" 
	author="Eric Lengyel"
	description="http://www.terathon.com/code/edges.html"
	license="*** Assumed Public Domain ***"
	licenseurl="http://www.terathon.com/code/edges.html"
	modification="Minor changes to not limit algo to 65536 indices and some fixes to get it compiling"
	/>*/

	// $(CitedCodeBegin)

	// Building an Edge List for an Arbitrary Mesh
	// The following code builds a list of edges for an arbitrary triangle 
	// mesh and has O(n) running time in the number of triangles n in the 
	// pGeometry-> The edgeArray parameter must point to a previously allocated 
	// array of Edge structures large enough to hold all of the mesh's 
	// edges, which in the worst possible case is 3 times the number of 
	// triangles in the pGeometry->

	// An edge list is useful for many geometric algorithms in computer 
	// graphics. In particular, an edge list is necessary for stencil 
	// shadows.

	struct Edge
	{
		unsigned int      vertexIndex[2]; 
		unsigned int      faceIndex[2];
	};


	struct Triangle
	{
		unsigned int      index[3];
	};


	long BuildEdges(long vertexCount, long triangleCount,
		const Triangle *triangleArray, Edge *edgeArray)
	{
		long maxEdgeCount = triangleCount * 3;
		unsigned int *firstEdge = new unsigned int[vertexCount + maxEdgeCount];
		unsigned int *nextEdge = firstEdge + vertexCount;

		for (long a = 0; a < vertexCount; a++) firstEdge[a] = 0xFFFFFFFF;

		// First pass over all triangles. This finds all the edges satisfying the
		// condition that the first vertex index is less than the second vertex index
		// when the direction from the first vertex to the second vertex represents
		// a counterclockwise winding around the triangle to which the edge belongs.
		// For each edge found, the edge index is stored in a linked list of edges
		// belonging to the lower-numbered vertex index i. This allows us to quickly
		// find an edge in the second pass whose higher-numbered vertex index is i.

		long edgeCount = 0;
		const Triangle *triangle = triangleArray;
		for (long a = 0; a < triangleCount; a++)
		{
			long i1 = triangle->index[2];
			for (long b = 0; b < 3; b++)
			{
				long i2 = triangle->index[b];
				if (i1 < i2)
				{
					Edge *edge = &edgeArray[edgeCount];

					edge->vertexIndex[0] = (unsigned int) i1;
					edge->vertexIndex[1] = (unsigned int) i2;
					edge->faceIndex[0] = (unsigned int) a;
					edge->faceIndex[1] = (unsigned int) a;

					long edgeIndex = firstEdge[i1];
					if (edgeIndex == 0xFFFFFFFF)
					{
						firstEdge[i1] = edgeCount;
					}
					else
					{
						for (;;)
						{
							long index = nextEdge[edgeIndex];
							if (index == 0xFFFFFFFF)
							{
								nextEdge[edgeIndex] = edgeCount;
								break;
							}

							edgeIndex = index;
						}
					}

					nextEdge[edgeCount] = 0xFFFFFFFF;
					edgeCount++;
				}

				i1 = i2;
			}

			triangle++;
		}

		// Second pass over all triangles. This finds all the edges satisfying the
		// condition that the first vertex index is greater than the second vertex index
		// when the direction from the first vertex to the second vertex represents
		// a counterclockwise winding around the triangle to which the edge belongs.
		// For each of these edges, the same edge should have already been found in
		// the first pass for a different triangle. So we search the list of edges
		// for the higher-numbered vertex index for the matching edge and fill in the
		// second triangle index. The maximum number of comparisons in this search for
		// any vertex is the number of edges having that vertex as an endpoint.

		triangle = triangleArray;
		for (long a = 0; a < triangleCount; a++)
		{
			long i1 = triangle->index[2];
			for (long b = 0; b < 3; b++)
			{
				long i2 = triangle->index[b];
				if (i1 > i2)
				{
					for (long edgeIndex = firstEdge[i2]; edgeIndex != 0xFFFFFFFF;
						edgeIndex = nextEdge[edgeIndex])
					{
						Edge *edge = &edgeArray[edgeIndex];
						if ((edge->vertexIndex[1] == (unsigned int)i1) &&
							(edge->faceIndex[0] == edge->faceIndex[1]))
						{
							edge->faceIndex[1] = (unsigned int) a;
							break;
						}
					}
				}

				i1 = i2;
			}

			triangle++;
		}

		delete[] firstEdge;
		return (edgeCount);
	}

	// $(CitedCodeEnd)

} // namespace TerathonEdges

void calc_edges(uint _NumVertices, const uint* _pIndices, uint _NumIndices, uint** _ppEdges, uint* _pNumberOfEdges)
{
	const uint numTriangles = _NumIndices / 3;
	if ((uint)((long)_NumVertices) != _NumVertices)
		throw std::out_of_range("_NumVertices is out of range");
	
	if ((uint)((long)numTriangles) != numTriangles)
		throw std::out_of_range("numTriangles is out of range");

	TerathonEdges::Edge* edgeArray = new TerathonEdges::Edge[3 * numTriangles];

	uint numEdges = static_cast<uint>(TerathonEdges::BuildEdges(static_cast<long>(_NumVertices), static_cast<long>(numTriangles), (const TerathonEdges::Triangle *)_pIndices, edgeArray));

	// @tony: Should the allocator be exposed?
	*_ppEdges = new uint[numEdges * 2];

	for (size_t i = 0; i < numEdges; i++)
	{
		(*_ppEdges)[i*2] = edgeArray[i].vertexIndex[0];
		(*_ppEdges)[i*2+1] = edgeArray[i].vertexIndex[1];
	}

	*_pNumberOfEdges = numEdges;

	delete [] edgeArray;
}

void free_edge_list(uint* _pEdges)
{
	delete [] _pEdges;
}

#define PUV(IndexT, UV0T, UV1T) \
void prune_unindexed_vertices(const IndexT* oRESTRICT _pIndices, uint _NumIndices \
	, float3* oRESTRICT _pPositions, float3* oRESTRICT _pNormals, float4* oRESTRICT _pTangents, UV0T* oRESTRICT _pTexcoords0, UV1T* oRESTRICT _pTexcoords1, color* oRESTRICT _pColors \
	, uint _NumVertices, uint* oRESTRICT _pNewNumVertices) \
{ detail::prune_unindexed_vertices(_pIndices, _NumIndices, _pPositions, _pNormals, _pTangents, _pTexcoords0, _pTexcoords1, _pColors, _NumVertices, _pNewNumVertices); }

PUV(uint, float2, float2)
PUV(uint, float2, float3)
PUV(uint, float3, float2)
PUV(uint, float3, float3)
PUV(ushort, float2, float2)
PUV(ushort, float2, float3)
PUV(ushort, float3, float2)
PUV(ushort, float3, float3)

	} // namespace gpu
} // namespace ouro
