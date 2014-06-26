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
#include <oMesh/primitive.h>
#include <oBase/throw.h>
#include <oHLSL/oHLSLMath.h>
#include "subdivide.h"

#include <oCompute/linear_algebra.h>

namespace ouro { namespace mesh {

static bool has(int semantic_flags, primitive::semantic_flag flag) { return (semantic_flags & flag) == flag; }

template<typename initT>
static bool has(const initT& i, primitive::semantic_flag flag) { return has(i.semantics, flag); }

template<typename initT>
static bool supports(const initT& i, uint semantics) { return !!(i.semantics & semantics); }

static float normal_sign(face_type::value type)
{
	// this lib was brought up as CCW so invert some values for CW systems

	switch (type)
	{
		case face_type::front_cw: return -1.0f;
		case face_type::front_ccw: return 1.0f;
		default: break;
	}

	return 0.0f;
}

class primitive_impl : public primitive
{
public:
	primitive_impl() {}
	~primitive_impl() { deinitialize(); }

	info get_info() const override { return inf; }
	source get_source() const override;
	void unmake() override;
	void initialize(const rectangle_init& i) override;
	void initialize(const box_init& i) override;
	void initialize(const frustum_init& i) override;
	void initialize(const circle_init& i) override;
	void initialize(const washer_init& i) override;
	void initialize(const sphere_init& i) override;
	void initialize(const cylinder_init& i) override;
	void initialize(const cone_init& i) override;
	void initialize(const torus_init& i) override;
	void initialize(const teardrop_init& i) override;
	void deinitialize() override;

private:
	info inf;
	std::vector<range> ranges;
	std::vector<uint> indices;
	std::vector<float3> positions;
	std::vector<float3> normals;
	std::vector<float4> tangents;
	std::vector<float2> texcoords;
	std::vector<color> colors;

	inline void clear()
	{
		ranges.clear();
		indices.clear();
		positions.clear();
		normals.clear();
		tangents.clear();
		texcoords.clear();
		colors.clear();
	}

	inline void shrink_to_fit()
	{
		ranges.shrink_to_fit();
		indices.shrink_to_fit();
		positions.shrink_to_fit();
		normals.shrink_to_fit();
		tangents.shrink_to_fit();
		texcoords.shrink_to_fit();
		colors.shrink_to_fit();
	}

	inline void reserve(size_t num_indices, size_t num_vertices)
	{
		indices.reserve(num_indices);
		positions.reserve(num_vertices);
		normals.reserve(num_vertices);
		tangents.reserve(num_vertices);
		texcoords.reserve(num_vertices);
		colors.reserve(num_vertices);
	}

	static void static_append_midpoint(uint i0, uint i1, void* data) { ((primitive_impl*)data)->append_midpoint(i0, i1); }
	void append_midpoint(uint i0, uint i1)
	{
		positions.push_back(::lerp(positions[i0], positions[i1], 0.5f));
		if (!normals.empty())
			normals.push_back(normalize(normals[i0] + normals[i1]));
		if (!tangents.empty())
			tangents.push_back(float4(normalize(tangents[i0].xyz() + tangents[i1].xyz()), tangents[i0].w));
		if (!texcoords.empty())
			texcoords.push_back(::lerp(texcoords[i0], texcoords[i1], 0.5f));
		if (!colors.empty())
			colors.push_back(lerp(colors[i0], colors[i1], 0.5f));
	}

	inline void subdivide(uint divide, uint num_edges)
	{
		for (uint i = 1; i < divide; i++)
			mesh::subdivide(&num_edges, indices, as_uint(positions.size()), static_append_midpoint, this);
	}

	void transform(const float4x4& m, uint base_vertex_index = 0);

	// handles common tasks
	void finalize(const face_type::value& face_type, int semantics, const color& c);
	template<typename initT> void finalize(const initT& i) { finalize(i.face_type, i.semantics, i.color); }

