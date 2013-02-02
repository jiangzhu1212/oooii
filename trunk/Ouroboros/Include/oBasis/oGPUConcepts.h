/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
// This header contains definitions of concepts relating to graphics processing.
// This header has been separated into oBasis to enable generic code such as 
// file formats and parsing and geometry synthesis to use a common vocabulary, 
// but this is also intended to be the primary vocabulary for a cross-platform 
// graphics interface as well.

// Some notable definitions for concepts used here:
// Pipeline: total configuration of GPU hardware execution that produces as 
//           result when Draw()/Submit() is called. This includes programmable 
//           shader binding as well as fixed-function state and input layout 
//           definition.

#pragma once
#ifndef oGPUConcepts_h
#define oGPUConcepts_h

#include <oBasis/oOperators.h>
#include <oBasis/oResizedType.h>
#include <oBasis/oRTTI.h>
#include <oBasis/oStddef.h>
#include <oBasis/oSurface.h>
#include <oBasis/oVersion.h>

// _____________________________________________________________________________
// Constants describing some rational upper limits on GPU resource usage

static const uint oGPU_MAX_NUM_INPUT_SLOTS = 3;
static const uint oGPU_MAX_NUM_VERTEX_ELEMENTS = 32;
static const uint oGPU_MAX_NUM_SAMPLERS = 16;
static const uint oGPU_MAX_NUM_MRTS = 8;
static const uint oGPU_MAX_NUM_UNORDERED_BUFFERS = 8;
static const uint oGPU_MAX_NUM_VIEWPORTS = 16;

// _____________________________________________________________________________
// "Textures" are a robust and complex concept in modern GPU usage, so define
// here some type trait flags for use in enumerating texture and resource types
// and usage.

#define oGPU_TRAIT_RESOURCE_READBACK 0x00000001
#define oGPU_TRAIT_RESOURCE_UNORDERED (oGPU_TRAIT_RESOURCE_READBACK << 1)
#define oGPU_TRAIT_TEXTURE_1D (oGPU_TRAIT_RESOURCE_UNORDERED << 1)
#define oGPU_TRAIT_TEXTURE_2D (oGPU_TRAIT_TEXTURE_1D << 1)
#define oGPU_TRAIT_TEXTURE_3D (oGPU_TRAIT_TEXTURE_2D << 1)
#define oGPU_TRAIT_TEXTURE_CUBE (oGPU_TRAIT_TEXTURE_3D << 1)
#define oGPU_TRAIT_TEXTURE_MIPS (oGPU_TRAIT_TEXTURE_CUBE << 1)
#define oGPU_TRAIT_TEXTURE_RENDER_TARGET (oGPU_TRAIT_TEXTURE_MIPS << 1)

// _____________________________________________________________________________
// Enumeration of common GPU-related concepts and fix-function pipeline states

enum oGPU_API
{
	oGPU_API_UNKNOWN,
	oGPU_API_D3D,
	oGPU_API_OGL,
	oGPU_API_OGLES,
	oGPU_API_WEBGL,
	oGPU_API_CUSTOM,

	oGPU_API_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGPU_API)

enum oGPU_VENDOR
{
	oGPU_VENDOR_UNKNOWN,
	oGPU_VENDOR_NVIDIA,
	oGPU_VENDOR_AMD,
	oGPU_VENDOR_INTEL,
	oGPU_VENDOR_ARM,
	oGPU_VENDOR_CUSTOM,
	oGPU_VENDOR_INTERNAL,
	
	oGPU_VENDOR_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGPU_VENDOR)

enum oGPU_CUBE_FACE
{
	oGPU_CUBE_POS_X,
	oGPU_CUBE_NEG_X,
	oGPU_CUBE_POS_Y,
	oGPU_CUBE_NEG_Y,
	oGPU_CUBE_POS_Z,
	oGPU_CUBE_NEG_Z,

	oGPU_CUBE_FACE_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGPU_CUBE_FACE)

enum oGPU_DEBUG_LEVEL
{
	oGPU_DEBUG_NONE, // No driver debug reporting
	oGPU_DEBUG_NORMAL, // Trivial/auto-handled warnings by driver squelched
	oGPU_DEBUG_UNFILTERED, // No oGPU suppression of driver warnings
	
	oGPU_DEBUG_LEVEL_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGPU_DEBUG_LEVEL)

