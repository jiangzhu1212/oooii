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
#include "platonic_solids.h"
#include "subdivide.h"

#include <oCompute/linear_algebra.h>

namespace ouro { namespace mesh {

static const float kVerySmall = 0.000001f;

static bool has(int semantic_flags, primitive::semantic_flag flag) { return (semantic_flags & flag) == flag; }

template<typename initT>
static bool has(const initT& i, primitive::semantic_flag flag) { return has(i.semantics, flag); }

template<typename initT>
static bool supports(const initT& i, uint semantics) { return !!(i.semantics & semantics); }

static float normal_sign(const face_type::value& type)
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

static face_type::value opposite_face_type(const face_type::value& type)
{
	switch (type)
	{
		case face_type::front_cw: return face_type::front_ccw;
		case face_type::front_ccw: return face_type::front_cw;
		default: break;
	}

	return type;
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

	uint num_indices() const { return as_uint(indices.size()); }
	uint num_vertices() const { return as_uint(positions.size()); }

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
	
	inline void resize(size_t num_indices, size_t num_vertices)
	{
		indices.resize(num_indices);
		positions.resize(num_vertices);
		normals.resize(num_vertices);
		tangents.resize(num_vertices);
		texcoords.resize(num_vertices);
		colors.resize(num_vertices);
	}

	// appends a vertex with all the same values as the one at the specified inded
	void append_duplicate(uint i)
	{
		#define DUP(s) if (!s.empty()) s.push_back(s[i]);
			DUP(positions);
			DUP(normals);
			DUP(tangents);
			DUP(texcoords);
			DUP(colors);
		#undef LERP
	}

	// appends a vertex with a value interpolated along the values indicated by
	// i0 and i1.
	void append_interpolant(uint i0, uint i1, float t)
	{
		#define LERP(s) if (!s.empty()) s.push_back(lerp_##s(s[i0], s[i1], t));
			LERP(positions);
			LERP(normals);
			LERP(tangents);
			LERP(texcoords);
			LERP(colors);
		#undef LERP
	}

	static void append_midpoint(uint i0, uint i1, void* data) { ((primitive_impl*)data)->append_interpolant(i0, i1, 0.5f); }

	inline void subdivide(uint divide, uint num_edges)
	{
		for (uint i = 1; i < divide; i++)
			mesh::subdivide(&num_edges, indices, num_vertices(), append_midpoint, this);
	}

	void transform(const float4x4& m, uint base_vertex_index = 0);

	// removes unindexed vertices
	void prune();

	// removes all triangles wholly on the negative side of the plane
	void cull(const planef& plane);

	// flips winding order from the specified triangle index to the 
	// end of the current index array.
	void flip_winding_order_to_end(uint base_index_index) { flip_winding_order(base_index_index, indices.data(), num_indices()); }

	// handles common tasks like assigning color, calculating derived values like normals, tangents, etc.
	void finalize(const face_type::value& face_type, int semantics, const color& c);
	template<typename initT> void finalize(const initT& i) { finalize(i.face_type, i.semantics, i.color); }

	// appends positions and texcoords
	void append_rectangle(const rectangle_init& i);

	void append_frustum_positions(const oFrustumf& f);

	void append_frustum_indices();

	void append_frustum_outline_indices();

	// appends the indices for a circle. Tessellation is zig-zagged, not a tri-fan
	void append_circle_indices(uint base_index_index, uint base_vertex_index, uint facet, const face_type::value& face_type);

	// appends the indices for a washer (2D torus). Tessellation is zig-zagged, not a tri-fan
	void append_washer_indices(uint base_index_index, uint base_vertex_index, uint facet, const face_type::value& face_type);

	// appends the positions for a circle to inout_positions, not to this
	void append_circle_positions(std::vector<float3>& inout_positions, float radius, uint facet, float z_value);

	// appends texcoords for vertices laid out with append_circle_positions
	void append_circle_texcoords(std::vector<float2>& inout_texcoords, uint facet, float radius);

	// appends Z up normals to match positions count
	void append_circle_normals(uint base_vertex_index, const face_type::value& face_type);

	// uses the above circle appends for a full circle
	void append_circle(const circle_init& i, uint base_index_index, uint base_vertex_index, bool clear_primitive, float z_value);
	inline void append_circle(const circle_init& i, bool clear_primitive, float z_value) { append_circle(i, num_indices(), num_vertices(), clear_primitive, z_value); }

	// uses the above circle appends for a full washer
	void append_washer(const washer_init& i, uint base_index_index, uint base_vertex_index, bool clear_primitive, float z_value);

	// appends the indices for a cylinder that indexes two circle vertices as created by append_circle_positions
	void append_cylinder_indices(uint facet, uint base_vertex_index, const face_type::value& face_type);

	// assumes wrap sample mode, this finds triangles that straddle the seam where texcoords close to 1.0 interpolate back to 0.0
	// and thus have a backwards-mapped stripe/seam.
	void fix_cylindrical_texcoord_seams(float texcoord_threshold);
	
	void fix_sphere_poles_texcoord_seams();

	void set_sphere_texcoords(bool hemisphere);
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

		static const uint kindices[] = 
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
		
		for (uint index : kindices)
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
	oCHECK(supports(i, flag_positions|flag_normals|flag_colors), "texcoords and tangents not supported");

	append_frustum_positions( i.bound);

	if (i.face_type != face_type::outline)
		append_frustum_indices();
	else
		append_frustum_outline_indices();

	if (i.face_type == face_type::front_ccw)
		flip_winding_order_to_end(0);

	if (i.face_type != face_type::outline)
		subdivide(i.divide, 36);

	finalize(i);
}

void primitive_impl::initialize(const circle_init& i)
{
	deinitialize();
	oCHECK(supports(i, all_semantics), "unsupported semantics");
	append_circle(i, 0, 0, false, 0.0f);
	finalize(i);
}

void primitive_impl::initialize(const washer_init& i)
{
	deinitialize();
	oCHECK(supports(i, flag_positions|flag_normals|flag_colors), "texcoords and tangents not supported");
	append_washer(i, 0, 0, false, 0.0f);
	finalize(i);
}

void primitive_impl::initialize(const sphere_init& i)
{
	deinitialize();
	oCHECK(supports(i, all_semantics), "unsupported semantics");
	oCHECK_ARG(i.divide > 0, "invalid divide");

	platonic_info pi = get_platonic_info(i.icosahedron ? platonic::icosahedron : platonic::octahedron);

	positions.assign(pi.positions, pi.positions + pi.num_vertices);
	indices.assign(pi.indices, pi.indices + 3*pi.num_faces);

	if (i.face_type == face_type::front_ccw)
		flip_winding_order_to_end(0);

	subdivide(i.divide, pi.num_edges);

	if (i.hemisphere)
		cull(planef(float3(0.0f, 0.0f, 1.0f), 0.0f));

	if (i.face_type == face_type::outline)
	{
		uint* pEdges = 0;
		uint nEdges = 0;

		calc_edges(num_vertices(), indices.data(), num_indices(), &pEdges, &nEdges);

		indices.clear();
		indices.reserve(nEdges * 2);
		for (uint i = 0; i < nEdges; i++)
		{
			indices.push_back(pEdges[i*2]);
			indices.push_back(pEdges[i*2+1]);
		}

		free_edge_list(pEdges);
	}

	else
	{
		if (has(i.semantics, flag_texcoords))
		{
			// set basic texcoords that will have singularities at the poles as well as
			// a stripe of backwards mapping where texcoords.u close to 1 roll back over 
			// to 0.
			set_sphere_texcoords(i.hemisphere);

			// fix the stripe
			fix_cylindrical_texcoord_seams(0.85f);

			// and the poles
			fix_sphere_poles_texcoord_seams();
		}

		if (has(i.semantics, flag_normals))
		{
			normals.resize(positions.size());
			for (size_t i = 0; i < normals.size(); i++)
				normals[i] = normalize(positions[i]);
		}
	}

	finalize(i);
}

void primitive_impl::initialize(const cylinder_init& i)
{
	deinitialize();

	// Tangents would be supported if texcoords were supported
	// Texcoord support is a bit hard b/c it has the same wrap-around problem spheres have.
	// This means we need to duplicate vertices along the seams and assign a 
	// different texcoord. It's a bit wacky because the circle doesn't align on 0,
	// so really the next steps are:
	// 1. Get Circle to have a vertex on (0,1,0)
	// 2. Be able to duplicate that vert and reindex triangles on the (0.0001,0.9999,0) 
	//    side
	// 3. Also texture a circle.
	oCHECK(supports(i, flag_positions|flag_normals|flag_colors), "texcoords and tangents not supported");
	oCHECK_ARG(i.facet >= 3, "facet must be >= 3");
	oCHECK_ARG(i.divide > 0, "invalid divide");

	const float fStep = i.height / static_cast<float>(i.divide);
	const uint divide = i.divide;

	if (i.face_type == face_type::outline)
	{
		circle_init c;
		c.semantics = i.semantics;
		c.face_type = i.face_type;
		c.facet = i.facet;
		c.color = i.color;

		for (uint ii = 0; ii <= divide; ii++)
		{
			c.radius = ::lerp(i.radius0, i.radius1, ii / static_cast<float>(divide));
			append_circle(c, false, ii * fStep);
		}

		// Now add lines along i.facet points

		const uint nCircleVerts = as_uint(positions.size() / (divide + 1));
		const uint nCircleVertsDivide = nCircleVerts * divide;
		indices.push_back(0);
		indices.push_back(0 + nCircleVertsDivide);
		uint ii = 2 * i.outline_vertical_skip + 1;
		for (; ii < nCircleVerts - 1; ii += 2 * (i.outline_vertical_skip + 1))
		{
			indices.push_back(ii);
			indices.push_back(ii + nCircleVertsDivide);
			indices.push_back(ii + 1);
			indices.push_back(ii + 1 + nCircleVertsDivide);
		}

		if (ii < nCircleVerts)
		{
			indices.push_back(ii);
			indices.push_back(ii + nCircleVertsDivide);
		}
	}

	else
	{
		append_circle_positions(positions, i.radius0, i.facet, 0.0f);

		for (uint ii = 1; ii <= divide; ii++)
		{
			append_circle_positions(positions, ::lerp(i.radius0, i.radius1, ii / static_cast<float>(divide)), i.facet, ii * fStep);
			append_cylinder_indices(i.facet, (ii-1) * i.facet, i.face_type);
		}

		if (has(i.semantics, flag_texcoords))
		{
			texcoords.resize(positions.size());
			for (size_t ii = 0; ii < texcoords.size(); ii++)
			{
				float2& c = texcoords[ii];
				const float3& p = positions[ii];

				float x = ((p.x + i.radius0) / (2.0f*i.radius0));
				float v = p.z / i.height;

				if (p.y <= 0.0f)
					c = float2(x * 0.5f, v);
				else
					c = float2(1.0f - (x * 0.5f), v);
			}
		}

		if (i.bases)
		{
			circle_init c;
			c.semantics = i.semantics & ~flag_normals; // normals get created later
			c.face_type = i.face_type;
			c.color = i.color;
			c.facet = i.facet;
			c.radius = __max(i.radius0, kVerySmall); // pure 0 causes a degenerate face and thus degenerate normals

			append_circle(c, false, 0.0f);
			
			c.face_type = opposite_face_type(i.face_type);
			c.radius = __max(i.radius1, kVerySmall); // pure 0 causes a degenerate face and thus degenerate normals

			append_circle(c, false, i.height);
		}
	}

	finalize(i);
}

void primitive_impl::initialize(const cone_init& i)
{
	// To support proper texcoords, the apex vert must be replicated, 
	// so just treat this as a sized cylinder.

	cylinder_init c;
	c.semantics = i.semantics;
	c.face_type = i.face_type;
	c.divide = i.divide;
	c.facet = i.facet;
	c.outline_vertical_skip = i.outline_vertical_skip;
	c.color = i.color;
	c.radius0 = i.radius;
	c.radius1 = kVerySmall;
	c.height = i.height;
	c.bases = i.base;

	initialize(c);
}

void primitive_impl::initialize(const torus_init& i)
{
	deinitialize();
	oCHECK(supports(i, all_semantics), "unsupported semantics");
	oCHECK_ARG(i.facet >= 3, "facet must be >= 3");
	oCHECK_ARG(i.divide >= 0, "invalid divide (must be >= 3)");
	oCHECK_ARG(i.outer_radius > i.inner_radius, "outer radius must be larger than inner radius");
	oCHECK_ARG(i.inner_radius >= kVerySmall && i.outer_radius >= kVerySmall, "inner/outer radius is too small");

	const float kCenterRadius = (i.inner_radius + i.outer_radius) * 0.5f;
	const float kRangeRadius = i.outer_radius - kCenterRadius;

	if (i.face_type == face_type::outline)
	{
		circle_init c;
		c.semantics = i.semantics;
		c.face_type = i.face_type;
		c.facet = i.facet;
		c.color = i.color;

		// main circle
		c.radius = kCenterRadius;
		append_circle(c, false, 0.0f);
		transform(make_rotation(radians(90.0f), float3(1.0f, 0.0f, 0.0f)));

		// small circles
		c.radius = kRangeRadius;
		uint nextCircleIndex = num_vertices();
		append_circle(c, false, 0.0f);
		transform(make_translation(float3(kCenterRadius, 0.0f, 0.0f)), nextCircleIndex);

		nextCircleIndex = num_vertices();
		append_circle(c, false, 0.0f);
		transform(make_translation(float3(-kCenterRadius, 0.0f, 0.0f)), nextCircleIndex);

		nextCircleIndex = num_vertices();
		append_circle(c, false, 0.0f);
		transform(make_rotation(radians(90.0f), mul(make_translation(float3(0.0f, 0.0f, kCenterRadius)), float3(0.0f, 1.0f, 0.0f))), nextCircleIndex);

		nextCircleIndex = num_vertices();
		append_circle(c, false, 0.0f);
		transform(make_rotation(radians(90.0f), mul(make_translation(float3(0.0f, 0.0f, -kCenterRadius)), float3(0.0f, 1.0f, 0.0f))), nextCircleIndex);
	}

	else
	{
		const float kInvDivide = 1.0f / static_cast<float>(i.divide);
		const float kInvFacet = 1.0f / static_cast<float>(i.facet);

		const float kDStep = 2.0f * oPIf / static_cast<float>(i.divide);
		const float kFStep = 2.0f * oPIf / static_cast<float>(i.facet);

		for (uint ii = 0; ii < i.divide; ii++)
		{
			const float2 S(sin(kDStep * ii), sin(kDStep * (ii+1)));
			const float2 C(cos(kDStep * ii), cos(kDStep * (ii+1)));

			for (uint j = 0; j < i.facet + 1; j++)
			{
				const float curFacet = (j % i.facet) * kFStep;
				const float fSin = sinf(curFacet);
				const float fCos = cosf(curFacet);

				for (int k = 0; k < 2; k++)
				{
					float3 center = float3(kCenterRadius * C[k], 0.0f, kCenterRadius* S[k]);

					float3 p = float3((kCenterRadius + kRangeRadius * fCos) * C[k], kRangeRadius * fSin, (kCenterRadius + kRangeRadius * fCos) * S[k]);

					if (has(i.semantics, flag_positions))
						positions.push_back(p);

					if (has(i.semantics, flag_texcoords))
						texcoords.push_back(float2(1.0f - (ii + k) * kInvDivide, j * kInvFacet));

					if (has(i.semantics, flag_normals))
						normals.push_back(normalize(p - center));
				}
			}
		}

		// This creates one long tri-strip so index it out to 
		// keep everything as indexed triangles.

		const uint nverts = num_vertices();
		for (uint ii = 2; ii < nverts; ii++)
		{
			if ((ii & 0x1) == 0)
			{
				indices.push_back(ii);
				indices.push_back(ii-1);
				indices.push_back(ii-2);
			}

			else
			{
				indices.push_back(ii-2);
				indices.push_back(ii-1);
				indices.push_back(ii-0);
			}
		}
	}

	if (i.face_type == face_type::front_ccw)
		flip_winding_order_to_end(0);

	finalize(i);
}

void primitive_impl::initialize(const teardrop_init& i)
{
	deinitialize();
	oCHECK(supports(i, flag_positions|flag_normals|flag_colors), "texcoords and tangents not supported");
	oCHECK_ARG(i.facet >= 3, "facet must be >= 3");
	oCHECK_ARG(i.divide > 3, "invalid divide");

	const uint kDividePlusOne = i.divide + 1;

	const float kUStep = oPIf / static_cast<float>(i.divide+1);
	const float kVStep = (2.0f * oPIf) / static_cast<float>(i.facet);

	positions.reserve((i.divide+1) * i.facet);
	normals.reserve(positions.size());

	for (size_t ii = 0; ii <= i.divide; ii++)
	{
		for (size_t j = 0; j < i.facet; j++)
		{
			const float u = kUStep * ii;
			const float v = kVStep * j;

			// TODO: finish this

			float o = 0.5f * (1.0f - cos(u)) * sin(u);
			float x = o * cos(v);
			float y = o * sin(v);
			float z = cos(u);
			
			float3 p(x,y,z);

			if (has(i.semantics, flag_positions))
				positions.push_back(p);
			if (has(i.semantics, flag_normals))
				normals.push_back(normalize(p));
		}
	}

	finalize(i);
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
	
	uint n = num_vertices();
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

void primitive_impl::prune()
{
	uint NewNumVerts = 0;
	prune_unindexed_vertices(indices.data(), as_uint(indices.size()), positions.data(), normals.data(), tangents.data(), texcoords.data(), (float2*)nullptr, colors.data(), num_vertices(), &NewNumVerts);
	if (NewNumVerts != positions.size())
		resize(indices.size(), NewNumVerts);
}

void primitive_impl::cull(const planef& plane)
{
	// go thru indices and remove triangles on back side of the plane
	auto it = std::begin(indices);
	while (it != indices.end())
	{
		if (!in_front_of(plane, positions[*it])
			&& !in_front_of(plane, positions[*(it+1)])
			&& !in_front_of(plane, positions[*(it+2)]))
			it = indices.erase(it, it+3);
		else
			it += 3;
	}

	prune();
}

void primitive_impl::finalize(const face_type::value& face_type, int semantics, const color& c)
{
	if (face_type != face_type::outline)
	{
		if (has(semantics, flag_normals) && normals.empty())
		{
			normals.resize(positions.size());
			calc_vertex_normals(normals.data(), indices.data(), num_indices(), positions.data(), num_vertices(), face_type == face_type::front_ccw);
		}
		
		if (has(semantics, flag_tangents) && tangents.empty())
		{
			tangents.resize(positions.size());
			calc_vertex_tangents(tangents.data(), indices.data(), num_indices(), positions.data(), normals.data(), texcoords.data(), num_vertices());
		}
	}

	if (has(semantics, flag_colors))
		colors.assign(positions.size(), c);

	if (ranges.empty())
	{
		range r;
		r.start_primitive = 0;
		r.num_primitives = as_uint(indices.size() / 3);
		r.max_vertex = num_vertices();
		ranges.push_back(r);
	}

	inf.local_space_bound = calc_bound(positions.data(), sizeof(float3), num_vertices());

	int el = 0;
	if (has(semantics, flag_positions))
		inf.elements[el] = element(semantic::position, 0, format::xyz32_float, 0), el++;
	if (has(semantics, flag_normals))
		inf.elements[el] = element(semantic::normal, 0, format::xyz32_float, 1), el++;
	if (has(semantics, flag_tangents))
		inf.elements[el] = element(semantic::tangent, 0, format::xyzw32_float, 2), el++;
	if (has(semantics, flag_texcoords))
		inf.elements[el] = element(semantic::texcoord, 0, format::xy32_float, 3), el++;
	if (has(semantics, flag_colors))
		inf.elements[el] = element(semantic::color, 0, format::bgra8_unorm, 4), el++;

	inf.num_indices = num_indices();
	inf.num_vertices = num_vertices();
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

	static const uint kOutlineindices[8] = { 0,1, 2,3, 0,2, 1,3 };
	static const uint kTriindices[6] = { 0,2,1, 1,2,3 };

	if (!has(i, flag_positions))
		return;

	float4x4 m = make_scale(float3(i.width, i.height, 1.0f));
	if (i.centered)
		m = mul(make_translation(float3(-0.5f, -0.5f, 0.0f)), m);

	uint baseVertexIndex = num_vertices();
	uint baseIndexIndex = num_indices();

	for (const float3& corner : kCorners)
		positions.push_back(mul(m, corner));

	if (i.face_type == face_type::outline)
	{
		for (const uint index : kOutlineindices)
			indices.push_back(baseVertexIndex + index);
	}

	else
	{
		for (const uint index : kTriindices)
			indices.push_back(baseVertexIndex + index);

		if (i.face_type == face_type::front_ccw)
			flip_winding_order_to_end(baseIndexIndex);
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

void primitive_impl::append_frustum_positions(const oFrustumf& f)
{
	uint index = num_vertices();
	positions.resize(positions.size() + 8 * 3); // extra verts for normals
	float3* p = &positions[index];
	oCHECK0(oExtractFrustumCorners(f, p));

	// duplicate corners for normals
	memcpy(p + 8, p, 8 * sizeof(float3));
	memcpy(p + 16, p, 8 * sizeof(float3));
}

void primitive_impl::append_frustum_indices()
{
	const uint kIndices[] =
	{
		// Left
		oFRUSTUM_LEFT_TOP_NEAR,
		oFRUSTUM_LEFT_BOTTOM_NEAR,
		oFRUSTUM_LEFT_TOP_FAR,
		oFRUSTUM_LEFT_TOP_FAR,
		oFRUSTUM_LEFT_BOTTOM_NEAR,
		oFRUSTUM_LEFT_BOTTOM_FAR,

		// Right
		oFRUSTUM_RIGHT_TOP_NEAR,
		oFRUSTUM_RIGHT_TOP_FAR,
		oFRUSTUM_RIGHT_BOTTOM_FAR,
		oFRUSTUM_RIGHT_BOTTOM_FAR,
		oFRUSTUM_RIGHT_BOTTOM_NEAR,
		oFRUSTUM_RIGHT_TOP_NEAR,

		// Top
		oFRUSTUM_LEFT_TOP_NEAR + 8,
		oFRUSTUM_LEFT_TOP_FAR + 8,
		oFRUSTUM_RIGHT_TOP_FAR + 8,
		oFRUSTUM_RIGHT_TOP_FAR + 8,
		oFRUSTUM_RIGHT_TOP_NEAR + 8,
		oFRUSTUM_LEFT_TOP_NEAR + 8,

		// Bottom
		oFRUSTUM_LEFT_BOTTOM_FAR + 8,
		oFRUSTUM_LEFT_BOTTOM_NEAR + 8,
		oFRUSTUM_RIGHT_BOTTOM_FAR + 8,
		oFRUSTUM_RIGHT_BOTTOM_FAR + 8,
		oFRUSTUM_LEFT_BOTTOM_NEAR + 8,
		oFRUSTUM_RIGHT_BOTTOM_NEAR + 8,

		// Near
		oFRUSTUM_LEFT_TOP_NEAR + 16,
		oFRUSTUM_RIGHT_TOP_NEAR + 16,
		oFRUSTUM_RIGHT_BOTTOM_NEAR + 16,
		oFRUSTUM_RIGHT_BOTTOM_NEAR + 16,
		oFRUSTUM_LEFT_BOTTOM_NEAR + 16,
		oFRUSTUM_LEFT_TOP_NEAR + 16,

		// Far
		oFRUSTUM_LEFT_TOP_FAR + 16,
		oFRUSTUM_LEFT_BOTTOM_FAR + 16,
		oFRUSTUM_RIGHT_BOTTOM_FAR + 16,
		oFRUSTUM_RIGHT_BOTTOM_FAR + 16,
		oFRUSTUM_RIGHT_TOP_FAR + 16,
		oFRUSTUM_LEFT_TOP_FAR + 16,
	};

	uint index = num_indices();
	indices.resize(indices.size() + oCOUNTOF(kIndices));
	memcpy(&indices[index], kIndices, sizeof(kIndices));
}

void primitive_impl::append_frustum_outline_indices()
{
	const uint kIndices[] =
	{
		// Left
		oFRUSTUM_LEFT_TOP_NEAR,
		oFRUSTUM_LEFT_TOP_FAR,
		oFRUSTUM_LEFT_TOP_FAR,
		oFRUSTUM_LEFT_BOTTOM_FAR,
		oFRUSTUM_LEFT_BOTTOM_FAR,
		oFRUSTUM_LEFT_BOTTOM_NEAR,
		oFRUSTUM_LEFT_BOTTOM_NEAR,
		oFRUSTUM_LEFT_TOP_NEAR,

		// Right
		oFRUSTUM_RIGHT_TOP_NEAR,
		oFRUSTUM_RIGHT_TOP_FAR,
		oFRUSTUM_RIGHT_TOP_FAR,
		oFRUSTUM_RIGHT_BOTTOM_FAR,
		oFRUSTUM_RIGHT_BOTTOM_FAR,
		oFRUSTUM_RIGHT_BOTTOM_NEAR,
		oFRUSTUM_RIGHT_BOTTOM_NEAR,
		oFRUSTUM_RIGHT_TOP_NEAR,


		// Top
		oFRUSTUM_LEFT_TOP_NEAR,
		oFRUSTUM_RIGHT_TOP_NEAR,
		oFRUSTUM_LEFT_TOP_FAR,
		oFRUSTUM_RIGHT_TOP_FAR,

		// Bottom
		oFRUSTUM_LEFT_BOTTOM_NEAR,
		oFRUSTUM_RIGHT_BOTTOM_NEAR,
		oFRUSTUM_LEFT_BOTTOM_FAR,
		oFRUSTUM_RIGHT_BOTTOM_FAR,
	};

	uint index = num_indices();
	indices.resize(indices.size() + oCOUNTOF(kIndices));
	memcpy(&indices[index], kIndices, sizeof(kIndices));
}

void primitive_impl::append_circle_indices(uint base_index_index, uint base_vertex_index, uint facet, const face_type::value& face_type)
{
	if (face_type == face_type::outline)
	{
		indices.reserve(indices.size() + facet * 2);
		
		const uint numEven = (facet - 1) / 2;
		const uint numOdd = (facet / 2) - 1; // -1 for transition from 0 to 1, which does not fit the for loop

		for (uint i = 0; i < numEven; i++)
		{
			indices.push_back(base_vertex_index + i * 2);
			indices.push_back(base_vertex_index + (i + 1) * 2);
		}

		for (uint i = 0; i < numOdd; i++)
		{
			uint idx = i * 2 + 1;

			indices.push_back(base_vertex_index + idx);
			indices.push_back(base_vertex_index + idx + 2);
		}

		// Edge cases are 0 -> 1 and even -> odd

		indices.push_back(base_vertex_index);
		indices.push_back(base_vertex_index + 1);

		indices.push_back(base_vertex_index + facet - 1);
		indices.push_back(base_vertex_index + facet - 2);
	}

	else
	{
		const uint count = facet - 2;
		indices.reserve(3 * count);

		const uint o[2][2] = { { 2, 1 }, { 1, 2 } };
		for (uint i = 0; i < count; i++)
		{
			indices.push_back(base_vertex_index + i);
			indices.push_back(base_vertex_index + i+o[i&0x1][0]);
			indices.push_back(base_vertex_index + i+o[i&0x1][1]);
		}

		if (face_type == face_type::front_ccw)
			flip_winding_order_to_end(base_index_index);
	}
}

void primitive_impl::append_washer_indices(uint base_index_index, uint base_vertex_index, uint facet, const face_type::value& face_type)
{
	if (face_type == face_type::outline)
	{
		append_circle_indices(base_index_index, base_vertex_index, facet, face_type);
		append_circle_indices(base_index_index+facet*2, base_vertex_index+facet, facet, face_type);
	}

	else
	{
		const uint count = facet - 2;
		indices.reserve(6 * facet);

		//end caps done out of loop. connecting first even odd pair
		indices.push_back(base_vertex_index + 0);
		indices.push_back(base_vertex_index + 2);
		indices.push_back(base_vertex_index + 1);

		indices.push_back(base_vertex_index + 1);
		indices.push_back(base_vertex_index + 2);
		indices.push_back(base_vertex_index + 3);

		{
			const uint o[2][4] = { { 1, 4, 5, 4 }, { 4, 1, 4, 5 } };
			for (uint i = 0; i < count; i++)
			{
				indices.push_back(base_vertex_index + 2*i);
				indices.push_back(base_vertex_index + 2*i+o[i&0x1][0]);
				indices.push_back(base_vertex_index + 2*i+o[i&0x1][1]);

				indices.push_back(base_vertex_index + 2*i+1);
				indices.push_back(base_vertex_index + 2*i+o[i&0x1][2]);
				indices.push_back(base_vertex_index + 2*i+o[i&0x1][3]);
			}
		}

		{
			//end caps done out of loop. connecting last even odd pair
			const uint o[2][4] = { { 1, 2, 3, 2 }, { 2, 1, 2, 3 } };
			uint i = facet*2-4;
			indices.push_back(base_vertex_index + i);
			indices.push_back(base_vertex_index + i+o[facet&0x1][0]);
			indices.push_back(base_vertex_index + i+o[facet&0x1][1]);

			indices.push_back(base_vertex_index + i+1);
			indices.push_back(base_vertex_index + i+o[facet&0x1][2]);
			indices.push_back(base_vertex_index + i+o[facet&0x1][3]);
		}

		if (face_type == face_type::front_ccw)
			flip_winding_order_to_end(base_index_index);
	}
}

void primitive_impl::append_circle_positions(std::vector<float3>& inout_positions, float radius, uint facet, float z_value)
{
	inout_positions.reserve(inout_positions.size() + facet);
	float step = (2.0f * oPIf) / static_cast<float>(facet);
	float curStep2 = 0.0f;
	float curStep = (2.0f * oPIf) - step;
	uint k = 0;
	for (uint i = 0; i < facet && k < facet; i++, curStep -= step, curStep2 += step)
	{
		inout_positions.push_back(float3(radius * cosf(curStep), radius * sinf(curStep), z_value));
		if (++k >= facet)
			break;
		inout_positions.push_back(float3(radius * cosf(curStep2), radius * sinf(curStep2), z_value));
		if (++k >= facet)
			break;
	}
}

void primitive_impl::append_circle_texcoords(std::vector<float2>& inout_texcoords, uint facet, float radius)
{
	inout_texcoords.reserve(inout_texcoords.size() + facet);
	float step = (2.0f * oPIf) / static_cast<float>(facet);
	float curStep2 = 0.0f;
	float curStep = (2.0f * oPIf) - step;
	uint k = 0;
	for (uint i = 0; i < facet && k < facet; i++, curStep -= step, curStep2 += step)
	{
		inout_texcoords.push_back(float2(radius * cosf(curStep), radius * sinf(curStep)) * 0.5f + 0.5f);
		if (++k >= facet)
			break;
		inout_texcoords.push_back(float2(radius * cosf(curStep2), radius * sinf(curStep2)) * 0.5f + 0.5f);
		if (++k >= facet)
			break;
	}
}

void primitive_impl::append_circle_normals(uint base_vertex_index, const face_type::value& face_type)
{
	normals.resize(positions.size());
	const float3 N = float3(0.0f, 0.0f, 1.0f) * normal_sign(face_type);
	for (size_t i = base_vertex_index; i < normals.size(); i++)
		normals[i] = N;
}

void primitive_impl::append_circle(const circle_init& i, uint base_index_index, uint base_vertex_index, bool clear_primitive, float z_value)
{
	oCHECK_ARG(i.radius >= kVerySmall, "radius too small");
	oCHECK_ARG(i.facet >= 3, "facet must be >= 3");
	if (clear_primitive)
		clear();

	append_circle_positions(positions, i.radius, i.facet, z_value);

	if (i.face_type == face_type::outline)
		append_circle_indices(base_index_index, base_vertex_index, i.facet, i.face_type);
	else
	{
		append_circle_indices(base_index_index, base_vertex_index, i.facet, i.face_type);
		if (has(i.semantics, flag_normals))
			append_circle_normals(base_vertex_index, i.face_type);
		if (has(i.semantics, flag_texcoords))
			append_circle_texcoords(texcoords, i.facet, 1.0f);
	}
}

void primitive_impl::append_washer(const washer_init& i, uint base_index_index, uint base_vertex_index, bool clear_primitive, float z_value)
{
	oCHECK_ARG(i.outer_radius > i.inner_radius, "outer radius must be larger than inner radius");
	oCHECK_ARG(i.inner_radius >= kVerySmall && i.outer_radius >= kVerySmall, "inner/outer radius is too small");
	oCHECK_ARG(i.facet >= 3, "facet must be >= 3");

	if (clear_primitive)
		clear();

	std::vector<float3> innerCircle;
	std::vector<float3> outerCircle;
	append_circle_positions(innerCircle, i.inner_radius, i.facet, z_value);
	append_circle_positions(outerCircle, i.outer_radius, i.facet, z_value);

	//For outlines, the vertex layout is the full inner circle, followed by the full outer circle
	if (i.face_type == face_type::outline)
	{
		positions.insert(end(positions), begin(innerCircle), end(innerCircle));
		positions.insert(end(positions), begin(outerCircle), end(outerCircle));
		append_washer_indices(base_index_index, base_vertex_index, i.facet, i.face_type);
	}

	else 
	{
		// Similarly to a normal circle, interleave even and odd pairs of vertices. For each pair the
		// first vertex is from the inner circle and second is from the outer circle. even pairs going 
		// one direction around the circle odd going the opposite direction.
		oASSERT(innerCircle.size() == outerCircle.size(),"washer inner circle and outer circle must be the same size");

		for (size_t ii = 0; ii < innerCircle.size(); ii++)
		{
			positions.push_back(innerCircle[ii]);
			positions.push_back(outerCircle[ii]);
		}

		append_washer_indices(base_index_index, base_vertex_index, i.facet, i.face_type);

		if (has(i.semantics, flag_normals))
			append_circle_normals(base_vertex_index, i.face_type);

		if (has(i.semantics, flag_texcoords))
		{
			std::vector<float2> innerCircleTexcoords;
			std::vector<float2> outerCircleTexcoords;
			append_circle_texcoords(innerCircleTexcoords, i.facet, i.inner_radius / i.outer_radius);
			append_circle_texcoords(outerCircleTexcoords, i.facet, 1.0f);

			for (size_t ii = 0; ii < innerCircleTexcoords.size(); ii++)
			{
				texcoords.push_back(innerCircleTexcoords[ii]);
				texcoords.push_back(outerCircleTexcoords[ii]);
			}
		}
	}
}

void primitive_impl::append_cylinder_indices(uint facet, uint base_vertex_index, const face_type::value& face_type)
{
	const uint numEvens = (facet + 1) / 2;
	const uint oddsStartI = facet - 1 - (facet & 0x1);
	const uint numOdds = facet / 2 - 1;
	const uint cwoStartIndex = num_indices();

	for (uint i = 1; i < numEvens; i++)
	{
		indices.push_back(base_vertex_index + i*2);
		indices.push_back(base_vertex_index + (i-1)*2);
		indices.push_back(base_vertex_index + i*2 + facet);

		indices.push_back(base_vertex_index + (i-1)*2);
		indices.push_back(base_vertex_index + (i-1)*2 + facet);
		indices.push_back(base_vertex_index + i*2 + facet);
	}

	// Transition from even to odd
	indices.push_back(base_vertex_index + facet-1);
	indices.push_back(base_vertex_index + facet-2);
	indices.push_back(base_vertex_index + facet-1 + facet);
	
	indices.push_back(base_vertex_index + facet-2);
	indices.push_back(base_vertex_index + facet-2 + facet);
	indices.push_back(base_vertex_index + facet-1 + facet);

	if (facet & 0x1)
	{
		std::swap(*(indices.end()-5), *(indices.end()-4));
		std::swap(*(indices.end()-2), *(indices.end()-1));
	}
	
	for (uint i = oddsStartI, k = 0; k < numOdds; i -= 2, k++)
	{
		indices.push_back(base_vertex_index + i);
		indices.push_back(base_vertex_index + i + facet);
		indices.push_back(base_vertex_index + i - 2);

		indices.push_back(base_vertex_index + i - 2);
		indices.push_back(base_vertex_index + i + facet);
		indices.push_back(base_vertex_index + i - 2 + facet);
	}

	// Transition from last odd back to 0
	indices.push_back(base_vertex_index);
	indices.push_back(base_vertex_index + 1);
	indices.push_back(base_vertex_index + facet);

	indices.push_back(base_vertex_index + facet);
	indices.push_back(base_vertex_index + 1);
	indices.push_back(base_vertex_index + 1 + facet);

	if (face_type == face_type::front_ccw)
		flip_winding_order_to_end(cwoStartIndex);
}

static void fix_cylindrical_seam(float2& texcoord0, float2& texcoord1, float threshold)
{
	if ((texcoord0.x - texcoord1.x) > threshold)
	{
		if (texcoord0.x < texcoord1.x)
			texcoord0.x += 1.0f;
		else
			texcoord1.x += 1.0f;
	}
}

void primitive_impl::fix_cylindrical_texcoord_seams(float texcoord_threshold)
{
	const uint kNumIndices = num_indices();
	for (uint i = 0; i < kNumIndices; i += 3)
	{
		const uint i0 = indices[i];
		const uint i1 = indices[i+1];
		const uint i2 = indices[i+2];

		fix_cylindrical_seam(texcoords[i0], texcoords[i1], texcoord_threshold);
		fix_cylindrical_seam(texcoords[i0], texcoords[i2], texcoord_threshold);
		fix_cylindrical_seam(texcoords[i1], texcoords[i2], texcoord_threshold);
	}
}

void primitive_impl::fix_sphere_poles_texcoord_seams()
{
	// find verts at extreme poles (hopefully this is one vert... rotate platonics to enforce this (I think icos doesn't have two pole verts though)
	// find each triangle that vert is a part of
	// dup the vert and average the other two verts' u coord and set that to the dup'ed vert's u coord
}

void primitive_impl::set_sphere_texcoords(bool hemisphere)
{
	texcoords.clear();
	texcoords.reserve(num_vertices());

	for (const float3& p : positions)
	{
		float phi = acos(p.z);
		float v = 1.0f - (phi / oPIf);

		// Map from 0 -> 1 rather than 0.5 -> 1 since half the sphere is cut off
		if (hemisphere)
			v = (v - 0.5f) * 2.0f;

		float u = 0.5f;
		if (!equal(phi, 0.0f))
		{
			float a = clamp(p.y / sin(phi), -1.0f, 1.0f);
			u = acos(a) / (2.0f * oPIf);

			if (p.x > 0.0f)
				u = 1.0f - u;
		}

		texcoords.push_back(float2(u, v));
	}
}

}}
