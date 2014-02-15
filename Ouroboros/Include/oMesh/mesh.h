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
// Library for handling buffers of irregular networks of points, lines, indices and triangles.
#pragma once
#ifndef oMesh_h
#define oMesh_h

#include <oBase/color.h>
#include <oBase/dec3n.h>
#include <oBase/macros.h>
#include <oBase/types.h>
#include <array>

namespace ouro {
	namespace mesh {

namespace primitive_type
{ oDECLARE_SMALL_ENUM(value, uchar) {

	unknown,
	points,
	lines,
	line_strips,
	triangles,
	triangle_strips,
	lines_adjacency,
	line_strips_adjacency,
	triangles_adjacency,
	triangle_strips_adjacency,
	patches1, // # postfix is the # of control points per patch
	patches2,
	patches3,
	patches4,
	patches5,
	patches6,
	patches7,
	patches8,
	patches9,
	patches10,
	patches11,
	patches12,
	patches13,
	patches14,
	patches15,
	patches16,
	patches17,
	patches18,
	patches19,
	patches20,
	patches21,
	patches22,
	patches23,
	patches24,
	patches25,
	patches26,
	patches27,
	patches28,
	patches29,
	patches30,
	patches31,
	patches32,
	
	count,

};}

namespace face_type
{ oDECLARE_SMALL_ENUM(value, uchar) {

	unknown,
	front_ccw,
	front_cw,
	outline,

	count,

};}

namespace semantic
{ oDECLARE_SMALL_ENUM(value, uchar) {

	position, // 3-component xyz (float3/ushort4)
	normal, // 3-component xyz (float3/dec3n)
	tangent, // 4-component xyz, w is -1, 0 or 1 (float4/dec3n)
	texcoord, // 2- or 3-component uv (float2/half2 or float3/ushort4)
	color, // 4-component color rgba (uint)

	count, 

};}

namespace usage
{ oDECLARE_SMALL_ENUM(value, uchar) {

	per_mesh_static,
	per_mesh_dynamic,
	per_instance_static,
	per_instance_dynamic,

	count,

};}

namespace layout
{ oDECLARE_SMALL_ENUM(value, uchar) {

	// positions: float3
	// normals: dec3n
	// tangents: dec3n
	// texcoords0: half2 \ mutually exclusive
	// texcoords0: half4 /
	// texcoords1: half2 \ mutually exclusive
	// texcoords1: half4 /
	// colors: color

	// if this is updated, remember to update is_positions, etc.

	none,

	pos,
	color,
	pos_color,
	pos_nrm,
	pos_nrm_tan,
	pos_nrm_tan_uv0,
	pos_nrm_tan_uvwx0,
	pos_uv0,
	pos_uvwx0,
	uv0,
	uvwx0,
	uv0_color,
	uvwx0_color,

	count,

};}

class layout_array : public std::array<layout::value, usage::count>
{
public:
	typedef std::array<layout::value, usage::count> this_type;
	layout_array() { fill(layout::none); }
	layout_array(const layout::value& _Dynamic, const layout::value& _Static = layout::none, const layout::value& _PerInstance = layout::none) { (*this)[usage::per_mesh_dynamic] = _Dynamic; (*this)[usage::per_mesh_static] = _Static; (*this)[usage::per_instance_static] = _PerInstance; }
	operator this_type() { return *this; }
	operator const this_type() const { return *this; }
};

struct vertex_soa
{
	vertex_soa()
		: positionsf(nullptr)
		, normalsf(nullptr)
		, normals(nullptr)
		, tangentsf(nullptr)
		, tangents(nullptr)
		, uv0sf(nullptr)
		, uvw0sf(nullptr)
		, uvwx0sf(nullptr)
		, uv0s(nullptr)
		, uvwx0s(nullptr)
		, colors(nullptr)
		, positionf_pitch(0)
		, normalf_pitch(0)
		, normal_pitch(0)
		, tangentf_pitch(0)
		, tangent_pitch(0)
		, uv0f_pitch(0)
		, uvw0f_pitch(0)
		, uvwx0f_pitch(0)
		, uv0_pitch(0)
		, uvwx0_pitch(0)
		, color_pitch(0)
		, vertex_layout(layout::none)
	{}