enum oGPU_PIPELINE_STAGE
{
	oGPU_VERTEX_SHADER,
	oGPU_HULL_SHADER,
	oGPU_DOMAIN_SHADER,
	oGPU_GEOMETRY_SHADER,
	oGPU_PIXEL_SHADER,

	oGPU_PIPELINE_STAGE_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGPU_PIPELINE_STAGE)

enum oGPU_PRIMITIVE_TYPE
{
	oGPU_UNKNOWN_PRIMITIVE_TYPE,
	oGPU_POINTS,
	oGPU_LINES,
	oGPU_LINE_STRIPS,
	oGPU_TRIANGLES,
	oGPU_TRIANGLE_STRIPS,
	oGPU_LINES_ADJACENCY,
	oGPU_LINE_STRIPS_ADJACENCY,
	oGPU_TRIANGLES_ADJACENCY,
	oGPU_TRIANGLE_STRIPS_ADJACENCY,
	oGPU_PATCHES1, // # postfix is the # of control points per patch
	oGPU_PATCHES2,
	oGPU_PATCHES3,
	oGPU_PATCHES4,
	oGPU_PATCHES5,
	oGPU_PATCHES6,
	oGPU_PATCHES7,
	oGPU_PATCHES8,
	oGPU_PATCHES9,
	oGPU_PATCHES10,
	oGPU_PATCHES11,
	oGPU_PATCHES12,
	oGPU_PATCHES13,
	oGPU_PATCHES14,
	oGPU_PATCHES15,
	oGPU_PATCHES16,
	oGPU_PATCHES17,
	oGPU_PATCHES18,
	oGPU_PATCHES19,
	oGPU_PATCHES20,
	oGPU_PATCHES21,
	oGPU_PATCHES22,
	oGPU_PATCHES23,
	oGPU_PATCHES24,
	oGPU_PATCHES25,
	oGPU_PATCHES26,
	oGPU_PATCHES27,
	oGPU_PATCHES28,
	oGPU_PATCHES29,
	oGPU_PATCHES30,
	oGPU_PATCHES31,
	oGPU_PATCHES32,
	oGPU_PRIMITIVE_TYPE_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGPU_PRIMITIVE_TYPE)

enum oGPU_RESOURCE_TYPE
{
	// @oooii-tony: The GPU concept level deals with real buffers of data, not 
	// inter-buffer referencing, so some of these resource types will come out
	// soon such as material, material_set, skeleton (it's a buffer), 
	// and skeleton_pose.

	// oGPU_TEXTURE_YUV will soon be collapsed into supporting YUV formats as part 
	// of oSURFACE_FORMAT, as appears DirectX 11.1 will do.

	oGPU_BUFFER, // generic RO, WO or RW non-texture buffer, or index/vertex buffer
	oGPU_INSTANCE_LIST, // batched set of per-draw parameters
	oGPU_LINE_LIST, // list of lines for drawing as lines (not triangles)
	oGPU_MATERIAL, // a collection of the textures and constants used to color a triangle
	oGPU_MATERIAL_SET, // a collection of named materials that maps to a mesh
	oGPU_MESH, // description of a geometry using index and vertex buffers to form triangles
	oGPU_OUTPUT, // receives final rendering output and channels it to the final presentation medium (often a swap chain -> window mapping)
	oGPU_SCENE, // container that stores all items on which a frustum cull is applied.
	oGPU_SKELETON, // for vertex blending (skinning), the mapping from animation to vertex
	oGPU_SKELETON_POSE, // the result of an animation as it is applied to an associated skeleton
	
	oGPU_TEXTURE, // buffer able to be bound as a rasterization target or HW sampled

	// @oooii-tony: Start using oGPU_TEXTURE and new YUV-related formats in 
	// oSURFACE_FORMAT. the _RGB and _YUV types will be going away soon.
	oGPU_TEXTURE_YUV, // deprecated format on its way out, YUV support is now directly in oSURFACE_FORMAT

	oGPU_RESOURCE_TYPE_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGPU_RESOURCE_TYPE)

// A mesh is a first-class citizen in oGPU, and index/vertex buffers are 
// components of the mesh atom. To access the components, use one of these 
// values as a subresource index.
enum oGPU_MESH_SUBRESOURCE
{
	oGPU_MESH_RANGES,
	oGPU_MESH_INDICES,
	oGPU_MESH_VERTICES0,
	oGPU_MESH_VERTICES1,
	oGPU_MESH_VERTICES2,

