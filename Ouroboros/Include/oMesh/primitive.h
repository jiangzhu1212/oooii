/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#pragma once
#ifndef oMesh_primitive_h
#define oMesh_primitive_h

#include <oBase/color.h>
#include <oMesh/mesh.h>

#include <oCompute/oFrustum.h>

namespace ouro { namespace mesh {

class primitive
{
public:
	struct deleter { void operator()(primitive* p) { p->unmake(); } };

	typedef std::unique_ptr<primitive, deleter> unique_ptr;

	enum semantic_flag
	{
		flag_positions = 1,
		flag_normals = 2,
		flag_tangents = 4,
		flag_texcoords = 8,
		flag_colors = 16,

		all_semantics = flag_positions|flag_normals|flag_tangents|flag_texcoords|flag_colors,
	};

	struct rectangle_init
	{
		int semantics; // semantic_flags
		face_type::value face_type;
		uint divide;
		color color;
		float width;
		float height;
		bool centered;
		bool flipv;
	};

	struct box_init
	{
		int semantics; // semantic_flags
		face_type::value face_type;
		uint divide;
		color color;
		aaboxf bound;
		bool flipv;
	};

	struct frustum_init
	{
		int semantics; // semantic_flags
		face_type::value face_type;
		uint divide;
		color color;
		oFrustumf bound;
	};

	struct circle_init
	{
		int semantics; // semantic_flags
		face_type::value face_type;
		uint facet;
		color color;
		float radius;
	};

	struct washer_init
	{
		int semantics; // semantic_flags
		face_type::value face_type;
		uint facet;
		color color;
		float inner_radius;
		float outer_radius;
	};

	struct sphere_init
	{
		// icosahedron: T: use icosahedron subdivision F: use octahedron subdivision
		// texture coord u goes from 0 at Y=+1 to 0.25 at X=-1 to 0.5 at Y=-1 to 0.75 at X=+1 back to 1.0 at Y=+1
		// texture coord v goes from 0 at Z=+1 to 1 at Z=-1, or if hemisphere, 0 at Z=+1 and 1 at Z=0

		int semantics; // semantic_flags
		face_type::value face_type;
		uint divide; // vale of 6 takes ~3 sec on an overclocked i7 920. 7 Takes ~11 sec.
		color color;
		float radius;
		bool hemisphere;
		bool icosahedron;
	};

	struct cylinder_init
	{
		int semantics; // semantic_flags
		face_type::value face_type;
		uint divide;
		uint facet;
		uint outline_vertical_skip; // # vertical lines to skip when generating an outline. 1 means skip every other line, etc.
		color color;
		float radius0;
		float radius1;
		float height;
		bool bases;
	};

	struct cone_init
	{
		int semantics; // semantic_flags
		face_type::value face_type;
		uint divide;
		uint facet;
		uint outline_vertical_skip; // # vertical lines to skip when generating an outline. 1 means skip every other line, etc.
		color color;
		float radius;
		float height;
		bool base;
	};

	struct torus_init
	{
		int semantics; // semantic_flags
		face_type::value face_type;
		uint divide;
		uint facet;
		color color;
		float inner_radius;
		float outer_radius;
	};

	struct teardrop_init
	{
		int semantics; // semantic_flags
		face_type::value face_type;
		uint divide;
		uint facet;
		color color;
	};

	struct source
	{
		const range* ranges;
		const uint* indices;
		union
		{
			const void* streams[5];
			struct
			{
				const float3* positions;
				const float3* normals;
				const float4* tangents;
				const float2* texcoords;
				const color* colors;
			};
		};
	};

	// do not call delete, call unmake to destroy the pointer
	static primitive* make();
	virtual void unmake() = 0;

	virtual info get_info() const = 0;
	virtual source get_source() const = 0;

	virtual void initialize(const rectangle_init& i) = 0;
	virtual void initialize(const box_init& i) = 0;
	virtual void initialize(const frustum_init& i) = 0;
	virtual void initialize(const circle_init& i) = 0;
	virtual void initialize(const washer_init& i) = 0;
	virtual void initialize(const sphere_init& i) = 0;
	virtual void initialize(const cylinder_init& i) = 0;
	virtual void initialize(const cone_init& i) = 0;
	virtual void initialize(const torus_init& i) = 0;
	virtual void initialize(const teardrop_init& i) = 0;
	virtual void deinitialize() = 0;

	template<typename initT>
	static inline primitive* make(const initT& i) { primitive* p = make(); p->initialize(i); return p; }
};

}}

#endif
