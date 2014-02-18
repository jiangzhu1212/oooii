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
#include <oBasis/oOBJ.h>
#include <oBasis/oError.h>
#include <oBasis/tests/oBasisTests.h>
#include "oBasisTestCommon.h"
#include "oBasisTestOBJ.h"
#include <oBase/finally.h>
#include <oBase/timer.h>
#include <oCompute/linear_algebra.h>

using namespace ouro;

static bool TestCorrectness(const oBasisTestOBJ* _pExpected, const std::shared_ptr<ouro::mesh::obj::mesh>& _OBJ)
{
	ouro::mesh::obj::info Expected = _pExpected->get_info();

	ouro::mesh::obj::info objInfo = _OBJ->get_info();

	oTESTB(!strcmp(Expected.mtl_path, objInfo.mtl_path), "MaterialLibraryPath \"%s\" (should be %s) does not match in obj file \"%s\"", objInfo.mtl_path.c_str(), Expected.mtl_path.c_str(), objInfo.obj_path.c_str());

	oTESTB(Expected.mesh_info.num_vertices == objInfo.mesh_info.num_vertices, "Position counts do not match in obj file \"%s\"", objInfo.obj_path.c_str());
	for (uint i = 0; i < objInfo.mesh_info.num_vertices; i++)
	{
		oTESTB(equal(Expected.data.positionsf[i], objInfo.data.positionsf[i]), "Position %u does not match in obj file \"%s\"", i, objInfo.obj_path.c_str());
		oTESTB(equal(Expected.data.normalsf[i], objInfo.data.normalsf[i]), "Normal %u does not match in obj file \"%s\"", i, objInfo.obj_path.c_str());
		oTESTB(equal(Expected.data.uvw0sf[i], objInfo.data.uvw0sf[i]), "Texcoord %u does not match in obj file \"%s\"", i, objInfo.obj_path.c_str());
	}
	
	oTESTB(Expected.mesh_info.num_indices == objInfo.mesh_info.num_indices, "Index counts do not match in obj file \"%s\"", objInfo.obj_path.c_str());
	for (uint i = 0; i < objInfo.mesh_info.num_indices; i++)
		oTESTB(equal(Expected.data.indicesi[i], objInfo.data.indicesi[i]), "Index %u does not match in obj file \"%s\"", i, objInfo.obj_path.c_str());

	oTESTB(Expected.mesh_info.num_ranges == objInfo.mesh_info.num_ranges, "Group counts do not match in obj file \"%s\"", objInfo.obj_path.c_str());
	for (uint i = 0; i < objInfo.mesh_info.num_ranges; i++)
	{
		oTESTB(!strcmp(Expected.groups[i].group_name.c_str(), objInfo.groups[i].group_name.c_str()), "Group %u does not match in obj file \"%s\"", i, objInfo.obj_path.c_str());
		oTESTB(!strcmp(Expected.groups[i].material_name.c_str(), objInfo.groups[i].material_name.c_str()), "Group %u does not match in obj file \"%s\"", i, objInfo.obj_path.c_str());
		oTESTB(Expected.data.ranges[i].start_primitive == objInfo.data.ranges[i].start_primitive, "Group %u does not match in obj file \"%s\"", i, objInfo.obj_path.c_str());
		oTESTB(Expected.data.ranges[i].num_primitives == objInfo.data.ranges[i].num_primitives, "Group %u does not match in obj file \"%s\"", i, objInfo.obj_path.c_str());
	}

	oErrorSetLast(0);
	return true;
}

static bool oBasisTest_oOBJLoad(const oBasisTestServices& _Services, const char* _Path, double* _pLoadTime = nullptr)
{
	path path;
	oTESTB(_Services.ResolvePath(path, _Path, true), "not found: %s", _Path);

	char* pOBJBuffer = nullptr;
	size_t Size = 0;
	double start = ouro::timer::now();
	if (!_Services.AllocateAndLoadBuffer((void**)&pOBJBuffer, &Size, path, true))
		return false;

	finally FreeBuffer([&] { _Services.DeallocateLoadedBuffer(pOBJBuffer); });
		
	ouro::mesh::obj::init init;
	init.calc_normals_on_error = false; // buddha doesn't have normals and is 300k faces... let's not sit in the test suite calculating such a large test case
	std::shared_ptr<ouro::mesh::obj::mesh> obj = ouro::mesh::obj::mesh::make(init, path, pOBJBuffer);

	if (_pLoadTime)
		*_pLoadTime = ouro::timer::now() - start;

	return true;
}

bool oBasisTest_oOBJ(const oBasisTestServices& _Services)
{
	path_string path;

	// Correctness
	{
		const oBasisTestOBJ* pCube = nullptr;
		oBasisTestOBJGet(oBASIS_TEST_CUBE_OBJ, &pCube);

		std::shared_ptr<ouro::mesh::obj::mesh> obj = ouro::mesh::obj::mesh::make(ouro::mesh::obj::init(), "Correctness (cube) obj", pCube->GetFileContents());
		oTESTB0(TestCorrectness(pCube, obj));
	}

	// Support for negative indices
	{
		static const char* NegIndicesFilename = "Test/Geometry/hunter.obj";
		oTESTB0(oBasisTest_oOBJLoad(_Services, NegIndicesFilename));
	}

	// Performance
	{
		static const char* BenchmarkFilename = "Test/Geometry/buddha.obj";
		double LoadTime = 0.0;
		oTESTB0(oBasisTest_oOBJLoad(_Services, BenchmarkFilename, &LoadTime));

		sstring time;
		format_duration(time, LoadTime, true);
		oErrorSetLast(0, "%s to load benchmark file %s", time.c_str(), BenchmarkFilename);
	}
	
	return true;
}