	oGPU_MESH_SUBRESOURCE_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGPU_MESH_SUBRESOURCE)

enum oGPU_BUFFER_TYPE
{
	// Binding fit for rasterization HW. Using this requires a structure byte size
	// to be specified.
	oGPU_BUFFER_DEFAULT,

	// An oGPU_BUFFER_DEFAULT fit for receiving a copy from GPU memory to CPU-
	// accessible memory.
	oGPU_BUFFER_READBACK,

	// A buffer fit for rasterization HW. This often acts to indirect into a 
	// vertex buffer to describe primitives. There is often a special place in the 
	// pipeline for index buffers and it is likely not able to be bound to any 
	// other part.
	oGPU_BUFFER_INDEX,

	// An oGPU_BUFFER_INDEX fit for receiving a copy from GPU memory to CPU-
	// accessible memory.
	oGPU_BUFFER_INDEX_READBACK,

	// A buffer fit for rasterization HW. This holds the data representation for 
	// geometry primitives such as those described by oGPU_PRIMITIVE_TYPE.
	oGPU_BUFFER_VERTEX,

	// An oGPU_BUFFER_VERTEX fit for receiving a copy from GPU memory to CPU-
	// accessible memory.
	oGPU_BUFFER_VERTEX_READBACK,

	// A raw buffer indexed by bytes (32-bit aligned access only though). This is
	// the type to use for Dispatch()/Draw() parameters coming from the GPU.
	// (Indirect drawing).
	oGPU_BUFFER_UNORDERED_RAW,

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

	oGPU_BUFFER_TYPE_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGPU_BUFFER_TYPE)

enum oGPU_TEXTURE_TYPE
{
	// 1D texture.
	oGPU_TEXTURE_1D_MAP = oGPU_TRAIT_TEXTURE_1D,
	oGPU_TEXTURE_1D_MAP_MIPS = oGPU_TRAIT_TEXTURE_1D | oGPU_TRAIT_TEXTURE_MIPS,
	oGPU_TEXTURE_1D_RENDER_TARGET = oGPU_TRAIT_TEXTURE_1D | oGPU_TRAIT_TEXTURE_RENDER_TARGET,
	oGPU_TEXTURE_1D_RENDER_TARGET_MIPS = oGPU_TRAIT_TEXTURE_1D | oGPU_TRAIT_TEXTURE_MIPS | oGPU_TRAIT_TEXTURE_RENDER_TARGET,
	oGPU_TEXTURE_1D_READBACK = oGPU_TRAIT_TEXTURE_1D | oGPU_TRAIT_RESOURCE_READBACK,
	oGPU_TEXTURE_1D_READBACK_MIPS = oGPU_TRAIT_TEXTURE_1D | oGPU_TRAIT_TEXTURE_MIPS | oGPU_TRAIT_RESOURCE_READBACK,

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
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGPU_TEXTURE_TYPE)

inline bool oGPUTextureTypeHasMips(oGPU_TEXTURE_TYPE _Type) { return 0 != ((int)_Type & oGPU_TRAIT_TEXTURE_MIPS); }
inline bool oGPUTextureTypeIsReadback(oGPU_TEXTURE_TYPE _Type) { return 0 != ((int)_Type & oGPU_TRAIT_RESOURCE_READBACK); }
inline bool oGPUTextureTypeIsRenderTarget(oGPU_TEXTURE_TYPE _Type) { return 0 != ((int)_Type & oGPU_TRAIT_TEXTURE_RENDER_TARGET); }
inline bool oGPUTextureTypeIs1DMap(oGPU_TEXTURE_TYPE _Type) { return 0 != ((int)_Type & oGPU_TRAIT_TEXTURE_1D); }
inline bool oGPUTextureTypeIs2DMap(oGPU_TEXTURE_TYPE _Type) { return 0 != ((int)_Type & oGPU_TRAIT_TEXTURE_2D); }
inline bool oGPUTextureTypeIsCubeMap(oGPU_TEXTURE_TYPE _Type) { return 0 != ((int)_Type & oGPU_TRAIT_TEXTURE_CUBE); }
inline bool oGPUTextureTypeIs3DMap(oGPU_TEXTURE_TYPE _Type) { return 0 != ((int)_Type & oGPU_TRAIT_TEXTURE_3D); }
inline bool oGPUTextureTypeIsUnordered(oGPU_TEXTURE_TYPE _Type) { return 0 != ((int)_Type & oGPU_TRAIT_RESOURCE_UNORDERED); }

