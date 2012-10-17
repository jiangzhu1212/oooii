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
// This header contains generic enums and utility code for working with graphics
// (3D rendering, CG, etc.) concepts. This has been put in oBasis to enable 
// generic code such as file formats and parsing and geometry synthesis to use a 
// common vocabulary, but this is also intended to be the primary API around a 
// cross-platform interface and uses in platform-specific implementations.

#pragma once
#ifndef oGPUEnums_h
#define oGPUEnums_h

#include <oBasis/oStddef.h>
#include <oBasis/oSurface.h>

#define oGPU_TRAIT_RESOURCE_READBACK 0x00000001
#define oGPU_TRAIT_RESOURCE_UNORDERED ( oGPU_TRAIT_RESOURCE_READBACK << 1 )

#define oGPU_TRAIT_TEXTURE_1D ( oGPU_TRAIT_RESOURCE_UNORDERED << 1 )
#define oGPU_TRAIT_TEXTURE_2D ( oGPU_TRAIT_TEXTURE_1D << 1 )
#define oGPU_TRAIT_TEXTURE_3D ( oGPU_TRAIT_TEXTURE_2D << 1 )
#define oGPU_TRAIT_TEXTURE_CUBE ( oGPU_TRAIT_TEXTURE_3D << 1 )
#define oGPU_TRAIT_TEXTURE_MIPS ( oGPU_TRAIT_TEXTURE_CUBE << 1 )
#define oGPU_TRAIT_TEXTURE_RENDER_TARGET ( oGPU_TRAIT_TEXTURE_MIPS << 1 )

enum oGPU_API
{
	oGPU_API_UNKNOWN,
	oGPU_API_D3D,
	oGPU_API_OGL,
	oGPU_API_OGLES,
	oGPU_API_WEBGL,
	oGPU_API_CUSTOM,
};

enum oGPU_VENDOR
{
	oGPU_VENDOR_UNKNOWN,
	oGPU_VENDOR_NVIDIA,
	oGPU_VENDOR_AMD,
	oGPU_VENDOR_INTEL,
	oGPU_VENDOR_ARM,
	oGPU_VENDOR_CUSTOM,
	oGPU_VENDOR_INTERNAL,
};

enum oGPU_PIPELINE_STAGE
{
	oGPU_VERTEX_SHADER,
	oGPU_HULL_SHADER,
	oGPU_DOMAIN_SHADER,
	oGPU_GEOMETRY_SHADER,
	oGPU_PIXEL_SHADER,

	oGPU_PIPELINE_NUM_STAGES,
};

enum oGPU_RESOURCE_TYPE
{
	oGPU_BUFFER, // a generic read-only, write-only or read-write buffer that is not a texture, index buffer or vertex buffer (constant buffer)
	oGPU_INSTANCE_LIST, // batched set of per-draw parameters sent to hardware to redraw the same mesh with these several parameters
	oGPU_LINE_LIST, // list of lines for drawing as lines
	oGPU_MATERIAL, // a collection of the textures and constants used to color a triangle
	oGPU_MATERIAL_SET, // a collection of named materials that maps to a mesh
	oGPU_MESH, // description of a geometry using index and vertex buffers to form triangles
	oGPU_OUTPUT, // receives final rendering output and channels it to the final presentation medium (often a swap chain -> window mapping)
	oGPU_SCENE, // container that stores all items on which a frustum cull is applied.
	oGPU_SKELETON, // for vertex blending (skinning), the mapping from animation to vertex
	oGPU_SKELETON_POSE, // the result of an animation as it is applied to an associated skeleton
	oGPU_TEXTURE_RGB, // texture whose colors are in RGB color spaces
	oGPU_TEXTURE_YUV, // texture whose colors are in YUV color spaces

	oGPU_RESOURCE_NUM_TYPES,
};

enum oGPU_RESIDENCY
{
	oGPU_UNINITIALIZED,
	oGPU_LOADING,
	oGPU_LOAD_FAILED,
	oGPU_CONDENSING,
	oGPU_CONDENSE_FAILED,
	oGPU_MAPPED,
	oGPU_RESIDENT,

	oGPU_NUM_RESIDENCIES,
};

