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

#include <oBase/aabox.h>
#include <oBase/color.h>
#include <oBase/dec3n.h>
#include <oBase/macros.h>
#include <oBase/plane.h>
#include <oBase/types.h>
#include <array>

namespace ouro { namespace mesh {

static const uint max_num_slots = 8;
static const uint max_num_elements = 16;

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

	unknown,
	position,
	normal,
	tangent,
	texcoord,
	color,

	count, 

};}

namespace format
{ oDECLARE_SMALL_ENUM(value, uchar) {

	unknown,
	xy32_float,
	xyz32_float,
	xyzw32_float,
	xy16_float,
	xy16_unorm,
	xy16_snorm,
	xy16_uint,
	xy16_sint,
	xyzw16_float,
	xyzw16_unorm,
	xyzw16_snorm,
	xyzw16_uint,
	xyzw16_sint,
	xyz10w2_unorm,
	xyz10w2_uint,
	xyzw8_unorm,
	xyzw8_snorm,
	xyzw8_uint,
	xyzw8_sint,

	count,

};}

class element
{
public:
	element() { *(ushort*)this = 0; }
	element(const semantic::value& _semantic, uint _index, const format::value& _format, uint _slot)
		: ussemantic(_semantic)
		, usindex((ushort)_index)
		, usformat(_format)
		, usslot((ushort)_slot)
	{}

	inline semantic::value semantic() const { return (semantic::value)ussemantic; }
	inline void semantic(const semantic::value& s) { ussemantic = s; }
	
	inline uint index() const { return usindex; }
	inline void index(uint i) { usindex = (ushort)i; }
	
	inline format::value format() const { return (format::value)usformat; }
	inline void format(const format::value& f) { usformat = f; }
	
	inline uint slot() const { return usslot; }
	inline void slot(uint s) { usslot = (ushort)s; }

private:
	ushort ussemantic : 4;
	ushort usindex : 3;
	ushort usformat : 5;
	ushort usslot : 4;
};

typedef std::array<element, max_num_elements> element_array;

struct range
{
	range(uint _start_prim = 0, uint _num_prims = 0, uint _min_vertex = 0, uint _max_vertex = ~0u)
		: start_primitive(_start_prim)
		, num_primitives(_num_prims)
		, min_vertex(_min_vertex)
		, max_vertex(_max_vertex)
	{}

	uint start_primitive; // index buffer offset in # of primitives
	uint num_primitives; // Number of primitives in range
	uint min_vertex; // min index into vertex buffer that will be accessed
	uint max_vertex; // max index into vertex buffer that will be accessed
};

struct info
{
	info()
		: num_indices(0)
		, num_vertices(0)
		, primitive_type(primitive_type::unknown)
		, face_type(face_type::unknown)
		, num_ranges(0)
		, vertex_scale_shift(0)
	{ elements.fill(element()); }

	aaboxf local_space_bound;
	element_array elements;
	uint num_indices;
	uint num_vertices;
	primitive_type::value primitive_type;
	face_type::value face_type;
	uchar num_ranges;
	uchar vertex_scale_shift; // for position as shorts for xyz it'll be (x / SHORT_MAX) * (1 << vertex_scale_shift)
};

// returns the number of bytes to store one item of the specified format
uint format_size(const format::value& f);

// returns the number of bytes from a base vertex in the specified element's slot to where the element's data begins
uint calc_offset(const element_array& elements, uint element_index);

// return the size of the entire vertex for the specified slot
uint calc_vertex_size(const element_array& elements, uint slot);

uint num_primitives(const primitive_type::value& type, uint num_indices, uint num_vertices);
inline uint num_primitives(const info& i) { return num_primitives(i.primitive_type, i.num_indices, i.num_vertices); }

inline bool has_16bit_indices(uint num_vertices) { return num_vertices <= 65535; }
inline uint index_size(uint num_vertices) { return has_16bit_indices(num_vertices) ? sizeof(ushort) : sizeof(uint); }

// swaps indices to have a triangles in the list face the other way
void flip_winding_order(uint base_index_index, ushort* indices, uint num_indices);
void flip_winding_order(uint base_index_index, uint* indices, uint num_indices);

// copies index buffers from one to another, properly converting from 16-bit to 32-bit and vice versa.
void copy_indices(void* oRESTRICT dst, uint dst_pitch, const void* oRESTRICT src, uint src_pitch, uint num_indices);
void copy_indices(ushort* oRESTRICT dst, const uint* oRESTRICT src, uint num_indices);
void copy_indices(uint* oRESTRICT dst, const ushort* oRESTRICT src, uint num_indices);