// Returns the matching readback type for the specified type.
inline oGPU_TEXTURE_TYPE oGPUTextureTypeGetReadbackType(oGPU_TEXTURE_TYPE _Type) { return (oGPU_TEXTURE_TYPE)((int)_Type | oGPU_TRAIT_RESOURCE_READBACK); }
inline oGPU_TEXTURE_TYPE oGPUTextureTypeGetMipMapType(oGPU_TEXTURE_TYPE _Type) { return (oGPU_TEXTURE_TYPE)((int)_Type | oGPU_TRAIT_TEXTURE_MIPS); }
inline oGPU_TEXTURE_TYPE oGPUTextureTypeGetRenderTargetType(oGPU_TEXTURE_TYPE _Type) { return (oGPU_TEXTURE_TYPE)((int)_Type | oGPU_TRAIT_TEXTURE_RENDER_TARGET); }
inline oGPU_TEXTURE_TYPE oGPUTextureTypeGetBasicType(oGPU_TEXTURE_TYPE _Type) { return (oGPU_TEXTURE_TYPE)((int)_Type & (oGPU_TRAIT_TEXTURE_1D|oGPU_TRAIT_TEXTURE_2D|oGPU_TRAIT_TEXTURE_3D|oGPU_TRAIT_TEXTURE_CUBE)); }

// Returns the matching non readback type for the specified type.
inline oGPU_TEXTURE_TYPE oGPUTextureTypeStripReadbackType(oGPU_TEXTURE_TYPE _Type) { return (oGPU_TEXTURE_TYPE)((int)_Type & ~oGPU_TRAIT_RESOURCE_READBACK); }
inline oGPU_TEXTURE_TYPE oGPUTextureTypeStripMipMapType(oGPU_TEXTURE_TYPE _Type) { return (oGPU_TEXTURE_TYPE)((int)_Type & ~oGPU_TRAIT_TEXTURE_MIPS); }

enum oGPU_SURFACE_STATE
{
	// Front-facing is clockwise winding order. Back-facing is counter-clockwise.

	oGPU_FRONT_FACE, // Draws all faces whose normal points towards the viewer
	oGPU_BACK_FACE,  // Draws all faces whose normal points away from the viewer
	oGPU_TWO_SIDED, // Draws all faces
	oGPU_FRONT_WIREFRAME, // Draws the borders of all faces whose normal points towards the viewer
	oGPU_BACK_WIREFRAME,  // Draws the borders of all faces whose normal points away from the viewer
	oGPU_TWO_SIDED_WIREFRAME, // Draws the borders of all faces
	oGPU_SURFACE_STATE_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGPU_SURFACE_STATE)

enum oGPU_DEPTH_STENCIL_STATE
{
	oGPU_DEPTH_STENCIL_NONE, // No depth or stencil operation
	oGPU_DEPTH_TEST_AND_WRITE, // normal z-buffering mode where if occluded, exit else render and write new Z value. No stencil operation
	oGPU_DEPTH_TEST, // test depth only, do not write. No stencil operation
	
	oGPU_DEPTH_STENCIL_STATE_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGPU_DEPTH_STENCIL_STATE)

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

	oGPU_BLEND_STATE_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGPU_BLEND_STATE)

enum oGPU_SAMPLER_STATE
{
	// Use 100% of the texel nearest to the sample point. If the sample is outside 
	// the texture, use an edge texel.
	oGPU_POINT_CLAMP, 
	
	// Use 100% of the texel nearest to the sample point. Use only the fractional 
	// part of the sample point.
	oGPU_POINT_WRAP,
	
	// Trilinear sample texels around sample point. If the sample is outside the 
	// texture, use an edge texel.
	oGPU_LINEAR_CLAMP,
	
	// Trilinear sample texels around sample point. Use only the fractional part 
	// of the sample point.
	oGPU_LINEAR_WRAP,
	
	// Anisotropically sample texels around sample point. If the sample is outside 
	// the texture, use an edge texel.
	oGPU_ANISO_CLAMP,
	
	// Anisotropically sample texels around sample point. Use only the fractional 
	// part of the sample point.
	oGPU_ANISO_WRAP,

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

	oGPU_SAMPLER_STATE_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGPU_SAMPLER_STATE)

