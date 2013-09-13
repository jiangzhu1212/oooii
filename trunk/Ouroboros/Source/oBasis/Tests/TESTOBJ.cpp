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
#include <oStd/finally.h>
#include <oBasis/oPath.h>
#include <oBasis/oString.h>
#include <oBasis/oTimer.h>
#include <oBasis/tests/oBasisTests.h>
#include "oBasisTestCommon.h"
#include "oBasisTestOBJ.h"

static bool TestCorrectness(const threadsafe oBasisTestOBJ* _pExpected, const threadsafe oOBJ* _pOBJ)
{
	oOBJ_DESC Expected;
	_pExpected->GetDesc(&Expected);

	oOBJ_DESC d;
	_pOBJ->GetDesc(&d);

	oTESTB(!oStrcmp(Expected.MTLPath, d.MTLPath), "MaterialLibraryPath \"%s\" (should be %s) does not match in obj file \"%s\"", d.MTLPath, Expected.MTLPath, d.OBJPath);

	oTESTB(Expected.NumVertices == d.NumVertices, "Position counts do not match in obj file \"%s\"", d.OBJPath);
	for (uint i = 0; i < d.NumVertices; i++)
	{
		oTESTB(oStd::equal(Expected.pPositions[i], d.pPositions[i]), "Position %u does not match in obj file \"%s\"", i, d.OBJPath);
		oTESTB(oStd::equal(Expected.pTexcoords[i], d.pTexcoords[i]), "Texcoord %u does not match in obj file \"%s\"", i, d.OBJPath);
		oTESTB(oStd::equal(Expected.pNormals[i], d.pNormals[i]), "Normal %u does not match in obj file \"%s\"", i, d.OBJPath);
	}
	
	oTESTB(Expected.NumIndices == d.NumIndices, "Index counts do not match in obj file \"%s\"", d.OBJPath);
	for (uint i = 0; i < d.NumIndices; i++)
		oTESTB(oStd::equal(Expected.pIndices[i], d.pIndices[i]), "Index %u does not match in obj file \"%s\"", i, d.OBJPath);

	oTESTB(Expected.NumGroups == d.NumGroups, "Group counts do not match in obj file \"%s\"", d.OBJPath);
	for (uint i = 0; i < d.NumGroups; i++)
	{
		oTESTB(!oStrcmp(Expected.pGroups[i].GroupName.c_str(), d.pGroups[i].GroupName.c_str()), "Group %u does not match in obj file \"%s\"", i, d.OBJPath);
		oTESTB(!oStrcmp(Expected.pGroups[i].MaterialName.c_str(), d.pGroups[i].MaterialName.c_str()), "Group %u does not match in obj file \"%s\"", i, d.OBJPath);
		oTESTB(Expected.pGroups[i].Range.StartPrimitive == d.pGroups[i].Range.StartPrimitive, "Group %u does not match in obj file \"%s\"", i, d.OBJPath);
		oTESTB(Expected.pGroups[i].Range.NumPrimitives == d.pGroups[i].Range.NumPrimitives, "Group %u does not match in obj file \"%s\"", i, d.OBJPath);
	}

	oErrorSetLast(0);
	return true;
}

static bool oBasisTest_oOBJLoad(const oBasisTestServices& _Services, const char* _Path, double* _pLoadTime = nullptr)
{
	oStd::path path;
	oTESTB(_Services.ResolvePath(path, _Path, true), "not found: %s", _Path);

	oStd::ref<threadsafe oOBJ> obj;
	char* pOBJBuffer = nullptr;
	size_t Size = 0;
	double start = oTimer();
	if (!_Services.AllocateAndLoadBuffer((void**)&pOBJBuffer, &Size, path, true))
		return false;

	oStd::finally FreeBuffer([&] { _Services.DeallocateLoadedBuffer(pOBJBuffer); });
		
	oOBJ_INIT init;
	init.CalcNormalsOnError = false; // buddha doesn't have normals and is 300k faces... let's not sit in the test suite calculating such a large test case
	if (!oOBJCreate(path, pOBJBuffer, init, &obj))
		return false;

	if (_pLoadTime)
		*_pLoadTime = oTimer() - start;

	return true;
}

bool oBasisTest_oOBJ(const oBasisTestServices& _Services)
{
	oStd::path_string path;

	// Correctness
	{
		const oBasisTestOBJ* pCube = nullptr;
		oBasisTestOBJGet(oBASIS_TEST_CUBE_OBJ, &pCube);

		oStd::ref<threadsafe oOBJ> obj;
		oTESTB(oOBJCreate("Correctness (cube) obj", pCube->GetFileContents(), oOBJ_INIT(), &obj), "Failed to parse correctness (cube) obj file");
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

		oStd::sstring time;
		oStd::format_duration(time, LoadTime, true);
		oErrorSetLast(0, "%s to load benchmark file %s", time.c_str(), BenchmarkFilename);
	}
	
	return true;
}