enum oGPU_MESH_SUBRESOURCE
{
	oGPU_MESH_RANGES,
	oGPU_MESH_INDICES,
	oGPU_MESH_VERTICES0,
	oGPU_MESH_VERTICES1,
	oGPU_MESH_VERTICES2,

	oGPU_MESH_NUM_SUBRESOURCES,
};

enum oGPU_BUFFER_TYPE
{
	// Binding fit for rasterization HW. Using this requires a structure byte size
	// to be specified.
	oGPU_BUFFER_DEFAULT, 

	// Buffer is for resolving GPU resources so they can be copied back to CPU for 
	// reading.
	oGPU_BUFFER_READBACK,

	// Buffer that does not guarantee order of access. Such ordering must be done
	// by the explicit use of atomics in client shader code. Using this requires
	// an oSURFACE_FORMAT to be specified.
	oGPU_BUFFER_UNORDERED_UNSTRUCTURED,

	// Buffer that does not guarantee order of access. Such ordering must be done
	// by the explicit use of atomics in client shader code. Using this requires
	// a structure byte size to be specified.
	oGPU_BUFFER_UNORDERED_STRUCTURED,

	// Like UNORDERED_STRUCTURED, but also support extra bookkeeping to enable 
	// atomic append/consume. Using this requires a structure byte size to be 
	// specified.
	oGPU_BUFFER_UNORDERED_STRUCTURED_APPEND, 

	// Like UNORDERED_STRUCTURED, but also support extra bookkeeping to enable 
	// atomic increment/decrement. Using this requires a structure byte size to be 
	// specified.
	oGPU_BUFFER_UNORDERED_STRUCTURED_COUNTER,
};

enum oGPU_TEXTURE_TYPE
{
	// "normal" 2D texture.
	oGPU_TEXTURE_2D_MAP = oGPU_TRAIT_TEXTURE_2D,
	oGPU_TEXTURE_2D_MAP_MIPS = oGPU_TRAIT_TEXTURE_2D | oGPU_TRAIT_TEXTURE_MIPS,
	oGPU_TEXTURE_2D_RENDER_TARGET = oGPU_TRAIT_TEXTURE_2D | oGPU_TRAIT_TEXTURE_RENDER_TARGET,
	oGPU_TEXTURE_2D_RENDER_TARGET_MIPS = oGPU_TRAIT_TEXTURE_2D | oGPU_TRAIT_TEXTURE_MIPS | oGPU_TRAIT_TEXTURE_RENDER_TARGET,
	oGPU_TEXTURE_2D_READBACK = oGPU_TRAIT_TEXTURE_2D | oGPU_TRAIT_RESOURCE_READBACK,
	oGPU_TEXTURE_2D_READBACK_MIPS = oGPU_TRAIT_TEXTURE_2D | oGPU_TRAIT_TEXTURE_MIPS | oGPU_TRAIT_RESOURCE_READBACK,

	// a "normal" 2D texture, no mips, configured for unordered access. Currently
	// all GPGPU access to such buffers are one subresource at a time, so there is 
	// no spec that describes unordered access to arbitrary mipped memory.
	oGPU_TEXTURE_2D_MAP_UNORDERED = oGPU_TRAIT_TEXTURE_2D | oGPU_TRAIT_RESOURCE_UNORDERED,
	
	// 6-sided texture array.
	oGPU_TEXTURE_CUBE_MAP = oGPU_TRAIT_TEXTURE_CUBE,
	oGPU_TEXTURE_CUBE_MAP_MIPS = oGPU_TRAIT_TEXTURE_CUBE | oGPU_TRAIT_TEXTURE_MIPS,
	oGPU_TEXTURE_CUBE_RENDER_TARGET = oGPU_TRAIT_TEXTURE_CUBE | oGPU_TRAIT_TEXTURE_RENDER_TARGET,
	oGPU_TEXTURE_CUBE_RENDER_TARGET_MIPS = oGPU_TRAIT_TEXTURE_CUBE | oGPU_TRAIT_TEXTURE_MIPS | oGPU_TRAIT_TEXTURE_RENDER_TARGET,
	oGPU_TEXTURE_CUBE_READBACK = oGPU_TRAIT_TEXTURE_CUBE | oGPU_TRAIT_RESOURCE_READBACK,
	oGPU_TEXTURE_CUBE_READBACK_MIPS = oGPU_TRAIT_TEXTURE_CUBE | oGPU_TRAIT_TEXTURE_MIPS | oGPU_TRAIT_RESOURCE_READBACK,