	// appends positions and texcoords
	void append_rectangle(const rectangle_init& i);
};

primitive* primitive::make()
{
	return new primitive_impl();
}

void primitive_impl::unmake()
{
	delete this;
}

primitive::source primitive_impl::get_source() const
{
	source s;
	s.ranges = ranges.data();
	s.indices = indices.data();
	s.positions = positions.data();
	s.normals = normals.data();
	s.tangents = tangents.data();
	s.texcoords = texcoords.data();
	s.colors = colors.data();
	return s;
}

void primitive_impl::initialize(const rectangle_init& i)
{
	deinitialize();
	oCHECK(supports(i, all_semantics), "unsupported semantics");
	append_rectangle(i);
	if (i.face_type != face_type::outline)
		subdivide(i.divide, 6);
	if (has(i, flag_normals))
		normals.assign(positions.size(), float3(0.0f, 0.0f, 1.0f) * normal_sign(i.face_type));
	if (has(i, flag_tangents))
		tangents.assign(positions.size(), float4(1.0f, 0.0f, 0.0f, 1.0f));

	finalize(i);
}

void primitive_impl::initialize(const box_init& i)
{
	deinitialize();
	oCHECK(supports(i, all_semantics), "unsupported semantics");

	const float s = normal_sign(i.face_type);
	float3 dim = i.bound.size();

	if (i.face_type == face_type::outline)
	{
		const float4x4 m = make_translation(i.bound.center());
		float3 kPositions[] =
		{
			float3(-dim.x, -dim.y, -dim.z) * 0.5f, // left bottom back
			float3(dim.x, -dim.y, -dim.z) * 0.5f, // right bottom back
			float3(-dim.x, -dim.y, dim.z) * 0.5f, // left bottom front
			float3(dim.x, -dim.y, dim.z) * 0.5f, // right bottom front
			float3(-dim.x, dim.y, -dim.z) * 0.5f, // left top back
			float3(dim.x, dim.y, -dim.z) * 0.5f, // right top back
			float3(-dim.x, dim.y, dim.z) * 0.5f, // left top front
			float3(dim.x, dim.y, dim.z) * 0.5f, // right top front
		};
		
		for (const float3& p : kPositions)
			positions.push_back(mul(m, p));

		static const uint kIndices[] = 
		{
			0,1, // bottom
			1,3,
			3,2,
			2,0,
			4,5, // top
			5,7,
			7,6,
			6,4,
			0,4, // sides
			1,5,
			2,6,
			3,7,
		};
		
		for (uint index : kIndices)
			indices.push_back(index);
	}

	else
	{
		float4 kPlanes[6] = 
		{
			float4(-1.0f, 0.0f, 0.0f, s * dim.x * 0.5f),
			float4(1.0f, 0.0f, 0.0f, s * dim.x * 0.5f),
			float4(0.0f, 1.0f, 0.0f, s * dim.y * 0.5f),
			float4(0.0f, -1.0f, 0.0f, s * dim.y * 0.5f),
			float4(0.0f, 0.0f, 1.0f, s * dim.z * 0.5f),
			float4(0.0f, 0.0f, -1.0f, s * dim.z * 0.5f),
		};

		float boxW[6] = { dim.z, dim.z, dim.x, dim.x, dim.x, dim.x, };
		float boxH[6] = { dim.y, dim.y, dim.z, dim.z, dim.y, dim.y, };

		const float4x4 m = make_translation(i.bound.center());
		for (uint face = 0; face < 6; face++)
		{
			rectangle_init r;
			r.semantics = i.semantics;
			r.face_type = i.face_type;
			r.divide = i.divide;
			r.color = i.color;
			r.width = boxW[face];
			r.height = boxH[face];
			r.centered = true;
			r.flipv = i.flipv;
			append_rectangle(r);

			float4x4 tx = make_plane(kPlanes[face]);
			transform(mul(tx, m), face * 4);
		}
	}

	if (i.face_type != face_type::outline)
		subdivide(i.divide, 36);

	finalize(i);
}

void primitive_impl::initialize(const frustum_init& i)
{
	deinitialize();
	oTHROW(not_supported, "not yet ported from oGeometry");
}

void primitive_impl::initialize(const circle_init& i)
{
	deinitialize();
	oTHROW(not_supported, "not yet ported from oGeometry");
}

void primitive_impl::initialize(const washer_init& i)
{
	deinitialize();
	oTHROW(not_supported, "not yet ported from oGeometry");
}

void primitive_impl::initialize(const sphere_init& i)
{
	deinitialize();
	oTHROW(not_supported, "not yet ported from oGeometry");
}

void primitive_impl::initialize(const cylinder_init& i)
{
	deinitialize();
	oTHROW(not_supported, "not yet ported from oGeometry");
}

void primitive_impl::initialize(const cone_init& i)
{
	deinitialize();
	oTHROW(not_supported, "not yet ported from oGeometry");
}

void primitive_impl::initialize(const torus_init& i)
{
	deinitialize();
	oTHROW(not_supported, "not yet ported from oGeometry");
}

void primitive_impl::initialize(const teardrop_init& i)
{
	deinitialize();
	oTHROW(not_supported, "not yet ported from oGeometry");
}

void primitive_impl::deinitialize()
{
	clear();
	shrink_to_fit();
	inf = info();
}

void primitive_impl::transform(const float4x4& m, uint base_vertex_index)
{
	// this should be picked up in finalize
	inf.local_space_bound.clear();
	
	uint n = as_uint(positions.size());
	for (uint i = base_vertex_index; i < n; i++)
		positions[i] = mul(m, positions[i]);
	
	float3x3 r = (float3x3)m;

	n = as_uint(normals.size());
	for (uint i = base_vertex_index; i < n; i++)
		normals[i] = normalize(mul(r, normals[i]));
	
	n = as_uint(tangents.size());
	for (uint i = base_vertex_index; i < n; i++)
	{
		const float4& t = tangents[i];
		tangents[i] = float4(normalize(mul(r, t.xyz())), t.w);
	}
}

void primitive_impl::finalize(const face_type::value& face_type, int semantics, const color& c)
{
	if (face_type != face_type::outline)
	{
		if (has(semantics, flag_normals) && normals.empty())
		{
			normals.resize(positions.size());
			calc_vertex_normals(normals.data(), indices.data(), as_uint(indices.size()), positions.data(), as_uint(positions.size()), face_type == face_type::front_ccw);
		}
		
		if (has(semantics, flag_tangents) && tangents.empty())
		{
			tangents.resize(positions.size());
			calc_vertex_tangents(tangents.data(), indices.data(), as_uint(indices.size()), positions.data(), normals.data(), texcoords.data(), as_uint(positions.size()));
		}
	}

	if (has(semantics, flag_colors))
		colors.assign(positions.size(), c);

	if (ranges.empty())
	{
		range r;
		r.start_primitive = 0;
		r.num_primitives = as_uint(indices.size() / 3);
		r.max_vertex = as_uint(positions.size());
		ranges.push_back(r);
	}

	inf.local_space_bound = calc_bound(positions.data(), sizeof(float3), as_uint(positions.size()));

	int el = 0;
	if (has(semantics, flag_positions))
		inf.elements[el] = element(semantic::position, 0, format::xyz32_float, el), el++;
	if (has(semantics, flag_normals))
		inf.elements[el] = element(semantic::normal, 0, format::xyz32_float, el), el++;
	if (has(semantics, flag_tangents))
		inf.elements[el] = element(semantic::tangent, 0, format::xyzw32_float, el), el++;
	if (has(semantics, flag_texcoords))
		inf.elements[el] = element(semantic::texcoord, 0, format::xy32_float, el), el++;
	if (has(semantics, flag_colors))
		inf.elements[el] = element(semantic::color, 0, format::xyzw8_unorm, el), el++;

	inf.num_indices = as_uint(indices.size());
	inf.num_vertices = as_uint(positions.size());
	inf.primitive_type = face_type == face_type::outline ? primitive_type::lines : primitive_type::triangles;
	inf.face_type = face_type;
	inf.num_ranges = as_uchar(ranges.size());
	inf.vertex_scale_shift = 0;
}

void primitive_impl::append_rectangle(const rectangle_init& i)
{
	static const float3 kCorners[4] = 
	{
		float3(0.0f, 0.0f, 0.0f),
		float3(1.0f, 0.0f, 0.0f),
		float3(0.0f, 1.0f, 0.0f),
		float3(1.0f, 1.0f, 0.0f),
	};

	static const uint kOutlineIndices[8] = { 0,1, 2,3, 0,2, 1,3 };
	static const uint kTriIndices[6] = { 0,2,1, 1,2,3 };

	if (!has(i, flag_positions))
		return;

	float4x4 m = make_scale(float3(i.width, i.height, 1.0f));
	if (i.centered)
		m = mul(make_translation(float3(-0.5f, -0.5f, 0.0f)), m);

	uint baseVertexIndex = as_uint(positions.size());
	uint baseIndexIndex = as_uint(indices.size());

	for (const float3& corner : kCorners)
		positions.push_back(mul(m, corner));

	if (i.face_type == face_type::outline)
	{
		for (const uint index : kOutlineIndices)
			indices.push_back(baseVertexIndex + index);
	}

	else
	{
		for (const uint index : kTriIndices)
			indices.push_back(baseVertexIndex + index);

		if (i.face_type == face_type::front_ccw)
			flip_winding_order(baseIndexIndex, indices.data(), as_uint(indices.size()));
	}

	if (has(i, flag_texcoords))
	{
		for (float3 tc : kCorners)
		{
			if (!i.flipv) // D3D standard by default
				tc.y = 1.0f - tc.y;
			texcoords.push_back(tc.xy());
		}
	}
}

}}
