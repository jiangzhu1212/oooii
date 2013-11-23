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
// Various factories for creating vertex based primitives such as cubes and 
// spheres. This is not meant to be necessarily runtime-optimal.
#pragma once
#ifndef oGeometry_h
#define oGeometry_h

#include <oBasis/oGPUConcepts.h>
#include <oBasis/oInterface.h>

// {7BA30462-0899-489a-87A8-D897D1CE929E}
oDEFINE_GUID_I(oGeometry, 0x7ba30462, 0x899, 0x489a, 0x87, 0xa8, 0xd8, 0x97, 0xd1, 0xce, 0x92, 0x9e);
interface oGeometry : oInterface
{
	// Intended for the internals of a tool-time or load-time process. Because 
	// this struct is the target of tessellation internally by the create 
	// functions, all the parts are exposed for easy access. This is not a 
	// structure for general runtime.

	enum FACE_TYPE
	{
		FRONT_CCW,
		FRONT_CW,
		OUTLINE, // not wiremesh
		NUM_FACE_TYPES,
	};

	enum PRIMITIVE_TYPE
	{
		POINTLIST,
		LINELIST,
		LINESTRIP,
		TRILIST,
		TRISTRIP,
		NUM_PRIMITIVE_TYPES,
	};

	struct LAYOUT
	{
		bool Positions;
		bool Normals;
		bool Tangents;
		bool Texcoords;
		bool Colors;

		// Per-vertex id where same ids indices 
		// curved/continuous surfaces
		bool ContinuityIDs;
	};

	struct DESC
	{
		unsigned int NumRanges;
		unsigned int NumVertices;
		unsigned int NumIndices;
		unsigned int NumPrimitives;
		FACE_TYPE FaceType;
		PRIMITIVE_TYPE PrimitiveType;
		oAABoxf Bounds;
		LAYOUT Layout;
	};

	struct MAPPED
	{
		oGPU_RANGE* pRanges;
		unsigned int* pIndices;
		float3* pPositions;
		float3* pNormals;
		float4* pTangents; // handedness in w component
		float3* pTexcoords;
		ouro::color* pColors;
	};

	struct CONST_MAPPED
	{
		const oGPU_RANGE* pRanges;
		const unsigned int* pIndices;
		const float3* pPositions;
		const float3* pNormals;
		const float4* pTangents; // handedness in w component
		const float3* pTexcoords;
		const ouro::color* pColors;
	};

	virtual void GetDesc(DESC* _pDesc) const = 0;
	virtual void Transform(const float4x4& _Matrix) = 0;

	// the MAPPED structure can have NULL values for streams that don't exist.
	virtual bool Map(MAPPED* _pMapped) = 0;
	virtual void Unmap() = 0;

	virtual bool MapConst(CONST_MAPPED* _pMapped) const = 0;
	virtual void UnmapConst() const = 0;
};

// {57FBE80E-60DC-4a7c-8ADC-6CA9B95D6366}
oDEFINE_GUID_I(oGeometryFactory, 0x57fbe80e, 0x60dc, 0x4a7c, 0x8a, 0xdc, 0x6c, 0xa9, 0xb9, 0x5d, 0x63, 0x66);
interface oGeometryFactory : oInterface
{
	struct RECT_DESC
	{
		oGeometry::FACE_TYPE FaceType;
		float Width;
		float Height;
		unsigned int Divide;
		ouro::color Color;
		bool Centered;
		bool FlipTexcoordV;
	};

	struct BOX_DESC
	{
		oGeometry::FACE_TYPE FaceType;
		oAABoxf Bounds;
		unsigned int Divide;
		ouro::color Color;
		bool FlipTexcoordV;
	};

	struct FRUSTUM_DESC
	{
		oGeometry::FACE_TYPE FaceType;
		oFrustumf Bounds;
		unsigned int Divide;
		ouro::color Color;
	};

	struct CIRCLE_DESC
	{
		oGeometry::FACE_TYPE FaceType;
		float Radius;
		unsigned int Facet;
		ouro::color Color;
	};

	struct WASHER_DESC
	{
		oGeometry::FACE_TYPE FaceType;
		float InnerRadius;
		float OuterRadius;
		unsigned int Facet;
		ouro::color Color;
	};

	struct SPHERE_DESC
	{
		// icosahedron: T: use icosahedron subdivision F: use octahedron subdivision
		// texture coord u goes from 0 at Y=+1 to 0.25 at X=-1 to 0.5 at Y=-1 to 0.75 at X=+1 back to 1.0 at Y=+1
		// texture coord v goes from 0 at Z=+1 to 1 at Z=-1, or if hemisphere, 0 at Z=+1 and 1 at Z=0

		oGeometry::FACE_TYPE FaceType;
		oSpheref Bounds;

		// Careful, a Divide of 6 takes ~3 sec on an overclocked i7 920. 
		// 7 Takes ~11 sec. 8, I didn't wait for it to finish.
		unsigned int Divide;

		ouro::color Color;
		bool Hemisphere;
		bool Icosahedron;

		unsigned int OutLineFacet;
	};

	struct CYLINDER_DESC
	{
		oGeometry::FACE_TYPE FaceType;
		unsigned int Divide;
		unsigned int Facet;
		float Radius0;
		float Radius1;
		float Height;
		ouro::color Color;
		bool IncludeBase;
		//Number of vertical lines to skip when generating an outline. 1 means skip every other line, ect.
		unsigned int OutlineVerticalSkip;
	};