	// Series of 2D slices sampled as a volume
	oGPU_TEXTURE_3D_MAP = oGPU_TRAIT_TEXTURE_3D,
	oGPU_TEXTURE_3D_MAP_MIPS = oGPU_TRAIT_TEXTURE_3D | oGPU_TRAIT_TEXTURE_MIPS,
	oGPU_TEXTURE_3D_RENDER_TARGET = oGPU_TRAIT_TEXTURE_3D | oGPU_TRAIT_TEXTURE_RENDER_TARGET,
	oGPU_TEXTURE_3D_RENDER_TARGET_MIPS = oGPU_TRAIT_TEXTURE_3D | oGPU_TRAIT_TEXTURE_MIPS | oGPU_TRAIT_TEXTURE_RENDER_TARGET,
	oGPU_TEXTURE_3D_READBACK = oGPU_TRAIT_TEXTURE_3D | oGPU_TRAIT_RESOURCE_READBACK,
	oGPU_TEXTURE_3D_READBACK_MIPS = oGPU_TRAIT_TEXTURE_3D | oGPU_TRAIT_TEXTURE_MIPS | oGPU_TRAIT_RESOURCE_READBACK,
};

inline bool oGPUTextureTypeHasMips(oGPU_TEXTURE_TYPE _Type) { return 0 != ((int)_Type & oGPU_TRAIT_TEXTURE_MIPS); }
inline bool oGPUTextureTypeIsReadback(oGPU_TEXTURE_TYPE _Type) { return 0 != ((int)_Type & oGPU_TRAIT_RESOURCE_READBACK); }
inline bool oGPUTextureTypeIsRenderTarget(oGPU_TEXTURE_TYPE _Type) { return 0 != ((int)_Type & oGPU_TRAIT_TEXTURE_RENDER_TARGET); }
inline bool oGPUTextureTypeIs2DMap(oGPU_TEXTURE_TYPE _Type) { return 0 != ((int)_Type & oGPU_TRAIT_TEXTURE_2D); }
inline bool oGPUTextureTypeIsCubeMap(oGPU_TEXTURE_TYPE _Type) { return 0 != ((int)_Type & oGPU_TRAIT_TEXTURE_CUBE); }
inline bool oGPUTextureTypeIs3DMap(oGPU_TEXTURE_TYPE _Type) { return 0 != ((int)_Type & oGPU_TRAIT_TEXTURE_3D); }
inline bool oGPUTextureTypeIsUnordered(oGPU_TEXTURE_TYPE _Type) { return 0 != ((int)_Type & oGPU_TRAIT_RESOURCE_UNORDERED); }

// Returns the matching readback type for the specified type.
inline oGPU_TEXTURE_TYPE oGPUTextureTypeGetReadbackType(oGPU_TEXTURE_TYPE _Type) { return (oGPU_TEXTURE_TYPE)((int)_Type | oGPU_TRAIT_RESOURCE_READBACK); }
inline oGPU_TEXTURE_TYPE oGPUTextureTypeGetMipMapType(oGPU_TEXTURE_TYPE _Type) { return (oGPU_TEXTURE_TYPE)((int)_Type | oGPU_TRAIT_TEXTURE_MIPS); }
inline oGPU_TEXTURE_TYPE oGPUTextureTypeGetRenderTargetType(oGPU_TEXTURE_TYPE _Type) { return (oGPU_TEXTURE_TYPE)((int)_Type | oGPU_TRAIT_TEXTURE_RENDER_TARGET); }

enum oGPU_CUBE_FACE
{
	oGPU_CUBE_POS_X,
	oGPU_CUBE_NEG_X,
	oGPU_CUBE_POS_Y,
	oGPU_CUBE_NEG_Y,
	oGPU_CUBE_POS_Z,
	oGPU_CUBE_NEG_Z,

	oGPU_CUBE_NUM_FACES,
};