enum oGPU_CLEAR
{
	oGPU_CLEAR_DEPTH,
	oGPU_CLEAR_STENCIL,
	oGPU_CLEAR_DEPTH_STENCIL,
	oGPU_CLEAR_COLOR,
	oGPU_CLEAR_COLOR_DEPTH,
	oGPU_CLEAR_COLOR_STENCIL,
	oGPU_CLEAR_COLOR_DEPTH_STENCIL,

	oGPU_CLEAR_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGPU_CLEAR)

enum oGPU_NORMAL_SPACE
{
	// AKA "object-space" normals. Normals can point in any direction.
	oGPU_NORMAL_LOCAL_SPACE,

	// Normals are relative to the surface at which they are sampled.
	oGPU_NORMAL_TANGENT_SPACE,

	// Single-channel intensity map that would probably be sobel-filtered to 
	// derive a vector in local (object) space.
	oGPU_NORMAL_BUMP_LOCAL_SPACE,
	
	// Single-channel intensity map that would probably be sobel-filtered to 
	// derive a vector in tangent (surface) space.
	oGPU_NORMAL_BUMP_TANGENT_SPACE,

	oGPU_NORMAL_SPACE_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGPU_NORMAL_SPACE)

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

	oGPU_BRDF_MODEL_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGPU_BRDF_MODEL)

// _____________________________________________________________________________
// Structures that encapsulate parameters for common GPU-related operations

struct oGPU_BOX
{
	// Empty box
	oGPU_BOX()
		: Left(0)
		, Right(0)
		, Top(0)
		, Bottom(1)
		, Front(0)
		, Back(1)
	{}

	// Convenience when an oGPU_BOX is required, but the data is 1-dimensional.
	oGPU_BOX(uint Width)
		: Left(0)
		, Right(Width)
		, Top(0)
		, Bottom(1)
		, Front(0)
		, Back(1)
	{}

	uint Left;
	uint Right;
	uint Top;
	uint Bottom;
	uint Front;
	uint Back;
};

struct oGPU_RANGE
{
	// A mesh's vertices are generally stored together for efficiency, but are 
	// also organized by material or other sub-grouping. A range describes that 
	// sub-grouping.

	oGPU_RANGE(uint _StartPrimitive = 0, uint _NumPrimitives = 0, uint _MinVertex = 0, uint _MaxVertex = ~0u)
		: StartPrimitive(_StartPrimitive)
		, NumPrimitives(_NumPrimitives)
		, MinVertex(_MinVertex)
		, MaxVertex(_MaxVertex)
	{}

	uint StartPrimitive; // index buffer offset in # of primitives
	uint NumPrimitives; // Number of primitives in range
	uint MinVertex; // min index into vertex buffer that will be accessed
	uint MaxVertex; // max index into vertex buffer that will be accessed
};

#define oGPU_VERTEX_ELEMENT_NULL { 0, oSURFACE_UNKNOWN, 0, false }
struct oGPU_VERTEX_ELEMENT
{
	// Generic description of an element in a heterogeneous (AOS) vertex. This 
	// struct is 16 bytes, thus 4-byte/8-byte aligned. This struct has no ctor so 
	// it can be used in const static declarations.

	//oGPU_VERTEX_ELEMENT()
	//	: InputSlot(0)
	//	, Format(oSURFACE_UNKNOWN)
	//	, Instanced(false)
	//{}

	// Name, such as 'POS0' for position. This code should be fit for writing to 
	// disk and uniquely identifying a semantic channel and its index. The 
	// following rules should be applied for index: if the right-most char in the 
	// oFourCC is numeric, use that value, else the value is zero. ('TANG' for 
	// example would be index 0).
	oFourCC Semantic;

	// The format of the data, i.e. a float4 would be oSURFACE_R32G32B32A32_FLOAT
	oResizedType<oSURFACE_FORMAT, short> Format;

	// The input slot this will be bound too. Basically modern pipelines allow
	// several vertex arrays to be bound, so this specifies from which this 
	// element will be drawn.
	uchar InputSlot;

	// If false, this is per-vertex data. If true, this is per-instance data as 
	// used during instanced drawing.
	bool Instanced;

	inline bool operator==(const oGPU_VERTEX_ELEMENT& _That) const
	{
		return Semantic == _That.Semantic 
			&& Format == _That.Format 
			&& InputSlot == _That.InputSlot 
			&& Instanced == _That.Instanced;
	}

