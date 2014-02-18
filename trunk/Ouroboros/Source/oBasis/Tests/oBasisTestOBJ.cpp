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
#include "oBasisTestOBJ.h"
#include <oCompute/oAABox.h>

struct oBasisTestOBJ_Cube : oBasisTestOBJ
{
public:
	oBasisTestOBJ_Cube()
	{
		InitGroups();
	}

	ouro::mesh::obj::info get_info() const threadsafe override
	{
		ouro::mesh::obj::info i;

		i.obj_path = "cube.obj";
		i.mtl_path = "cube.mtl";
		i.groups = sGroups;

		i.data.indicesi = sIndices;
		i.data.ranges = sRanges;
		i.data.indexi_pitch = sizeof(uint);
		i.data.positionsf = sPositions;
		i.data.positionf_pitch = sizeof(float3);
		i.data.normalsf = sNormals;
		i.data.normalf_pitch = sizeof(float3);
		i.data.uvw0sf = sTexcoords;
		i.data.uvw0f_pitch = sizeof(float3);
		
		i.mesh_info.face_type = ouro::mesh::face_type::unknown;
		i.mesh_info.local_space_bound = ouro::aaboxf(ouro::aaboxf::min_max, float3(-0.5), float3(0.5f));
		i.mesh_info.num_indices = oCOUNTOF(sIndices);
		i.mesh_info.num_ranges = oCOUNTOF(sRanges);
		i.mesh_info.num_vertices = oCOUNTOF(sPositions);
		i.mesh_info.primitive_type = ouro::mesh::primitive_type::triangles;
		i.mesh_info.vertex_layouts[0] = ouro::mesh::layout::pos_nrm_tan_uv0;
		i.mesh_info.vertex_scale_shift = 0;
		return i;
	}

	const char* GetFileContents() const threadsafe override
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
	static ouro::mesh::obj::group sGroups[6];
	static ouro::mesh::range sRanges[6];
	static bool InitGroupsDone;
	static void InitGroups()
	{
		if (!InitGroupsDone)
		{
			int i = 0;
			for (auto& g : sGroups)
			{
				g.group_name = sGroupNames[i];
				g.material_name = "Body";
				sRanges[i] = ouro::mesh::range(i*2, 2, i*4, (i+1)*4-1); // min/max is valid after vertex reduction
				i++;
			}

			InitGroupsDone = true;
		}
	}
};

const float3 oBasisTestOBJ_Cube::sPositions[24] = 
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

const float3 oBasisTestOBJ_Cube::sTexcoords[24] = 
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

const float3 oBasisTestOBJ_Cube::sNormals[24] = 
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

const uint oBasisTestOBJ_Cube::sIndices[36] =
{
	0,2,1,3,0,1,
	4,6,5,7,6,4,
	8,10,9,8,9,11,
	12,14,13,15,14,12,
	16,18,17,17,18,19,
	20,22,21,22,23,21,
};

const char* oBasisTestOBJ_Cube::sGroupNames[6] = 
{
	"Left",
	"Right",
	"Top",
	"Bottom",
	"Near",
	"Far"
};

bool oBasisTestOBJ_Cube::InitGroupsDone = false;
ouro::mesh::obj::group oBasisTestOBJ_Cube::sGroups[6];
ouro::mesh::range oBasisTestOBJ_Cube::sRanges[6];

bool oBasisTestOBJGet(oBASIS_TEST_OBJ _OBJ, const oBasisTestOBJ** _ppTestOBJ)
{
	static oBasisTestOBJ_Cube c;

	switch (_OBJ)
	{
		case oBASIS_TEST_CUBE_OBJ:
			*_ppTestOBJ = &c;
			break;
		default:
			return oErrorSetLast(std::errc::no_such_file_or_directory, "the specified test obj was not found");
	}

	return true;
}