enum oGPU_SURFACE_STATE
{
	// Front-facing is clockwise winding order. Back-facing is counter-clockwise.

	oGPU_FRONT_FACE, // Draws all faces whose normal points towards the viewer
	oGPU_BACK_FACE,  // Draws all faces whose normal points away from the viewer
	oGPU_TWO_SIDED, // Draws all faces
	oGPU_FRONT_WIREFRAME, // Draws the borders of all faces whose normal points towards the viewer
	oGPU_BACK_WIREFRAME,  // Draws the borders of all faces whose normal points away from the viewer
	oGPU_TWO_SIDED_WIREFRAME, // Draws the borders of all faces
	oGPU_FRONT_POINTS, // Draws the corners as points of all faces whose normal points towards the viewer
	oGPU_BACK_POINTS, // Draws the corners as points of all faces whose normal points away from the viewer
	oGPU_TWO_SIDED_POINTS, // Draws the corners as points of all faces

	oGPU_SURFACE_NUM_STATES,
};

enum oGPU_DEPTH_STENCIL_STATE
{
	oGPU_DEPTH_STENCIL_NONE, // No depth or stencil operation
	oGPU_DEPTH_TEST_AND_WRITE, // normal z-buffering mode where if occluded, exit else render and write new Z value. No stencil operation
	oGPU_DEPTH_TEST, // test depth only, do not write. No stencil operation
	
	oGPU_DEPTH_STENCIL_NUM_STATES,
};

enum oGPU_BLEND_STATE
{
	// Blend mode math from http://en.wikipedia.org/wiki/Blend_modes

	oGPU_OPAQUE, // Output.rgba = Source.rgba
	oGPU_ALPHA_TEST, // Same as oGPU_OPAQUE, test is done in user code
	oGPU_ACCUMULATE, // Output.rgba = Source.rgba + Destination.rgba
	oGPU_ADDITIVE, // Output.rgb = Source.rgb * Source.a + Destination.rgb
	oGPU_MULTIPLY, // Output.rgb = Source.rgb * Destination.rgb
	oGPU_SCREEN, // Output.rgb = Source.rgb * (1 - Destination.rgb) + Destination.rgb (as reduced from webpage's 255 - [((255 - Src)*(255 - Dst))/255])
	oGPU_TRANSLUCENT, // Output.rgb = Source.rgb * Source.a + Destination.rgb * (1 - Source.a)
	oGPU_MIN, // Output.rgba = min(Source.rgba, Destination.rgba)
	oGPU_MAX, // Output.rgba = max(Source.rgba, Destination.rgba)

	oGPU_BLEND_NUM_STATES,
};

enum oGPU_SAMPLER_STATE
{
	oGPU_POINT_CLAMP, // Use 100% of the texel nearest to the sample point. If the sample is outside the texture, use an edge texel.
	oGPU_POINT_WRAP, // Use 100% of the texel nearest to the sample point. Use only the fractional part of the sample point.
	oGPU_LINEAR_CLAMP, // Trilinear sample texels around sample point. If the sample is outside the texture, use an edge texel.
	oGPU_LINEAR_WRAP, // Trilinear sample texels around sample point. Use only the fractional part of the sample point.
	oGPU_ANISO_CLAMP, // Anisotropically sample texels around sample point. If the sample is outside the texture, use an edge texel.
	oGPU_ANISO_WRAP,// Anisotropically sample texels around sample point. Use only the fractional part of the sample point.

	// Same as above, but with a mip bias that evaluates to one mip level higher
	// than that which is ideal for hardware. (sharper, more shimmer)
	oGPU_POINT_CLAMP_BIAS_UP1,
	oGPU_POINT_WRAP_BIAS_UP1,
	oGPU_LINEAR_CLAMP_BIAS_UP1,
	oGPU_LINEAR_WRAP_BIAS_UP1,
	oGPU_ANISO_CLAMP_BIAS_UP1,
	oGPU_ANISO_WRAP_BIAS_UP1,

