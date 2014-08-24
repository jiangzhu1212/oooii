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
#include "obj_test.h"
#include <oBase/aabox.h>

namespace ouro {
	namespace tests {

class obj_test_cube : public obj_test
{
public:
	obj_test_cube()
	{
		init_groups();
	}

	mesh::obj::info get_info() const override
	{
		mesh::obj::info i;

		i.obj_path = "cube.obj";
		i.mtl_path = "cube.mtl";
		i.groups = sGroups;

		i.ranges = sRanges;
		i.indices = sIndices;
		i.positions = sPositions;
		i.normals = sNormals;
		i.texcoords = sTexcoords;

		i.mesh_info.local_space_bound = aaboxf(aaboxf::min_max, float3(-0.5), float3(0.5f));
		i.mesh_info.elements[0] = mesh::element(surface::semantic::vertex_position, 0, surface::format::r32g32b32_float, 0);
		i.mesh_info.elements[1] = mesh::element(surface::semantic::vertex_normal, 0, surface::format::r32g32b32_float, 1);
		i.mesh_info.elements[2] = mesh::element(surface::semantic::vertex_tangent, 0, surface::format::r32g32b32a32_float, 2);
		i.mesh_info.num_indices = oCOUNTOF(sIndices);
		i.mesh_info.num_vertices = oCOUNTOF(sPositions);
		i.mesh_info.primitive_type = mesh::primitive_type::triangles;
		i.mesh_info.face_type = mesh::face_type::unknown;
		i.mesh_info.num_ranges = oCOUNTOF(sRanges);
		i.mesh_info.vertex_scale_shift = 0;
		return i;
	}

	const char* file_contents() const override
	{
		return
			"# Simple Unit Cube\n" \
			"mtllib cube.mtl\n" \
			"\n" \
			"	v -0.5 -0.5 -0.5\n" \
			"	v 0.5 -0.5 -0.5\n" \
			"	v -0.5 0.5 -0.5\n" \
			"	v 0.5 0.5 -0.5\n" \
			"	v -0.5 -0.5 0.5\n" \
			"	v 0.5 -0.5 0.5\n" \
			"	v -0.5 0.5 0.5\n" \
			"	v 0.5 0.5 0.5\n" \
			"\n" \
			"	vn -1.0 0.0 0.0\n" \
			"	vn 1.0 0.0 0.0\n" \
			"	vn 0.0 1.0 0.0\n" \
			"	vn 0.0 -1.0 0.0\n" \
			"	vn 0.0 0.0 -1.0\n" \
			"	vn 0.0 0.0 1.0\n" \
			"\n" \
			"	vt 0.0 0.0\n" \
			"	vt 1.0 0.0\n" \
			"	vt 0.0 1.0\n" \
			"	vt 1.0 1.0\n" \
			"\n" \
			"	usemtl Body\n" \
			"	g Left\n" \
			"	f 5/3/1 3/4/1 7/1/1\n" \
			"	f 1/2/1 3/4/1 5/3/1\n" \
			"\n" \
			"	g Right\n" \
			"	f 6/2/2 8/4/2 4/3/2\n" \
			"	f 2/1/2 6/2/2 4/3/2\n" \
			"\n" \
			"	g Top\n" \
			"	f 3/1/3 8/4/3 7/3/3\n" \
			"	f 3/1/3 4/2/3 8/4/3\n" \
			"\n" \
			"	g Bottom\n" \
			"	f 1/3/4 5/1/4 6/2/4\n" \
			"	f 2/4/4 1/3/4 6/2/4\n" \
			"\n" \
			"	g Near\n" \
			"	f 1/1/5 2/2/5 3/3/5\n" \
			"	f 2/2/5 4/4/5 3/3/5\n" \
			"\n" \
			"	g Far\n" \
			"	f 5/2/6 7/4/6 6/1/6\n" \
			"	f 6/1/6 7/4/6 8/3/6\n";
	}

private:

	static const float3 sPositions[24];
	static const float3 sTexcoords[24];
	static const float3 sNormals[24];
	static const uint sIndices[36];
	static const char* sGroupNames[6];
	static mesh::obj::group sGroups[6];
	static mesh::range sRanges[6];
	static bool InitGroupsDone;
	static void init_groups()
	{
		if (!InitGroupsDone)
		{
			int i = 0;
			for (auto& g : sGroups)
			{
				g.group_name = sGroupNames[i];
				g.material_name = "Body";
				sRanges[i] = mesh::range(i*2, 2, i*4, (i+1)*4-1); // min/max is valid after vertex reduction
				i++;
			}

			InitGroupsDone = true;
		}
	}
};

const float3 obj_test_cube::sPositions[24] = 
{
	float3(-0.500000f, -0.500000f, 0.500000f),
	float3(-0.500000f, 0.500000f, -0.500000f),
	float3(-0.500000f, 0.500000f, 0.500000f),
	float3(-0.500000f, -0.500000f, -0.500000f),
	float3(0.500000f, -0.500000f, 0.500000f),
	float3(0.500000f, 0.500000f, 0.500000f),
	float3(0.500000f, 0.500000f, -0.500000f),
	float3(0.500000f, -0.500000f, -0.500000f),
	float3(-0.500000f, 0.500000f, -0.500000f),
	float3(0.500000f, 0.500000f, 0.500000f),
	float3(-0.500000f, 0.500000f, 0.500000f),
	float3(0.500000f, 0.500000f, -0.500000f),
	float3(-0.500000f, -0.500000f, -0.500000f),
	float3(-0.500000f, -0.500000f, 0.500000f),
	float3(0.500000f, -0.500000f, 0.500000f),
	float3(0.500000f, -0.500000f, -0.500000f),
	float3(-0.500000f, -0.500000f, -0.500000f),
	float3(0.500000f, -0.500000f, -0.500000f),
	float3(-0.500000f, 0.500000f, -0.500000f),
	float3(0.500000f, 0.500000f, -0.500000f),
	float3(-0.500000f, -0.500000f, 0.500000f),
	float3(-0.500000f, 0.500000f, 0.500000f),
	float3(0.500000f, -0.500000f, 0.500000f),
	float3(0.500000f, 0.500000f, 0.500000f),
};

const float3 obj_test_cube::sTexcoords[24] = 
{
	float3(0.000000f, 1.000000f, 0.000000f),
	float3(1.000000f, 1.000000f, 0.000000f),
	float3(0.000000f, 0.000000f, 0.000000f),
	float3(1.000000f, 0.000000f, 0.000000f),
	float3(1.000000f, 0.000000f, 0.000000f),
	float3(1.000000f, 1.000000f, 0.000000f),
	float3(0.000000f, 1.000000f, 0.000000f),
	float3(0.000000f, 0.000000f, 0.000000f),
	float3(0.000000f, 0.000000f, 0.000000f),
	float3(1.000000f, 1.000000f, 0.000000f),
	float3(0.000000f, 1.000000f, 0.000000f),
	float3(1.000000f, 0.000000f, 0.000000f),
	float3(0.000000f, 1.000000f, 0.000000f),
	float3(0.000000f, 0.000000f, 0.000000f),
	float3(1.000000f, 0.000000f, 0.000000f),
	float3(1.000000f, 1.000000f, 0.000000f),
	float3(0.000000f, 0.000000f, 0.000000f),
	float3(1.000000f, 0.000000f, 0.000000f),
	float3(0.000000f, 1.000000f, 0.000000f),
	float3(1.000000f, 1.000000f, 0.000000f),
	float3(1.000000f, 0.000000f, 0.000000f),
	float3(1.000000f, 1.000000f, 0.000000f),
	float3(0.000000f, 0.000000f, 0.000000f),
	float3(0.000000f, 1.000000f, 0.000000f),
};

const float3 obj_test_cube::sNormals[24] = 
{
	float3(-1.000000f, 0.000000f, 0.000000f),
	float3(-1.000000f, 0.000000f, 0.000000f),
	float3(-1.000000f, 0.000000f, 0.000000f),
	float3(-1.000000f, 0.000000f, 0.000000f),
	float3(1.000000f, 0.000000f, 0.000000f),
	float3(1.000000f, 0.000000f, 0.000000f),
	float3(1.000000f, 0.000000f, 0.000000f),
	float3(1.000000f, 0.000000f, 0.000000f),
	float3(0.000000f, 1.000000f, 0.000000f),
	float3(0.000000f, 1.000000f, 0.000000f),
	float3(0.000000f, 1.000000f, 0.000000f),
	float3(0.000000f, 1.000000f, 0.000000f),
	float3(0.000000f, -1.000000f, 0.000000f),
	float3(0.000000f, -1.000000f, 0.000000f),
	float3(0.000000f, -1.000000f, 0.000000f),
	float3(0.000000f, -1.000000f, 0.000000f),
	float3(0.000000f, 0.000000f, -1.000000f),
	float3(0.000000f, 0.000000f, -1.000000f),
	float3(0.000000f, 0.000000f, -1.000000f),
	float3(0.000000f, 0.000000f, -1.000000f),
	float3(0.000000f, 0.000000f, 1.000000f),
	float3(0.000000f, 0.000000f, 1.000000f),
	float3(0.000000f, 0.000000f, 1.000000f),
	float3(0.000000f, 0.000000f, 1.000000f),
};

const uint obj_test_cube::sIndices[36] =
{
	0,2,1,3,0,1,
	4,6,5,7,6,4,
	8,10,9,8,9,11,
	12,14,13,15,14,12,
	16,18,17,17,18,19,
	20,22,21,22,23,21,
};

const char* obj_test_cube::sGroupNames[6] = 
{
	"Left",
	"Right",
	"Top",
	"Bottom",
	"Near",
	"Far"
};

bool obj_test_cube::InitGroupsDone = false;
mesh::obj::group obj_test_cube::sGroups[6];
mesh::range obj_test_cube::sRanges[6];

std::shared_ptr<obj_test> obj_test::make(which _Which)
{
	switch (_Which)
	{
		case obj_test::cube: return std::make_shared<obj_test_cube>();
		default: break;
	}

	oTHROW_INVARG("invalid obj_test");
}

	} // tests
} // namespace ouro