	const float3* positionsf;
	const float3* normalsf;
	const dec3n* normals;
	const float4* tangentsf;
	const dec3n* tangents;
	const float2* uv0sf;
	const float3* uvw0sf;
	const float4* uvwx0sf;
	const half2* uv0s;
	const half4* uvwx0s;
	const color* colors;

	uint positionf_pitch;
	uint normalf_pitch;
	uint normal_pitch;
	uint tangentf_pitch;
	uint tangent_pitch;
	uint uv0f_pitch;
	uint uvw0f_pitch;
	uint uvwx0f_pitch;
	uint uv0_pitch;
	uint uvwx0_pitch;
	uint color_pitch;

	layout::value vertex_layout;

	inline bool operator==(const vertex_soa& _That) const
	{
		const void* const* thisP = (const void* const*)&positionsf;
		const void* const* end = (const void* const*)&colors;
		const void* const* thatP = (const void* const*)&_That.positionsf;
		while (thisP <= end) if (*thisP++ != *thatP++) return false;
		const uint* thisI = &positionf_pitch;
		const uint* endI = &color_pitch;
		const uint* thatI = &_That.positionf_pitch;
		while (thisI <= endI) if (*thisI++ != *thatI++) return false;
		return vertex_layout == _That.vertex_layout;
	}
	inline bool operator!=(const vertex_soa& _That) const { return !(*this == _That); }
};

template<typename T>
class bound
{
	// hybrid aabox/sphere class
	// the aabox is the minimal size to store all points and the sphere contains that box
public:
	typedef T element_type;
	typedef TVEC3<element_type> vec3_type;
	typedef TVEC4<element_type> vec4_type;

	bound() { clear(); }
	bound(const TVEC3<T>& _Extents) { extents(_Extents); }
	bound(const TVEC3<T>& _Min, const TVEC3<T>& _Max) { extents(_Min, _Max); }

	bool empty() const { return any(Extents < T(0)); }
	void clear() { Sphere = TVEC4<T>(T(0)); extents(T(-1)); }

	TVEC3<T> center() const { return Sphere.xyz(); }
	void center(const TVEC3<T>& _Center) const { return Sphere = TVEC4<T>(_Position, sphere.w); }

	T radius() const { return sphere.w; }
	void radius(const T& _Radius) { Sphere.w = _Radius; }

	TVEC4<T> sphere() const { return Sphere; }

	TVEC3<T> extents() const { return Extents; }
	void extents(const TVEC3<T>& _Extents) { Extents = _Extents; Sphere.w = length(get_max() - get_min()) / T(2); }
	void extents(const TVEC3<T>& _Min, const TVEC3<T>& _Max) { Sphere.xyz() = (_Max - _Min) / T(2); extents(_Max - Sphere.xyz()); }

	TVEC3<T> size() const { return Extents * 2.0f; }

	TVEC3<T> get_min() const { return center() - Extents; }
	TVEC3<T> get_max() const { return center() + Extents; }

private:
	TVEC4<T> Sphere;
	TVEC3<T> Extents;
};

typedef bound<float> boundf; typedef bound<double> boundd;

struct range
{
	range(uint _StartPrimitive = 0, uint _NumPrimitives = 0, uint _MinVertex = 0, uint _MaxVertex = invalid, uint _Material = 0)
		: start_primitive(_StartPrimitive)
		, num_primitives(_NumPrimitives)
		, min_vertex(_MinVertex)
		, max_vertex(_MaxVertex)
		, material(_Material)
	{}