// Adds offset to each index
void offset_indices(ushort* oRESTRICT dst, uint num_indices, int offset);
void offset_indices(uint* oRESTRICT dst, uint num_indices, int offset);

// Copies from the src to the dst + dst_byte_offset and does any appropriate format conversion.
// If no copy can be done the values are set to zero. This returns dst_byte_offset + size of dst_type.
uint copy_element(uint dst_byte_offset, void* oRESTRICT dst, uint dst_stride, const format::value& dst_format,
									 const void* oRESTRICT src, uint src_stride, const format::value& src_format, uint num_vertices);

// Uses copy_element to find a src for each dst and copies it in (or sets to zero).
void copy_vertices(void* oRESTRICT* oRESTRICT dst, const element_array& dst_elements, const void* oRESTRICT* oRESTRICT src, const element_array& src_elements, uint num_vertices);

// Calculates the min and max index as stored in indices. This starts iterating at start_index through
// num_indices as if from 0. num_vertices is the number to traverse (end() will be start_index + num_indices).
//out_min_vertex and out_max_vertex receive the lowest and highest index values respectively.
void calc_min_max_indices(const uint* oRESTRICT indices, uint start_index, uint num_indices, uint num_vertices, uint* oRESTRICT out_min_vertex, uint* oRESTRICT out_max_vertex);
void calc_min_max_indices(const ushort* oRESTRICT indices, uint start_index, uint num_indices, uint num_vertices, uint* oRESTRICT out_min_vertex, uint* oRESTRICT out_max_vertex);

aaboxf calc_bound(const float3* vertices, uint vertex_stride, uint num_vertices);

void transform_points(const float4x4& matrix, float3* oRESTRICT dst, uint dst_stride, const float3* oRESTRICT src, uint source_stride, uint num_points);
void transform_vectors(const float4x4& matrix, float3* oRESTRICT dst, uint dst_stride, const float3* oRESTRICT src, uint source_stride, uint num_vectors);

// Removes indices for degenerate triangles. After calling this function use prune_unindexed_vertices() to clean up extra vertices.
// positions: list of XYZ positions indexed by the index array
// num_vertices: The number of vertices in the positions array
// indices: array of triangles (every 3 specifies a triangle)
// num_indices: The number of indices in the indices array
// out_new_num_indices: The new number of indices as a result of removed degenerages
void remove_degenerates(const float3* oRESTRICT positions, uint num_positions, uint* oRESTRICT indices, uint num_indices, uint* oRESTRICT out_new_num_indices);
void remove_degenerates(const float3* oRESTRICT positions, uint num_positions, ushort* oRESTRICT indices, uint num_indices, uint* oRESTRICT out_new_num_indices);

// Calculate the face normals from the following inputs:
// face_normals: output, array to fill with normals. This should be at least as
//                large as the number of faces in the specified mesh (_NumberOfIndices/3)
// indices: array of triangles (every 3 specifies a triangle)
// num_indices: The number of indices in the indices array
// positions: list of XYZ positions for the mesh that are indexed by the index array
// num_positions: The number of vertices in the positions array
// ccw: If true triangles are assumed to have their front-face be specified by the counter-
//       clockwise order of vertices in a triangle. This affects which way a normal points.
void calc_face_normals(float3* oRESTRICT face_normals, const uint* oRESTRICT indices, uint num_indices, const float3* oRESTRICT positions, uint num_positions, bool ccw = false);
void calc_face_normals(float3* oRESTRICT face_normals, const ushort* oRESTRICT indices, uint num_indices, const float3* oRESTRICT positions, uint num_positions, bool ccw = false);

// Calculates the vertex normals by averaging face normals from the following 
// inputs:
// vertex_normals: output, array to fill with normals. This should be at least 
//                 as large as the number of vertices in the specified mesh
//                 (_NumberOfVertices).
// indices: array of triangles (every 3 specifies a triangle)
// num_indices: The number of indices in the indices array
// positions: list of XYZ positions for the mesh that are indexed by the index 
//            array
// num_positions: The number of vertices in the positions array
// ccw: If true, triangles are assumed to have their front-face be specified by
//       the counter-clockwise order of vertices in a triangle. This affects 
//       which way a normal points.
// overwrite_all: Overwrites any pre-existing data in the array. If this is 
// false, any zero-length vector will be overwritten. Any length-having vector
// will not be touched.
// This can return EINVAL if a parameters isn't something that can be used.
void calc_vertex_normals(float3* vertex_normals, const uint* indices, uint num_indices, const float3* positions, uint num_positions, bool ccw = false, bool overwrite_all = true);
void calc_vertex_normals(float3* vertex_normals, const ushort* indices, uint num_indices, const float3* positions, uint num_positions, bool ccw = false, bool overwrite_all = true);

