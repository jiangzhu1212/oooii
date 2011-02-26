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
#pragma once
#ifndef geometry_h
#define geometry_h

#include <oooii/oColor.h>
#include <oooii/oMath.h>
#include <oooii/oSTL.h>

enum GEOMETRY_FACE_TYPE
{
	GEOMETRY_FACE_UNKNOWN = -1,
	GEOMETRY_FACE_SOLID_CCW,
	GEOMETRY_FACE_SOLID_CW,
	GEOMETRY_FACE_OUTLINE,
	NUM_GEOMETRY_FACE_TYPES,
};

enum GEOMETRY_PRIMITIVE_TYPE
{
	GEOMETRY_PRIMITIVE_UNKNOWN = -1,
	GEOMETRY_PRIMITIVE_POINTLIST,
	GEOMETRY_PRIMITIVE_LINELIST,
	GEOMETRY_PRIMITIVE_LINESTRIP,
	GEOMETRY_PRIMITIVE_TRILIST,
	GEOMETRY_PRIMITIVE_TRISTRIP,
	NUM_GEOMETRY_PRIMITIVE_TYPES,
};

struct GEOMETRY_LAYOUT
{
	bool Positions;
	bool Normals;
	bool Tangents;
	bool Texcoords;
	bool Colors;
};

struct GEOMETRY_RECT_DESC
{
	GEOMETRY_FACE_TYPE FaceType;
	float Width;
	float Height;
	unsigned int DivideW;
	unsigned int DivideH;
	oColor Color;
	bool Centered;
	bool FlipTexcoordV;
};

struct GEOMETRY_BOX_DESC
{
	GEOMETRY_FACE_TYPE FaceType;
	oAABoxf Bound;
	unsigned int Divide;
	oColor Color;
	bool FlipTexcoordV;
};

struct GEOMETRY_CIRCLE_DESC
{
	GEOMETRY_FACE_TYPE FaceType;
	float Radius;
	unsigned int Facet;
	oColor Color;
};

struct GEOMETRY_SPHERE_DESC
{
	// icosahedron: T: use icosahedron subdivision F: use octahedron subdivision
	// texture coord u goes from 0 at Y=+1 to 0.25 at X=-1 to 0.5 at Y=-1 to 0.75 at X=+1 back to 1.0 at Y=+1
	// texture coord v goes from 0 at Z=+1 to 1 at Z=-1, or if hemisphere, 0 at Z=+1 and 1 at Z=0

	GEOMETRY_FACE_TYPE FaceType;
	oSpheref Bound;

	// Careful, a Divide of 6 takes ~3 sec on an overclocked i7 920. 
	// 7 Takes ~11 sec. 8, I didn't wait for it to finish.
	unsigned int Divide;

	oColor Color;
	bool Hemisphere;
	bool Icosahedron;
};

struct GEOMETRY_CYLINDER_DESC
{
	GEOMETRY_FACE_TYPE FaceType;
	unsigned int Divide;
	unsigned int Facet;
	float Radius0;
	float Radius1;
	float Height;
	oColor Color;
	bool IncludeBase;
};

struct GEOMETRY_CONE_DESC
{
	// note: if the cone is to be textured, it's probably 
	// better to use a cylinder with radius0 = 0 so that 
	// the texture coords are better distributed

	GEOMETRY_FACE_TYPE FaceType;
	unsigned int Divide;
	unsigned int Facet;
	float Radius;
	float Height;
	oColor Color;
	bool IncludeBase;
};

struct oGeometry
{
	// Intended for the internals of a tool-time or load-time
	// process. Because this is used in tessellation, all the
	// parts are exposed for easy access. This is not a 
	// structure for general runtime.

	oGeometry() { Clear(); }

	GEOMETRY_PRIMITIVE_TYPE PrimitiveType;

	std::vector<unsigned int> Indices;
	std::vector<float3> Positions;
	std::vector<float3> Normals;
	std::vector<float4> Tangents; // handedness in w component
	std::vector<float2> Texcoords;
	std::vector<oColor> Colors;
	oAABoxf Bounds;

	inline size_t GetNumVertices() const { return Positions.size(); }
	size_t GetNumPrimitives() const;
	bool IsValid() const;
	void Clear();
	void Transform(const float4x4& _Matrix);
	void CalculateBounds();
};

// _FaceNormals should be allocated to _NumberOfIndices / 3
void CalculateFaceNormals(float3* _pFaceNormals, size_t _NumberOfIndices, const unsigned int* _pIndices, size_t _NumberOfVertices, const float3* _pPositions, bool _CCW);
void CalculateVertexNormals(float3* _pVertexNormals, size_t _NumberOfIndices, const unsigned int* _pIndices, size_t _NumberOfVertices, const float3* _pPositions, bool _CCW);

// All vertex buffers should be allocated to _NumberOfVertices elements
// and the index buffer should be allocated to _NumberOfIndices
void CalculateTangents(float4* _pTangents, size_t _NumberOfIndices, const unsigned int* _pIndices, size_t _NumberOfVertices, const float3* _pPositions, const float3* _pNormals, const float2* _pTexcoords);

errno_t CreateRect(const GEOMETRY_RECT_DESC* _pDesc, const GEOMETRY_LAYOUT* _pLayout, oGeometry* _pGeometry);
errno_t CreateBox(const GEOMETRY_BOX_DESC* _pDesc, const GEOMETRY_LAYOUT* _pLayout, oGeometry* _pGeometry);
errno_t CreateCircle(const GEOMETRY_CIRCLE_DESC* _pDesc, const GEOMETRY_LAYOUT* _pLayout, oGeometry* _pGeometry);
errno_t CreateSphere(const GEOMETRY_SPHERE_DESC* _pDesc, const GEOMETRY_LAYOUT* _pLayout, oGeometry* _pGeometry);
errno_t CreateCylinder(const GEOMETRY_CYLINDER_DESC* _pDesc, const GEOMETRY_LAYOUT* _pLayout, oGeometry* _pGeometry);
errno_t CreateCone(const GEOMETRY_CONE_DESC* _pDesc, const GEOMETRY_LAYOUT* _pLayout, oGeometry* _pGeometry);
errno_t CreateGeometryFromOBJ(const char* _pOBJFileString, const GEOMETRY_LAYOUT* _pLoadLayout, oGeometry* _pGeometry);

#endif