	uint start_primitive; // index buffer offset in # of primitives
	uint num_primitives; // Number of primitives in range
	uint min_vertex; // min index into vertex buffer that will be accessed
	uint max_vertex; // max index into vertex buffer that will be accessed
	uint material;
};

struct info
{
	info()
		: num_indices(0)
		, num_vertices(0)
		, primitive_type(primitive_type::unknown)
		, face_type(face_type::unknown)
		, num_vertex_ranges(0)
		, vertex_scale_shift(0)
		, pad0(0)
	{ vertex_layouts.fill(layout::none); }

	boundf local_space_bound;
	uint num_indices;
	uint num_vertices;
	layout_array vertex_layouts;
	primitive_type::value primitive_type;
	face_type::value face_type;
	uchar num_vertex_ranges;
	uchar vertex_scale_shift; // for position as shorts for xyz it'll be (x / SHORT_MAX) * (1 << vertex_scale_shift)
	uchar pad0;
};

inline bool has_16bit_indices(uint _NumVertices) { return _NumVertices <= 65535; }
inline uint index_size(uint _NumVertices) { return has_16bit_indices(_NumVertices) ? sizeof(ushort) : sizeof(uint); }
uint num_primitives(const primitive_type::value& _PrimitiveType, uint _NumIndices, uint _NumVertices);
uint vertex_size(const layout::value& _Layout);

inline bool has_positions(const layout::value& _Layout) { return _Layout >= layout::pos && _Layout <= layout::pos_uvwx0; }
inline bool has_normals(const layout::value& _Layout) { return _Layout >= layout::pos_nrm && _Layout <= layout::pos_nrm_tan_uvwx0; }
inline bool has_tangents(const layout::value& _Layout) { return _Layout >= layout::pos_nrm_tan && _Layout <= layout::pos_nrm_tan_uvwx0; }
inline bool has_texcoords(const layout::value& _Layout) { return _Layout >= layout::pos_nrm_tan_uv0 && _Layout <= layout::uvwx0_color; }
inline bool has_uv0s(const layout::value& _Layout) { return ((_Layout&0x1)==0) && has_texcoords(_Layout); }
inline bool has_uvwx0s(const layout::value& _Layout) { return (_Layout&0x1) && has_texcoords(_Layout); }
inline bool has_colors(const layout::value& _Layout) { return _Layout == layout::pos_color || _Layout == layout::color || _Layout == layout::uv0_color || _Layout == layout::uvwx0_color; }

void flip_winding_order(uint _BaseIndexIndex, ushort* _pIndices, uint _NumIndices);
void flip_winding_order(uint _BaseIndexIndex, uint* _pIndices, uint _NumIndices);

// copies index buffers from one to another, properly converting from 16-bit to 32-bit and vice versa.
void copy_indices(void* oRESTRICT _pDestination, uint _DestinationPitch, const void* oRESTRICT _pSource, uint _SourcePitch, uint _NumIndices);
void copy_indices(ushort* oRESTRICT _pDestination, const uint* oRESTRICT _pSource, uint _NumIndices);
void copy_indices(uint* oRESTRICT _pDestination, const ushort* oRESTRICT _pSource, uint _NumIndices);

// uses the above utility functions to do all necessary conversions to copy the source to the destination
void copy_vertices(void* oRESTRICT _pDestination, const layout::value& _DestinationLayout, const vertex_soa& _Source, uint _NumVertices);

// Calculates the min and max index as stored in _pIndices. This starts iterating at _StartIndex through
// _NumIndices as if from 0. _NumVertices is the number to traverse (end() will be _StartIndex + _NumIndices).
//_pMinVertex and _pMaxVertex receive the lowest and highest index values respectively.
void calc_min_max_indices(const uint* oRESTRICT _pIndices, uint _StartIndex, uint _NumIndices, uint _NumVertices, uint* oRESTRICT _pMinVertex, uint* oRESTRICT _pMaxVertex);
void calc_min_max_indices(const ushort* oRESTRICT _pIndices, uint _StartIndex, uint _NumIndices, uint _NumVertices, uint* oRESTRICT _pMinVertex, uint* oRESTRICT _pMaxVertex);

boundf calc_bound(const float3* _pVertices, uint _VertexStride, uint _NumVertices);

void transform_points(const float4x4& _Matrix, float3* oRESTRICT _pDestination, uint _DestinationStride, const float3* oRESTRICT _pSource, uint _SourceStride, uint _NumPoints);
void transform_vectors(const float4x4& _Matrix, float3* oRESTRICT _pDestination, uint _DestinationStride, const float3* oRESTRICT _pSource, uint _SourceStride, uint _NumVectors);

// Removes indices for degenerate triangles. After calling this function use prune_unindexed_vertices() to clean up extra vertices.
// _pPositions: list of XYZ positions indexed by the index array
// _NumVertices: The number of vertices in the _pPositions array
// _pIndices: array of triangles (every 3 specifies a triangle)
// _NumIndices: The number of indices in the _pIndices array
// _pNewNumIndices: The new number of indices as a result of removed degenerages
void remove_degenerates(const float3* oRESTRICT _pPositions, uint _NumPositions, uint* oRESTRICT _pIndices, uint _NumIndices, uint* oRESTRICT _pNewNumIndices);
void remove_degenerates(const float3* oRESTRICT _pPositions, uint _NumPositions, ushort* oRESTRICT _pIndices, uint _NumIndices, uint* oRESTRICT _pNewNumIndices);

// Calculate the face normals from the following inputs:
// _pFaceNormals: output, array to fill with normals. This should be at least as
//                large as the number of faces in the specified mesh (_NumberOfIndices/3)
// _pIndices: array of triangles (every 3 specifies a triangle)
// _NumIndices: The number of indices in the _pIndices array
// _pPositions: list of XYZ positions for the mesh that are indexed by the index array
// _NumPositions: The number of vertices in the _pPositions array
// _CCW: If true triangles are assumed to have their front-face be specified by the counter-
//       clockwise order of vertices in a triangle. This affects which way a normal points.
void calc_face_normals(float3* oRESTRICT _pFaceNormals, const uint* oRESTRICT _pIndices, uint _NumIndices, const float3* oRESTRICT _pPositions, uint _NumPositions, bool _CCW = false);
void calc_face_normals(float3* oRESTRICT _pFaceNormals, const ushort* oRESTRICT _pIndices, uint _NumIndices, const float3* oRESTRICT _pPositions, uint _NumPositions, bool _CCW = false);

// Calculates the vertex normals by averaging face normals from the following 
// inputs:
// _pVertexNormals: output, array to fill with normals. This should be at least 
//                  as large as the number of vertices in the specified mesh
//                  (_NumberOfVertices).
// _pIndices: array of triangles (every 3 specifies a triangle)
// _NumberOfIndices: The number of indices in the _pIndices array
// _pPositions: list of XYZ positions for the mesh that are indexed by the index 
//              array
// _NumberOfPositions: The number of vertices in the _pPositions array
// _CCW: If true, triangles are assumed to have their front-face be specified by
//       the counter-clockwise order of vertices in a triangle. This affects 
//       which way a normal points.
// _OverwriteAll: Overwrites any pre-existing data in the array. If this is 
// false, any zero-length vector will be overwritten. Any length-having vector
// will not be touched.
// This can return EINVAL if a parameters isn't something that can be used.
void calc_vertex_normals(float3* _pVertexNormals, const uint* _pIndices, uint _NumIndices, const float3* _pPositions, uint _NumPositions, bool _CCW = false, bool _OverwriteAll = true);
void calc_vertex_normals(float3* _pVertexNormals, const ushort* _pIndices, uint _NumIndices, const float3* _pPositions, uint _NumPositions, bool _CCW = false, bool _OverwriteAll = true);

// Calculates the tangent space vector and its handedness from the following
// inputs:
// _pTangents: output, array to fill with tangents. This should be at least as large as the number of vertices 
//             in the specified mesh (_NumVertices)
// _pIndices: array of triangles (every 3 specifies a triangle)
// _NumIndices: The number of indices in the _pIndices array
// _pPositions: list of XYZ positions for the mesh that are indexed by the index array
// _pNormals: list of normalized normals for the mesh that are indexed by the index array
// _pTexcoords: list of texture coordinates for the mesh that are indexed by the index array
// _NumVertices: The number of vertices in the _pPositions, _pNormals and _pTexCoords arrays
void calc_vertex_tangents(float4* _pTangents, const uint* _pIndices, uint _NumIndices, const float3* _pPositions, const float3* _pNormals, const float3* _pTexcoords, uint _NumVertices);
void calc_vertex_tangents(float4* _pTangents, const ushort* _pIndices, uint _NumIndices, const float3* _pPositions, const float3* _pNormals, const float3* _pTexcoords, uint _NumVertices);

// Fills _pOutTexcoords with texture coordinates calculated using LCSM. The 
// pointer should be allocated to have at least _NumVertices elements. If 
// _pSolveTime is specified the number of seconds to calculate texcoords will 
// be returned.

// NOTE: No LCSM code or middleware has been integrated, so these will only throw
// operation_not_supported right now.
void calc_texcoords(const boundf& _Bound, const uint* _pIndices, uint _NumIndices, const float3* _pPositions, float2* _pOutTexcoords, uint _NumVertices, double* _pSolveTime);
void calc_texcoords(const boundf& _Bound, const uint* _pIndices, uint _NumIndices, const float3* _pPositions, float3* _pOutTexcoords, uint _NumVertices, double* _pSolveTime);
void calc_texcoords(const boundf& _Bound, const ushort* _pIndices, uint _NumIndices, const float3* _pPositions, float2* _pOutTexcoords, uint _NumVertices, double* _pSolveTime);
void calc_texcoords(const boundf& _Bound, const ushort* _pIndices, uint _NumIndices, const float3* _pPositions, float3* _pOutTexcoords, uint _NumVertices, double* _pSolveTime);

// Allocates and fills an edge list for the mesh described by the specified indices:
// _NumVertices: The number of vertices the index array indexes
// _pIndices: array of triangles (every 3 specifies a triangle)
// _NumIndices: The number of indices in the _pIndices array
// _ppEdges: a pointer to receive an allocation and be filled with index pairs 
//           describing an edge. Use oFreeEdgeList() to free memory the edge 
//           list allocation. So every two uints in *_ppEdges represents an edge.
// _pNumberOfEdges: a pointer to receive the number of edge pairs returned
void calc_edges(uint _NumVertices, const uint* _pIndices, uint _NumIndices, uint** _ppEdges, uint* _pNumberOfEdges);

// Frees the buffer allocated by calc_edges
void free_edge_list(uint* _pEdges);

#define PUV_PARAMS(IndexT, UV0T, UV1T) const IndexT* oRESTRICT _pIndices, uint _NumIndices \
	, float3* oRESTRICT _pPositions, float3* oRESTRICT _pNormals, float4* oRESTRICT _pTangents \
	, UV0T* oRESTRICT _pTexcoords0, UV1T* oRESTRICT _pTexcoords1, color* oRESTRICT _pColors \
	, uint _NumVertices, uint* oRESTRICT _pNewNumVertices

// Given the parameters as described in the above macro, contract the vertex element arrays
// to remove any vertex not indexed by the specified indices. Call this after remove_degenerates.
void prune_unindexed_vertices(PUV_PARAMS(uint, float2, float2));
void prune_unindexed_vertices(PUV_PARAMS(uint, float2, float3));
void prune_unindexed_vertices(PUV_PARAMS(uint, float3, float2));
void prune_unindexed_vertices(PUV_PARAMS(uint, float3, float3));
void prune_unindexed_vertices(PUV_PARAMS(ushort, float2, float2));
void prune_unindexed_vertices(PUV_PARAMS(ushort, float2, float3));
void prune_unindexed_vertices(PUV_PARAMS(ushort, float3, float2));
void prune_unindexed_vertices(PUV_PARAMS(ushort, float3, float3));

	} // namespace mesh
} // namespace ouro

#endif