	// Same as above, but with a mip bias that evaluates to one mip level lower
	// than that which is ideal for hardware. (blurrier, less shimmer)
	oGPU_POINT_CLAMP_BIAS_DOWN1,
	oGPU_POINT_WRAP_BIAS_DOWN1,
	oGPU_LINEAR_CLAMP_BIAS_DOWN1,
	oGPU_LINEAR_WRAP_BIAS_DOWN1,
	oGPU_ANISO_CLAMP_BIAS_DOWN1,
	oGPU_ANISO_WRAP_BIAS_DOWN1,

	// Same as above, but with a mip bias that evaluates to two mip levels higher
	// than that which is ideal for hardware. (shader, more shimmer)
	oGPU_POINT_CLAMP_BIAS_UP2,
	oGPU_POINT_WRAP_BIAS_UP2,
	oGPU_LINEAR_CLAMP_BIAS_UP2,
	oGPU_LINEAR_WRAP_BIAS_UP2,
	oGPU_ANISO_CLAMP_BIAS_UP2,
	oGPU_ANISO_WRAP_BIAS_UP2,

	// Same as above, but with a mip bias that evaluates to two mip levels lower
	// than that which is ideal for hardware. (blurrier, less shimmer)
	oGPU_POINT_CLAMP_BIAS_DOWN2,
	oGPU_POINT_WRAP_BIAS_DOWN2,
	oGPU_LINEAR_CLAMP_BIAS_DOWN2,
	oGPU_LINEAR_WRAP_BIAS_DOWN2,
	oGPU_ANISO_CLAMP_BIAS_DOWN2,
	oGPU_ANISO_WRAP_BIAS_DOWN2,

	oGPU_SAMPLER_NUM_STATES,
};

enum oGPU_CLEAR
{
	oGPU_CLEAR_DEPTH,
	oGPU_CLEAR_STENCIL,
	oGPU_CLEAR_DEPTH_STENCIL,
	oGPU_CLEAR_COLOR,
	oGPU_CLEAR_COLOR_DEPTH,
	oGPU_CLEAR_COLOR_STENCIL,
	oGPU_CLEAR_COLOR_DEPTH_STENCIL,

	oGPU_NUM_CLEARS,
};

enum oGPU_NORMAL_SPACE
{
	oGPU_NORMAL_LOCAL_SPACE,
	oGPU_NORMAL_TANGENT_SPACE,
	oGPU_NORMAL_BUMP_LOCAL_SPACE, // single-channel intensity map that is sobel-filtered
	oGPU_NORMAL_BUMP_TANGENT_SPACE, // single-channel intensity map that is sobel-filtered

	oGPU_NORMAL_NUM_SPACES,
};

enum oGPU_BRDF_MODEL
{
	oGPU_BRDF_PHONG,
	oGPU_BRDF_GOOCH,
	oGPU_BRDF_MINNAERT,
	oGPU_BRDF_GAUSSIAN,
	oGPU_BRDF_BECKMANN,
	oGPU_BRDF_HEIDRICH_SEIDEL_ANISO, 
	oGPU_BRDF_WARD_ANISO,
	oGPU_BRDF_COOK_TORRANCE,

	oGPU_BRDF_NUM_MODELS,
};

oAPI bool oFromString(oGPU_PIPELINE_STAGE* _pValue, const char* _StrSource);
oAPI bool oFromString(oGPU_RESOURCE_TYPE* _pValue, const char* _StrSource);
oAPI bool oFromString(oGPU_RESIDENCY* _pValue, const char* _StrSource);
oAPI bool oFromString(oGPU_MESH_SUBRESOURCE* _pValue, const char* _StrSource);
oAPI bool oFromString(oGPU_TEXTURE_TYPE* _pValue, const char* _StrSource);
oAPI bool oFromString(oGPU_CUBE_FACE* _pValue, const char* _StrSource);
oAPI bool oFromString(oGPU_SURFACE_STATE* _pValue, const char* _StrSource);
oAPI bool oFromString(oGPU_DEPTH_STENCIL_STATE* _pValue, const char* _StrSource);
oAPI bool oFromString(oGPU_BLEND_STATE* _pValue, const char* _StrSource);
oAPI bool oFromString(oGPU_SAMPLER_STATE* _pValue, const char* _StrSource);
oAPI bool oFromString(oGPU_CLEAR* _pValue, const char* _StrSource);
oAPI bool oFromString(oGPU_NORMAL_SPACE* _pValue, const char* _StrSource);
oAPI bool oFromString(oGPU_BRDF_MODEL* _pValue, const char* _StrSource);