// Calculates the tangent space vector and its handedness from the following
// inputs:
// tangents: output, array to fill with tangents. This should be at least as large as the number of vertices 
//           in the specified mesh (num_vertices)
// indices: array of triangles (every 3 specifies a triangle)
// num_indices: The number of indices in the indices array
// positions: list of XYZ positions for the mesh that are indexed by the index array
// normals: list of normalized normals for the mesh that are indexed by the index array
// texcoords: list of texture coordinates for the mesh that are indexed by the index array
// num_vertices: The number of vertices in the positions, normals and texcoords arrays
void calc_vertex_tangents(float4* tangents, const uint* indices, uint num_indices, const float3* positions, const float3* normals, const float3* texcoords, uint num_vertices);
void calc_vertex_tangents(float4* tangents, const uint* indices, uint num_indices, const float3* positions, const float3* normals, const float2* texcoords, uint num_vertices);
void calc_vertex_tangents(float4* tangents, const ushort* indices, uint num_indices, const float3* positions, const float3* normals, const float3* texcoords, uint num_vertices);
void calc_vertex_tangents(float4* tangents, const ushort* indices, uint num_indices, const float3* positions, const float3* normals, const float2* texcoords, uint num_vertices);

// Fills out_texcoords with texture coordinates calculated using LCSM. The 
// pointer should be allocated to have at least num_vertices elements. If 
// out_solve_time is specified the number of seconds to calculate texcoords will 
// be returned.

// NOTE: No LCSM code or middleware has been integrated, so these will only throw
// operation_not_supported right now.
void calc_texcoords(const aaboxf& bound, const uint* indices, uint num_indices, const float3* positions, float2* out_texcoords, uint num_vertices, double* out_solve_time);
void calc_texcoords(const aaboxf& bound, const uint* indices, uint num_indices, const float3* positions, float3* out_texcoords, uint num_vertices, double* out_solve_time);
void calc_texcoords(const aaboxf& bound, const ushort* indices, uint num_indices, const float3* positions, float2* out_texcoords, uint num_vertices, double* out_solve_time);
void calc_texcoords(const aaboxf& bound, const ushort* indices, uint num_indices, const float3* positions, float3* out_texcoords, uint num_vertices, double* out_solve_time);

// Allocates and fills an edge list for the mesh described by the specified indices:
// num_vertices: The number of vertices the index array indexes
// indices: array of triangles (every 3 specifies a triangle)
// num_indices: The number of indices in the indices array
// _ppEdges: a pointer to receive an allocation and be filled with index pairs 
//           describing an edge. Use oFreeEdgeList() to free memory the edge 
//           list allocation. So every two uints in *_ppEdges represents an edge.
// out_num_edges: a pointer to receive the number of edge pairs returned
void calc_edges(uint num_vertices, const uint* indices, uint num_indices, uint** _ppEdges, uint* out_num_edges);

// Frees the buffer allocated by calc_edges
void free_edge_list(uint* edges);

#define PUV_PARAMS(IndexT, UV0T, UV1T) const IndexT* oRESTRICT indices, uint num_indices \
	, float3* oRESTRICT positions, float3* oRESTRICT normals, float4* oRESTRICT tangents \
	, UV0T* oRESTRICT texcoords0, UV1T* oRESTRICT texcoords1, color* oRESTRICT colors \
	, uint num_vertices, uint* oRESTRICT out_new_num_vertices

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

inline float3 lerp_positions(const float3& a, const float3& b, const float s) { return lerp(a, b, s); }
inline float3 lerp_normals(const float3& a, const float3& b, const float s) { return normalize(lerp(a, b, s)); }
inline float4 lerp_tangents(const float4& a, const float4& b, const float s) { return float4(normalize(lerp(a.xyz(), b.xyz(), s)), a.w); }
inline float2 lerp_texcoords(const float2& a, const float2& b, const float s) { return lerp(a, b, s); }
inline float3 lerp_texcoords(const float3& a, const float3& b, const float s) { return lerp(a, b, s); }
inline color lerp_colors(const color& a, const color& b, const float s) { return lerp(a, b, s); }

// clips the convex polygon to the plane (keeps verts on positive side of plane) by copying the 
// result to out_clipped_vertices which must be able to accomodate num_vertices+1 output vertices. 
// Vertex winding order will be maintained. If no clipping was done at all out_clipped will receive 
// false. This will rotate the order of vertices by 1. Meaning 012 if unclipped will come out 201
uint clip_convex(const planef& plane, const float3* oRESTRICT polygon, uint num_vertices, float3* oRESTRICT out_clipped_vertices);

}}

#endif