	inline bool operator<(const oGPU_VERTEX_ELEMENT& _That) const
	{
		return ((InputSlot < _That.InputSlot)
			|| (InputSlot == _That.InputSlot && !Instanced && _That.Instanced)
			|| (InputSlot == _That.InputSlot && Instanced == _That.Instanced && Semantic < _That.Semantic));
	}

	oOPERATORS_COMPARABLE(oGPU_VERTEX_ELEMENT)
};

// Extracts the right-most part of a fourcc and interprets it as a number used
// for the semantic index of the vertex element. If the value is non-numeric 
// then the return value will be 0.
oAPI uint oGPUGetSemanticIndex(const oFourCC& _FourCC);

// Returns the size in bytes of the sum of all vertex elements for the specified 
// input slot.
oAPI uint oGPUCalcVertexSize(const threadsafe oGPU_VERTEX_ELEMENT* _pElements, uint _NumElements, uint _InputSlot);
template<size_t size> uint oGPUCalcVertexSize(const threadsafe oGPU_VERTEX_ELEMENT (&_pElements)[size], uint _InputSlot) { return oGPUCalcVertexSize(_pElements, size, _InputSlot); }

// Returns the number of input slots used by the specified vertex elements.
oAPI uint oGPUCalcNumInputSlots(const oGPU_VERTEX_ELEMENT* _pElements, uint _NumElements);
template<size_t size> uint oGPUCalcNumInputSlots(const oGPU_VERTEX_ELEMENT (&_pElements)[size]) { return oGPUCalcNumInputSlots(_pElements, size); }

inline bool oGPUHas16BitIndices(uint _NumVertices) { return _NumVertices <= oNumericLimits<ushort>::GetMax(); }
inline uint oGPUGetIndexSize(uint _NumVertices) { return oGPUHas16BitIndices(_NumVertices) ? sizeof(ushort) : sizeof(uint); }

struct oGPU_LINE_VERTEX
{
	// A line list's reserved memory or a user source for a commit must be in this
	// format.

	oGPU_LINE_VERTEX()
		: Position(0.0f, 0.0f, 0.0f)
		, Color(std::Black)
	{}

	float3 Position;
	oColor Color;
};

struct oGPU_LINE
{
	// Lines are common for debugging, so define what they are in one place: two 
	// end points with a color for each.
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

// _____________________________________________________________________________
// DESC-class structures: structs that describe large GPU-related buffers

struct oGPU_INSTANCE_LIST_DESC
{
	// Elements points to a full pipeline's expected inputs, including non-
	// instanced vertex elements. InputSlot specifies which of these elements will 
	// be populated in the instance list described by this struct.

	oGPU_INSTANCE_LIST_DESC()
		: InputSlot(oInvalid)
		, MaxNumInstances(0)
		, NumInstances(0)
		, NumVertexElements(0)
	{
		oGPU_VERTEX_ELEMENT Init = oGPU_VERTEX_ELEMENT_NULL;
		oINIT_ARRAY(VertexElements, Init);
	}

	uint InputSlot;
	uint MaxNumInstances;
	uint NumInstances;
	uint NumVertexElements;
	oGPU_VERTEX_ELEMENT VertexElements[oGPU_MAX_NUM_VERTEX_ELEMENTS];
};

struct oGPU_LINE_LIST_DESC
{
	oGPU_LINE_LIST_DESC()
		: MaxNumLines(0)
		, NumLines(0)
	{}

	uint MaxNumLines;
	uint NumLines;
};

struct oGPU_MESH_DESC
{
	oGPU_MESH_DESC()
		: NumRanges(0)
		, NumIndices(0)
		, NumVertices(0)
		, NumVertexElements(0)
	{
		static const oGPU_VERTEX_ELEMENT Init = oGPU_VERTEX_ELEMENT_NULL;
		oINIT_ARRAY(VertexElements, Init);
	}

	oAABoxf LocalSpaceBounds;
	uint NumRanges;
	uint NumIndices;
	uint NumVertices;
	uint NumVertexElements;
	oGPU_VERTEX_ELEMENT VertexElements[oGPU_MAX_NUM_VERTEX_ELEMENTS];
};

struct oGPU_BUFFER_DESC
{
	// Description of a constant buffer (view, draw, material). Client code can 
	// defined whatever value are to be passed to a shader that expects them.
	// StructByteSize must be 16-byte aligned.
	// Instead of structured (StructByteSize=0), you can provide a Format

