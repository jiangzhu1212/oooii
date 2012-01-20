/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#include <oBasis/oOBJLoader.h>
#include <oBasis/oError.h>
#include <oBasis/oOnScopeExit.h>
#include <oBasis/oPath.h>
#include <oBasis/oString.h>
#include <oBasis/oTimer.h>
#include <oBasisTests/oBasisTests.h>
#include "oBasisTestCommon.h"

static const char* CorrectnessTestOBJ =
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

static void ConstructGroup(const char* _GroupName, const char* _MaterialName, unsigned int _StartIndex, unsigned int _NumIndices, oOBJ::GROUP* _pGroup)
{
	_pGroup->GroupName = _GroupName;
	_pGroup->MaterialName = _MaterialName;
	_pGroup->StartIndex = _StartIndex;
	_pGroup->NumIndices = _NumIndices;
}

static bool TestCorrectness(const oOBJ& _OBJ)
{
	// This function assumes exact topology of the specified correctness obj for
	// testing, so if the file changes, this code would have to as well.

	static const float3 sExpectedVertices[] = 
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

	static const float3 sExpectedNormals[] = 
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

	static const float2 sExpectedTexcoords[] = 
	{
		float2(0.000000f, 1.000000f),
		float2(1.000000f, 1.000000f),
		float2(0.000000f, 0.000000f),
		float2(1.000000f, 0.000000f),
		float2(1.000000f, 0.000000f),
		float2(1.000000f, 1.000000f),
		float2(0.000000f, 1.000000f),
		float2(0.000000f, 0.000000f),
		float2(0.000000f, 0.000000f),
		float2(1.000000f, 1.000000f),
		float2(0.000000f, 1.000000f),
		float2(1.000000f, 0.000000f),
		float2(0.000000f, 1.000000f),
		float2(0.000000f, 0.000000f),
		float2(1.000000f, 0.000000f),
		float2(1.000000f, 1.000000f),
		float2(0.000000f, 0.000000f),
		float2(1.000000f, 0.000000f),
		float2(0.000000f, 1.000000f),
		float2(1.000000f, 1.000000f),
		float2(1.000000f, 0.000000f),
		float2(1.000000f, 1.000000f),
		float2(0.000000f, 0.000000f),
		float2(0.000000f, 1.000000f),
	};

	static const unsigned int sExpectedIndices[] = { 0,2,1,3,0,1,4,6,5,7,6,4,8,10,9,8,9,11,12,14,13,15,14,12,16,18,17,17,18,19,20,22,21,22,23,21, };

	for (size_t i = 0; i < _OBJ.Positions.size(); i++)
		oTESTB(oEqual(sExpectedVertices[i], _OBJ.Positions[i]), "Position %u does not match in obj file \"%s\"", i, _OBJ.OBJPath.c_str());
			
	for (size_t i = 0; i < oCOUNTOF(sExpectedNormals); i++)
		oTESTB(oEqual(sExpectedNormals[i], _OBJ.Normals[i]), "Normal %u does not match in obj file \"%s\"", i, _OBJ.OBJPath.c_str());

	for (size_t i = 0; i < oCOUNTOF(sExpectedVertices); i++)
		oTESTB(oEqual(sExpectedVertices[i], _OBJ.Positions[i]), "Texcoord %u does not match in obj file \"%s\"", i, _OBJ.OBJPath.c_str());

	for (size_t i = 0; i < oCOUNTOF(sExpectedIndices); i++)
		oTESTB(oEqual(sExpectedIndices[i], _OBJ.Indices[i]), "Index %u does not match in obj file \"%s\"", i, _OBJ.OBJPath.c_str());

	oTESTB(!strcmp(_OBJ.MaterialLibraryPath.c_str(), "cube.mtl"), "MaterialLibraryPath \"%s\" (should be cube.mtl) does not match in obj file \"%s\"", _OBJ.MaterialLibraryPath.c_str(), _OBJ.OBJPath.c_str());

	// @oooii-tony: This reports a leak because of std::string implicit construction at static init time,
	// I'm going to convert all this code, so live with this in my branch for now...
	oOBJ::GROUP sExpectedGroups[6];
	ConstructGroup("Left", "Body", 0, 6, &sExpectedGroups[0]);
	ConstructGroup("Right", "Body", 6, 6, &sExpectedGroups[1]);
	ConstructGroup("Top", "Body", 12, 6, &sExpectedGroups[2]);
	ConstructGroup("Bottom", "Body", 18, 6, &sExpectedGroups[3]);
	ConstructGroup("Near", "Body", 24, 6, &sExpectedGroups[4]);
	ConstructGroup("Far", "Body", 30, 6, &sExpectedGroups[5]);

	for (size_t i = 0; i < oCOUNTOF(sExpectedGroups); i++)
	{
		oTESTB(!strcmp(sExpectedGroups[i].GroupName.c_str(), _OBJ.Groups[i].GroupName.c_str()), "Group %u does not match in obj file \"%s\"", i, _OBJ.OBJPath.c_str());
		oTESTB(!strcmp(sExpectedGroups[i].MaterialName.c_str(), _OBJ.Groups[i].MaterialName.c_str()), "Group %u does not match in obj file \"%s\"", i, _OBJ.OBJPath.c_str());
		oTESTB(sExpectedGroups[i].StartIndex == _OBJ.Groups[i].StartIndex, "Group %u does not match in obj file \"%s\"", i, _OBJ.OBJPath.c_str());
		oTESTB(sExpectedGroups[i].NumIndices == _OBJ.Groups[i].NumIndices, "Group %u does not match in obj file \"%s\"", i, _OBJ.OBJPath.c_str());
	}

	return true;
}

bool oBasisTest_oOBJLoader(const oBasisTestServices& _Services)
{
	oStringPath path;

	// Correctness
	{
		oOBJ obj;
		oTESTB(oOBJLoad("Correctness (cube) obj", CorrectnessTestOBJ, false, oKB(100), &obj), "Failed to parse correctness (cube) obj file");
		oTESTB(TestCorrectness(obj), "Incorrect parsing for correctness (cube) obj file");
	}

	// Performance
	{
		static const char* BenchmarkFilename = "Test/Geometry/buddha.obj";
		oTESTB(_Services.ResolvePath(path.c_str(), path.capacity(), BenchmarkFilename, true), "not found: %s", BenchmarkFilename);
		oTESTB(oCleanPath(path.c_str(), path.capacity(), path), "Failed to clean path on \"%s\"", path);

		oOBJ obj;
		char* pOBJBuffer = nullptr;
		size_t Size = 0;
		double start = oTimer();
		oTESTB(_Services.AllocateAndLoadBuffer((void**)&pOBJBuffer, &Size, path, true), "Failed to load file \"%s\"", path);
		oOnScopeExit FreeBuffer([&] { _Services.DeallocateLoadedBuffer(pOBJBuffer); });
		oTESTB(oOBJLoad(path, pOBJBuffer, false, oMB(20), &obj), "Failed to parse obj file \"%s\"", path);
		char time[64];
		oFormatTimeSize(time, oTimer() - start, true);
		oErrorSetLast(oERROR_NONE, "%s to load benchmark file %s", time, BenchmarkFilename);
	}
	
	return true;
}
