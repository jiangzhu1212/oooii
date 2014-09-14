// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// Library for handling buffers of irregular networks of points, lines, indices and triangles.

#pragma once
#include <oBase/aabox.h>
#include <oBase/color.h>
#include <oBase/dec3n.h>
#include <oBase/macros.h>
#include <oBase/plane.h>
#include <oBase/types.h>
#include <oSurface/surface.h>
#include <array>

namespace ouro { namespace mesh {

static const uint max_num_slots = 8;
static const uint max_num_elements = 16;

enum class primitive_type : uchar
{
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
};

enum face_type : uchar
{
	unknown,
	front_ccw,
	front_cw,
	outline,

	count,
};

class element
{
	typedef uchar storage_t;

public:
	element() { *(uint*)this = 0; }
	element(const surface::semantic& s, uint _index, const surface::format& f, uint _slot)
	{
		semantic(s);
		index(_index);
		format(f);
		slot(_slot);
	}

	inline surface::semantic semantic() const { return (surface::semantic)semantic_; }
	inline void semantic(const surface::semantic& s)
	{
		if (s > surface::semantic::lastvertex)
			throw std::invalid_argument("invalid vertex semantic specified");
		semantic_ = (storage_t)s;
	}
	
	inline uint index() const { return index_; }
	inline void index(uint i) { index_ = (storage_t)i; }
	
	inline surface::format format() const { return (surface::format)format_; }
	inline void format(const surface::format& f) { format_ = (storage_t)f; }
	
	inline uint slot() const { return slot_; }
	inline void slot(uint s) { slot_ = (storage_t)s; }

private:
	storage_t semantic_;
	storage_t index_;
	storage_t format_;
	storage_t slot_;
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
	primitive_type primitive_type;
	face_type face_type;
	uchar num_ranges;
	uchar vertex_scale_shift; // for position as shorts for xyz it'll be (x / SHORT_MAX) * (1 << vertex_scale_shift)
};

// returns the number of bytes from a base vertex in the specified element's slot to where the element's data begins
uint calc_offset(const element_array& elements, uint element_index);

// return the size of the entire vertex for the specified slot
uint calc_vertex_size(const element_array& elements, uint slot);

uint num_primitives(const primitive_type& type, uint num_indices, uint num_vertices);
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
uint copy_element(uint dst_byte_offset, void* oRESTRICT dst, uint dst_stride, const surface::format& dst_format,
									 const void* oRESTRICT src, uint src_stride, const surface::format& src_format, uint num_vertices);

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
