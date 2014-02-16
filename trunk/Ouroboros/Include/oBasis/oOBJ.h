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

// Structures and utilities for loading the Wavefront OBJ 3D object file format
#pragma once
#ifndef oOBJ_h
#define oOBJ_h

#include <oBase/invalid.h>
#include <oBase/colors.h>
#include <oBasis/oGPUConcepts.h>
#include <oBasis/oInterface.h>
#include <oCompute/oAABox.h>
#include <oCompute/rgb.h>

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

enum oOBJ_TEXTURE_TYPE
{
	oOBJ_DEFAULT,
	oOBJ_CUBE_RIGHT,
	oOBJ_CUBE_LEFT,
	oOBJ_CUBE_TOP,
	oOBJ_CUBE_BOTTOM,
	oOBJ_CUBE_BACK,
	oOBJ_CUBE_FRONT,
	oOBJ_SPHERE,
	oOBJ_TEXTURE_TYPE_COUNT,
};

struct oOBJ_GROUP
{
	ouro::mstring GroupName;
	ouro::mstring MaterialName;
	ouro::mesh::range Range;
};

struct oOBJ_TEXTURE
{
	oOBJ_TEXTURE()
		: Boost(0.0f)
		, BrightnessGain(0.0f, 1.0f)
		, OriginOffset(0.0f, 0.0f, 0.0f)
		, Scale(1.0f, 1.0f, 1.0f)
		, Turbulance(0.0f, 0.0f, 0.0f)
		, Resolution(ouro::invalid, ouro::invalid)
		, BumpMultiplier(1.0f)
		, Type(oOBJ_DEFAULT)
		, IMFChan('l')
		, Blendu(true)
		, Blendv(true)
		, Clamp(false)
	{}

	ouro::path_string Path;
	float Boost;
	float2 BrightnessGain;
	float3 OriginOffset;
	float3 Scale;
	float3 Turbulance;
	uint2 Resolution;
	float BumpMultiplier;
	oOBJ_TEXTURE_TYPE Type;
	char IMFChan;
	bool Blendu;
	bool Blendv;
	bool Clamp;
};

struct oOBJ_MATERIAL
{
	oOBJ_MATERIAL()
		: AmbientColor(ouro::black)
		, EmissiveColor(ouro::black)
		, DiffuseColor(ouro::white_smoke)
		, SpecularColor(ouro::white)
		, Specularity(0.25f)
		, Transparency(1.0f) // (1 means opaque 0 means transparent)
		, RefractionIndex(1.0f)
		, Illum(oOBJ_COLOR_ON_AMBIENT_OFF)
	{}

	rgbf AmbientColor;
	rgbf EmissiveColor;
	rgbf DiffuseColor;
	rgbf SpecularColor;
	rgbf TransmissionColor;
	float Specularity;
	float Transparency;
	float RefractionIndex;
	oOBJ_ILLUMINATION Illum;

	ouro::path_string Name;
	oOBJ_TEXTURE Ambient;
	oOBJ_TEXTURE Diffuse;
	oOBJ_TEXTURE Alpha;
	oOBJ_TEXTURE Specular;
	oOBJ_TEXTURE Bump;
};

struct oOBJ_DESC
{
	// positions/texcoords/normals will all have the same count (i.e. 
	// reduction/duplication is done internally). If there is no data for one of 
	// the channels, then the pointer will be null and pVertexElements will be 
	// collapsed (no null element between others).

	oOBJ_DESC()
		: OBJPath(nullptr)
		, MTLPath(nullptr)
		, pPositions(nullptr)
		, pNormals(nullptr)
		, pTexcoords(nullptr)
		, pIndices(nullptr)
		, pGroups(nullptr)
		, VertexLayout(ouro::mesh::layout::pos)
		, NumVertices(0)
		, NumIndices(0)
		, NumGroups(0)
	{}

	const char* OBJPath;
	const char* MTLPath;
	const float3* pPositions;
	const float3* pNormals;
	const float3* pTexcoords;
	const uint* pIndices;
	const oOBJ_GROUP* pGroups;
	ouro::mesh::layout::value VertexLayout;

	uint NumVertices;
	uint NumIndices;
	uint NumGroups;

	oAABoxf Bound;
};

interface oOBJ : oInterface
{
	virtual void GetDesc(oOBJ_DESC* _pDesc) const threadsafe = 0;
};

struct oMTL_DESC
{
	oMTL_DESC()
		: MTLPath(nullptr)
		, pMaterials(nullptr)
		, NumMaterials(0)
	{}

	const char* MTLPath;
	const oOBJ_MATERIAL* pMaterials;
	uint NumMaterials;
};

interface oMTL : oInterface
{
	virtual void GetDesc(oMTL_DESC* _pDesc) const threadsafe = 0;
};

struct oOBJ_INIT
{
	oOBJ_INIT()
		: EstimatedNumVertices(100000)
		, EstimatedNumIndices(100000)
		, FlipHandedness(false)
		, CounterClockwiseFaces(true)
		, CalcNormalsOnError(true)
		, CalcTexcoordsOnError(true)
	{}

	// Estimates are used to pre-allocate memory in order to minimize reallocs
	uint EstimatedNumVertices;
	uint EstimatedNumIndices;
	bool FlipHandedness;
	bool CounterClockwiseFaces;
	bool CalcNormalsOnError; // either for no loaded normals or degenerates
	bool CalcTexcoordsOnError; // uses LCSM if no texcoords in source
};

// oOBJ processing hashes faces for sharing of vertices, but this can result in 
// many small allocations internally that then need to be freed, which can take 
// a long time. As an optimization, client code can pass a hint in 
// _InternalReserve to make internal memory management a lot faster.
bool oOBJCreate(const char* _OBJPath, const char* _OBJString, const oOBJ_INIT& _Init, threadsafe oOBJ** _ppOBJ);
bool oMTLCreate(const char* _MTLPath, const char* _MTLString, threadsafe oMTL** _ppMTL);

// Convenience function to collapse groups into an array of ranges. If this 
// succeeds, the number of valid ranges will be Desc.NumRanges.
bool oOBJCopyRanges(ouro::mesh::range* _pDestination, size_t _NumRanges, const oOBJ_DESC& _Desc);
template<size_t size> bool oOBJCopyRanges(ouro::mesh::range (&_pDestination)[size], const oOBJ_DESC& _Desc) { return oOBJCopyRanges(_pDestination, size, _Desc); }

#endif