oAPI const char* oAsString(oGPU_PIPELINE_STAGE _Stage);
oAPI const char* oAsString(oGPU_RESOURCE_TYPE _Type);
oAPI const char* oAsString(oGPU_RESIDENCY _Residency);
oAPI const char* oAsString(oGPU_MESH_SUBRESOURCE _MeshSubresource);
oAPI const char* oAsString(oGPU_TEXTURE_TYPE _Type);
oAPI const char* oAsString(oGPU_CUBE_FACE _Face);
oAPI const char* oAsString(oGPU_SURFACE_STATE _State);
oAPI const char* oAsString(oGPU_DEPTH_STENCIL_STATE _State);
oAPI const char* oAsString(oGPU_BLEND_STATE _State);
oAPI const char* oAsString(oGPU_SAMPLER_STATE _State);
oAPI const char* oAsString(oGPU_CLEAR _Clear);
oAPI const char* oAsString(oGPU_NORMAL_SPACE _Space);
oAPI const char* oAsString(oGPU_BRDF_MODEL _Model);

inline bool oGPUResidencyIsError(oGPU_RESIDENCY _Residency) { return _Residency == oGPU_CONDENSE_FAILED || _Residency == oGPU_LOAD_FAILED; }

// Generic description of an element in a heterogeneous (AOS) vertex.
struct oGPU_VERTEX_ELEMENT
{
	// This struct is 28 bytes, thus 4-byte aligned
	// This struct has no ctor so it can be used in const static declarations

	// Name, such as 'POS0' for position. This code should be fit for writing to 
	// disk and uniquely identifying a semantic channel and its index. The 
	// following rules should be applied for index: if the right-most char in the 
	// oFourCC is numeric, use that value, else the value is zero. ('TANG' for 
	// example would be index 0).
	oFourCC Semantic; 

	// The format of the data, i.e. a float4 would be oSURFACE_R32G32B32A32_FLOAT
	oSURFACE_FORMAT Format;

	// The input slot this will be bound too. Basically modern pipelines allow
	// several vertex arrays to be bound, so this specifies from which this 
	// element will be drawn.
	short InputSlot;

	// If false, this is per-vertex data. If true, this is per-instance data as 
	// used during instanced drawing.
	bool Instanced;
};

// Extracts the right-most part of a fourcc and interprets it as a number used
// for the semantic index of the vertex element. If the value is non-numeric 
// then the return value will be 0.
oAPI int oGPUGetSemanticIndex(const oFourCC& _FourCC);

// A mesh's vertices are generally stored together for efficiency, but are also
// organized by material or other sub-grouping. A range describes that sub-
// grouping.
struct oGPU_RANGE
{
	// This struct is 16 bytes, thus 4-byte and 8-byte aligned

	// Min/Max vertex are set to invalid values - they should be initialized by
	// meaningful values when this struct is populated.
	oGPU_RANGE(unsigned int _StartTriangle = 0, unsigned int _NumTriangles = 0, unsigned int _MinVertex = ~0u, unsigned int _MaxVertex = 0)
		: StartTriangle(_StartTriangle)
		, NumTriangles(_NumTriangles)
		, MinVertex(_MinVertex)
		, MaxVertex(_MaxVertex)
	{}

	unsigned int StartTriangle; // index buffer offset in # of triangles
	unsigned int NumTriangles; // Number of triangles in range
	unsigned int MinVertex; // min index into vertex buffer that will be accessed
	unsigned int MaxVertex; // max index into vertex buffer that will be accessed
};

// Lines are common for debugging, so define what they are in one place: two 
// end points with a color for each.
struct oGPU_LINE
{
	// Line shaders should expect a position and a color for each vertex of a line

	oGPU_LINE()
		: Start(0.0f)
		, StartColor(std::Black)
		, End(0.0f)
		, EndColor(std::Black)
	{}

	float3 Start;
	oColor StartColor;
	float3 End;
	oColor EndColor;
};

#endif