	struct CONE_DESC
	{
		// note: if the cone is to be textured, it's probably 
		// better to use a cylinder with radius0 = 0 so that 
		// the texture coords are better distributed

		oGeometry::FACE_TYPE FaceType;
		unsigned int Divide;
		unsigned int Facet;
		float Radius;
		float Height;
		ouro::color Color;
		bool IncludeBase;
		//Number of vertical lines to skip when generating an outline. 1 means skip every other line, ect.
		unsigned int OutlineVerticalSkip;
	};

	struct TORUS_DESC
	{
		oGeometry::FACE_TYPE FaceType;
		unsigned int Divide;
		unsigned int Facet;
		float InnerRadius;
		float OuterRadius;
		ouro::color Color;
	};

	struct TEARDROP_DESC
	{
		oGeometry::FACE_TYPE FaceType;
		unsigned int Divide;
		unsigned int Facet;
		ouro::color Color;
	};

	struct OBJ_DESC
	{
		const char* OBJPath;
		const char* OBJString;
		float Scale;
		bool FlipFaces;
		bool RecalculateNormals;
	};

	struct MOSAIC_DESC
	{
		// Dimensions of total space in which pSourceRects map (usually the 
		// dimensions of a texture/video)
		int2 SourceSize;

		// Image space of the source (texture/video). The actual texture in memory
		// may just be a fraction of the total space in which pSourceRects map.
		// Because of complex decoding requirements and/or power-of-two textures
		// the image space of the source may extend past the SourceSize. Also if 
		// the total image space used by pSourceRects doesn't start at (0,0) or
		// doesn't reach the full SourceSize, the actual source could be allocated
		// only for the used region, of course also taking all alignments and 
		// complex decoding requirements into account.
		// So we need to adjust pSourceRects by SourceImageSpace.GetMin() and then
		// map UVs appropriately by using SourceImageSpace.GetDimensions(), which 
		// represents the actual range [0,1] in texture coordinates.
		oRECT SourceImageSpace;

		// Dimensions of total space in which pDestinationRects map (usually the 
		// size of the client area of a window or the total screen dimensions).
		int2 DestinationSize;

		// An array of source rectangles used to set up texcoords for each quad
		// subcomponent of the geometry produced by CreateMosaic().
		const oRECT* pSourceRects;

		// An array of destination rectangles used t oset up positions for each quad
		// subcomponents of the geometry produced by CreateMosaic()
		const oRECT* pDestinationRects;

		// The number of rectangles in both pSourceRects and pDestinationRects. 
		// Because sources map 1:1 to destinations, the sizes of the arrays must be 
		// the same as described by NumRectangles. If either array is nullptr, this
		// will be ignored and a count of 1 will be used as a default all-of-screen 
		// rectangle will be calculated internally.
		int NumRectangles;

		oGeometry::FACE_TYPE FaceType;
		float ZPosition;

		bool FlipTexcoordV;
		
		// If true 0,0 position is at top-left which is more like how screen space 
		// is described
		bool FlipPositionY;
	};

	virtual bool CreateRect(const RECT_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) = 0;
	virtual bool CreateBox(const BOX_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) = 0;
	virtual bool CreateFrustum(const FRUSTUM_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) = 0;
	virtual bool CreateCircle(const CIRCLE_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) = 0;
	virtual bool CreateWasher(const WASHER_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) = 0;
	virtual bool CreateSphere(const SPHERE_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) = 0;
	virtual bool CreateCylinder(const CYLINDER_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) = 0;
	virtual bool CreateCone(const CONE_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) = 0;
	virtual bool CreateTorus(const TORUS_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) = 0;
	virtual bool CreateTeardrop(const TEARDROP_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) = 0;
	virtual bool CreateOBJ(const OBJ_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) = 0;
	virtual bool CreateMosaic(const MOSAIC_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) = 0;

	// Readied for template usage
	inline bool Create(const RECT_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) { return CreateRect(_Desc, _Layout, _ppGeometry); }
	inline bool Create(const BOX_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) { return CreateBox(_Desc, _Layout, _ppGeometry); }
	inline bool Create(const FRUSTUM_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) { return CreateFrustum(_Desc, _Layout, _ppGeometry); }
	inline bool Create(const CIRCLE_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) { return CreateCircle(_Desc, _Layout, _ppGeometry); }
	inline bool Create(const WASHER_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) { return CreateWasher(_Desc, _Layout, _ppGeometry); }
	inline bool Create(const SPHERE_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) { return CreateSphere(_Desc, _Layout, _ppGeometry); }
	inline bool Create(const CYLINDER_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) { return CreateCylinder(_Desc, _Layout, _ppGeometry); }
	inline bool Create(const CONE_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) { return CreateCone(_Desc, _Layout, _ppGeometry); }
	inline bool Create(const TORUS_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) { return CreateTorus(_Desc, _Layout, _ppGeometry); }
	inline bool Create(const TEARDROP_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) { return CreateTeardrop(_Desc, _Layout, _ppGeometry); }
	inline bool Create(const OBJ_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) { return CreateOBJ(_Desc, _Layout, _ppGeometry); }
	inline bool Create(const MOSAIC_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) { return CreateMosaic(_Desc, _Layout, _ppGeometry); }
};

oAPI bool oGeometryFactoryCreate(oGeometryFactory** _ppGeometryFactory);

#endif
