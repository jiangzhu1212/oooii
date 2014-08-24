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
#include <oMesh/obj.h>
#include <oBase/finally.h>
#include <oBase/throw.h>
#include <oBase/timer.h>

#include "../../test_services.h"

//#include <oBasis/tests/oBasisTests.h>
//#include "oBasisTestCommon.h"
#include "obj_test.h"
//#include <oBase/finally.h>
//#include <oCompute/linear_algebra.h>

namespace ouro {
	namespace tests {

static void test_correctness(const std::shared_ptr<obj_test>& _Expected, const std::shared_ptr<mesh::obj::mesh>& _OBJ)
{
	mesh::obj::info expectedInfo = _Expected->get_info();
	mesh::obj::info objInfo = _OBJ->get_info();

	oCHECK(!strcmp(expectedInfo.mtl_path, objInfo.mtl_path), "MaterialLibraryPath \"%s\" (should be %s) does not match in obj file \"%s\"", objInfo.mtl_path.c_str(), expectedInfo.mtl_path.c_str(), objInfo.obj_path.c_str());
	
	oCHECK(expectedInfo.mesh_info.num_vertices == objInfo.mesh_info.num_vertices, "Position counts do not match in obj file \"%s\"", objInfo.obj_path.c_str());
	for (uint i = 0; i < objInfo.mesh_info.num_vertices; i++)
	{
		oCHECK(equal(expectedInfo.positions[i], objInfo.positions[i]), "Position %u does not match in obj file \"%s\"", i, objInfo.obj_path.c_str());
		oCHECK(equal(expectedInfo.normals[i], objInfo.normals[i]), "Normal %u does not match in obj file \"%s\"", i, objInfo.obj_path.c_str());
		oCHECK(equal(expectedInfo.texcoords[i], objInfo.texcoords[i]), "Texcoord %u does not match in obj file \"%s\"", i, objInfo.obj_path.c_str());
	}
	
	oCHECK(expectedInfo.mesh_info.num_indices == objInfo.mesh_info.num_indices, "Index counts do not match in obj file \"%s\"", objInfo.obj_path.c_str());
	for (uint i = 0; i < objInfo.mesh_info.num_indices; i++)
		oCHECK(equal(expectedInfo.indices[i], objInfo.indices[i]), "Index %u does not match in obj file \"%s\"", i, objInfo.obj_path.c_str());

	oCHECK(expectedInfo.mesh_info.num_ranges == objInfo.mesh_info.num_ranges, "Group counts do not match in obj file \"%s\"", objInfo.obj_path.c_str());
	for (uint i = 0; i < objInfo.mesh_info.num_ranges; i++)
	{
		oCHECK(!strcmp(expectedInfo.groups[i].group_name.c_str(), objInfo.groups[i].group_name.c_str()), "Group %u does not match in obj file \"%s\"", i, objInfo.obj_path.c_str());
		oCHECK(!strcmp(expectedInfo.groups[i].material_name.c_str(), objInfo.groups[i].material_name.c_str()), "Group %u does not match in obj file \"%s\"", i, objInfo.obj_path.c_str());
		oCHECK(expectedInfo.ranges[i].start_primitive == objInfo.ranges[i].start_primitive, "Group %u does not match in obj file \"%s\"", i, objInfo.obj_path.c_str());
		oCHECK(expectedInfo.ranges[i].num_primitives == objInfo.ranges[i].num_primitives, "Group %u does not match in obj file \"%s\"", i, objInfo.obj_path.c_str());
	}
}

static void obj_load(test_services& _Services, const char* _Path, double* _pLoadTime = nullptr)
{
	double start = timer::now();
	scoped_allocation b = _Services.load_buffer(_Path);

	mesh::obj::init init;
	init.calc_normals_on_error = false; // buddha doesn't have normals and is 300k faces... let's not sit in the test suite calculating such a large test case
	std::shared_ptr<mesh::obj::mesh> obj = mesh::obj::mesh::make(init, _Path, b);

	if (_pLoadTime)
		*_pLoadTime = timer::now() - start;
}

void TESTobj(test_services& _Services)
{
	// Correctness
	{
		std::shared_ptr<obj_test> test = obj_test::make(obj_test::cube);
		std::shared_ptr<mesh::obj::mesh> obj = mesh::obj::mesh::make(mesh::obj::init(), "Correctness (cube) obj", test->file_contents());
		test_correctness(test, obj);
	}

	// Support for negative indices
	{
		obj_load(_Services, "Test/Geometry/hunter.obj");
	}

	// Performance
	{
		static const char* BenchmarkFilename = "Test/Geometry/buddha.obj";
		double LoadTime = 0.0;
		obj_load(_Services, BenchmarkFilename, &LoadTime);

		sstring time;
		format_duration(time, LoadTime, true);
		_Services.report("%s to load benchmark file %s", time.c_str(), BenchmarkFilename);
	}
}

	} // namespace tests
} // namespace ouro