	oGPU_BUFFER_DESC()
		: Type(oGPU_BUFFER_DEFAULT)
		, Format(oSURFACE_UNKNOWN)
		, StructByteSize(oInvalid)
		, ArraySize(1)
	{}

	// Specifies the type of the constant buffer. Normally the final buffer size
	// is StructByteSize*ArraySize. If the type is specified as 
	// oGPU_BUFFER_UNORDERED_UNSTRUCTURED, then StructByteSize must be 
	// oInvalid and the size is calculated as (size of Format) * ArraySize.
	oGPU_BUFFER_TYPE Type;

	// This must be valid for oGPU_BUFFER_UNORDERED_UNSTRUCTURED types, and 
	// oSURFACE_UNKNOWN for all other types.
	oSURFACE_FORMAT Format;

	// This must be oInvalid for oGPU_BUFFER_UNORDERED_UNSTRUCTURED types,
	// but valid for all other types.
	uint StructByteSize;

	// The number of format elements or structures in the buffer.
	uint ArraySize;
};

struct oGPU_TEXTURE_DESC
{
	oGPU_TEXTURE_DESC()
		: Dimensions(oInvalid, oInvalid, oInvalid)
		, NumSlices(1)
		, Format(oSURFACE_B8G8R8A8_UNORM)
		, Type(oGPU_TEXTURE_2D_MAP)
	{}

	int3 Dimensions;
	int NumSlices;
	oSURFACE_FORMAT Format;
	oGPU_TEXTURE_TYPE Type;
};

struct oGPU_STATIC_PIPELINE_DESC
{
	// Same as oGPU_PIPELINE_DESC but with no ctor so it can be statically 
	// initialized.

	const char* DebugName;
	const oGPU_VERTEX_ELEMENT* pElements;
	uint NumElements;
	oGPU_PRIMITIVE_TYPE InputType;
	const void* pVertexShader;
	const void* pHullShader;
	const void* pDomainShader;
	const void* pGeometryShader;
	const void* pPixelShader;
};

struct oGPU_PIPELINE_DESC : oGPU_STATIC_PIPELINE_DESC
{
	// Abstracts the raw data readied for use by a GPU. This might be text source
	// that is compiled by the system, intermediate bytecode that is further 
	// compiled by the driver, or direct assembly.

	oGPU_PIPELINE_DESC()
	{
		DebugName = "oGPU_PIPELINE_DESC";
		pElements = nullptr;
		NumElements = oInvalid;
		InputType = oGPU_UNKNOWN_PRIMITIVE_TYPE;
		pVertexShader = nullptr;
		pHullShader = nullptr;
		pDomainShader = nullptr;
		pGeometryShader = nullptr;
		pPixelShader = nullptr;
	}

	const oGPU_PIPELINE_DESC& operator=(const oGPU_STATIC_PIPELINE_DESC& _That) { *(oGPU_STATIC_PIPELINE_DESC*)this = _That; return *this; }
};

struct oGPU_STATIC_COMPUTE_SHADER_DESC
{
	const char* DebugName;
	const void* pComputeShader;
};

struct oGPU_COMPUTE_SHADER_DESC : oGPU_STATIC_COMPUTE_SHADER_DESC
{
	oGPU_COMPUTE_SHADER_DESC()
	{
		DebugName = "oGPU_COMPUTE_SHADER_DESC";
		pComputeShader = nullptr;
	}

	const oGPU_COMPUTE_SHADER_DESC& operator=(const oGPU_STATIC_COMPUTE_SHADER_DESC& _That) { *(oGPU_STATIC_COMPUTE_SHADER_DESC*)this = _That; return *this; }
};

struct oGPU_CLEAR_DESC
{
	oGPU_CLEAR_DESC()
		: DepthClearValue(1.0f)
		, StencilClearValue(0)
	{ oINIT_ARRAY(ClearColor, 0); }

	oColor ClearColor[oGPU_MAX_NUM_MRTS];
	float DepthClearValue;
	uchar StencilClearValue;
};

struct oGPU_RENDER_TARGET_DESC
{
	oGPU_RENDER_TARGET_DESC()
		: Dimensions(oInvalid, oInvalid, oInvalid)
		, NumSlices(1)
		, MRTCount(1)
		, DepthStencilFormat(oSURFACE_UNKNOWN)
		, Type(oGPU_TEXTURE_2D_RENDER_TARGET)
	{ oINIT_ARRAY(Format, oSURFACE_UNKNOWN); }

