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

// Structures and utilities for loading the Wavefront OBJ 3D object file format
#pragma once
#ifndef oOBJ_h
#define oOBJ_h

#include <oBasis/oGPUEnums.h>

// http://local.wasp.uwa.edu.au/~pbourke/dataformats/mtl/
enum oOBJ_ILLUMINATION
{
	oOBJ_COLOR_ON_AMBIENT_OFF,
	oOBJ_COLOR_ON_AMBIENT_ON,
	oOBJ_HIGHLIGHT_ON,
	oOBJ_REFLECTION_ON_RAY_TRACE_ON,
	oOBJ_TRANSPARENCY_GLASS_ON_REFLECTION_RAY_TRACE_ON,
	oOBJ_REFLECTION_FRESNEL_ON_RAY_TRACE_ON,
	oOBJ_TRANSPARENCY_REFRACTION_ON_REFLECTION_FRESNEL_OFF_RAY_TRACE_ON,
	oOBJ_TRANSPARENCY_REFRACTION_ON_REFLECTION_FRENSEL_ON_RAY_TRACE_ON,
	oOBJ_REFLECTION_ON_RAY_TRACE_OFF,
	oOBJ_TRANSPARENCY_GLASS_ON_REFLECTION_RAY_TRACE_OFF,
	oOBJ_CASTS_SHADOWS_ONTO_INVISIBLE_SURFACES,
};

struct oOBJ_GROUP
{
	oStringM GroupName;
	oStringM MaterialName;
	oGPU_RANGE Range;
};

struct oOBJ_MATERIAL
{
	oOBJ_MATERIAL()
		: AmbientColor(std::Black)
		, DiffuseColor(std::WhiteSmoke)
		, SpecularColor(std::White)
		, Specularity(0.25f)
		, Transparency(1.0f) // (1 means opaque 0 means transparent)
		, RefractionIndex(1.0f)
		, Illum(oOBJ_COLOR_ON_AMBIENT_OFF)
	{}

	oRGBf AmbientColor;
	oRGBf DiffuseColor;
	oRGBf SpecularColor;
	oRGBf TransmissionColor;
	float Specularity;
	float Transparency;
	float RefractionIndex;
	oOBJ_ILLUMINATION Illum;

	oStringPath Name;
	oStringPath AmbientTexturePath;
	oStringPath DiffuseTexturePath;
	oStringPath AlphaTexturePath;
	oStringPath SpecularTexturePath;
	oStringPath BumpTexturePath;
};

struct oOBJ_DESC
{
	oOBJ_DESC()
		: OBJPath(nullptr)
		, MTLPath(nullptr)
		, pPositions(nullptr)
		, pNormals(nullptr)
		, pTexcoords(nullptr)
		, pIndices(nullptr)
		, pGroups(nullptr)
		, NumVertices(0)
		, NumIndices(0)
		, NumGroups(0)
	{}

	const char* OBJPath;
	const char* MTLPath;
	const float3* pPositions;
	const float3* pNormals;
	const float2* pTexcoords;
	const unsigned int* pIndices;
	const oOBJ_GROUP* pGroups;

	// positions/normals/texcoords will all have the same count. If there is no
	// data for one of the channels, then the pointer will be null
	unsigned int NumVertices;
	unsigned int NumIndices;
	unsigned int NumGroups;
};

interface oOBJ : oInterface
{
	virtual void GetDesc(oOBJ_DESC* _pDesc) const threadsafe = 0;
};

struct oMTL_DESC
{
	const oOBJ_MATERIAL* pMaterials;
	unsigned int NumMaterials;
};

interface oMTL : oInterface
{
	virtual void GetDesc(oMTL_DESC* _pDesc) const threadsafe = 0;
};

struct oOBJ_INIT
{
	oOBJ_INIT()
		: EstimatedNumVertices(100000)
		,	EstimatedNumIndices(100000)
		, FlipHandedness(false)
		, CounterClockwiseFaces(true)
		, CalcNormalsOnError(true)
	{}

	// Estimates are used to pre-allocate memory in order to minimize reallocs
	unsigned int EstimatedNumVertices;
	unsigned int EstimatedNumIndices;
	bool FlipHandedness;
	bool CounterClockwiseFaces;
	bool CalcNormalsOnError; // either no normals loaded or there are degenerate normals
};

// oOBJ processing hashes faces for sharing of vertices, but this can result in 
// many small allocations internally that then need to be freed, which can take 
// a long time. As an optimization, client code can pass a hint in 
// _InternalReserve to make internal memory management a lot faster.
bool oOBJCreate(const char* _OBJPath, const char* _OBJString, const oOBJ_INIT& _Init, threadsafe oOBJ** _ppOBJ);
bool oMTLCreate(const char* _MTLPath, const char* _MTLString, threadsafe oMTL** _ppMTL);

#endif