	int3 Dimensions;
	int NumSlices;
	int MRTCount;
	oSURFACE_FORMAT Format[oGPU_MAX_NUM_MRTS];
	oSURFACE_FORMAT DepthStencilFormat; // Use UNKNOWN for no depth
	oGPU_CLEAR_DESC ClearDesc;
	oGPU_TEXTURE_TYPE Type;
};

struct oGPU_COMMAND_LIST_DESC
{
	int DrawOrder;
};

struct oGPU_DEVICE_INIT
{
	oGPU_DEVICE_INIT(const char* _DebugName = "oGPUDevice")
		: DebugName(_DebugName)
		, Version(11,0)
		, DriverDebugLevel(oGPU_DEBUG_NONE)
		, AdapterIndex(0)
		, VirtualDesktopPosition(oDEFAULT, oDEFAULT)
		, UseSoftwareEmulation(false)
		, DriverVersionMustBeExact(false)
	{}

	// Name associated with this device in debug output
	oStringS DebugName;

	// The minimum version of the driver required to successfully create the 
	// device. If the driver version is 0.0.0.0 (the default) then a hard-coded
	// internal value is used which represents a pivot point where QA was done at 
	// OOOii that showed significant problems or missing features before that 
	// default version.
	oVersion MinDriverVersion;

	// The version of the underlying API to use.
	oVersion Version;

	// Specify to what degree driver warnings/errors are reported. oGPU-level 
	// errors and warnings are always reported.
	oGPU_DEBUG_LEVEL DriverDebugLevel;

	// If VirtualDesktopPosition is oDEFAULT, oDEFAULT, then use the nth found
	// device as specified by this Index. If VirtualDesktopPosition is anything
	// valid, then the device used to handle that desktop position will be used
	// and Index is ignored.
	int AdapterIndex;

	// Position on the desktop and thus on a monitor to be used to determine which 
	// GPU is used for that monitor and create a device for that GPU.
	int2 VirtualDesktopPosition;

	// Allow SW emulation for the specified version. If false, a create will fail
	// if HW acceleration is not available.
	bool UseSoftwareEmulation;

	// If true, == is used to match MinDriverVersion to the specified GPU's 
	// driver. If false, CurVer >= MinDriverVersion is used.
	bool DriverVersionMustBeExact;
};

struct oGPU_DEVICE_DESC
{
	oGPU_DEVICE_DESC()
		: NativeMemory(0)
		, DedicatedSystemMemory(0)
		, SharedSystemMemory(0)
		, AdapterIndex(0)
		, API(oGPU_API_UNKNOWN)
		, Vendor(oGPU_VENDOR_UNKNOWN)
		, IsSoftwareEmulation(false)
		, DebugReportingEnabled(false)
	{}

	// Name associated with this device in debug output
	oStringS DebugName;

	// Description as provided by the device vendor
	oStringM DeviceDescription;

	// Description as provided by the driver vendor
	oStringM DriverDescription;

	// Number of bytes present on the device (AKA VRAM)
	ullong NativeMemory;

	// Number of bytes reserved by the system to accommodate data transfer to the 
	// device
	ullong DedicatedSystemMemory;

	// Number of bytes reserved in system memory used instead of a separate bank 
	// of NativeMemory 
	ullong SharedSystemMemory;

	// The version for the software that supports the native API. This depends on 
	// the API type being used.
	oVersion DriverVersion;

	// The feature level the device supports. This depends on the API type being 
	// used.
	oVersion FeatureVersion; 

	// The driver/software interface version that might be different than the 
	// capabilities of the device. This depends on the API type being used.
	oVersion InterfaceVersion;

	// The zero-based index of the adapter. This may be different than what is 
	// specified in oGPU_DEVICE_INIT in certain debug/development modes.
	int AdapterIndex;

	// Describes the API used to implement the oGPU API
	oGPU_API API;

	// Describes the company that made the device
	oGPU_VENDOR Vendor;

	// True if the device was created in software emulation mode
	bool IsSoftwareEmulation;

	// True if the device was created with debug reporting enalbed.
	bool DebugReportingEnabled;
};

#endif
