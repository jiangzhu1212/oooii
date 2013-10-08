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

#include <oBasis/oResizedType.h>
#include <oBasis/oRTTI.h>
#include <oBasis/oStddef.h>
#include <oBase/vendor.h>
#include <oBase/version.h>
#include <oSurface/surface.h>
#include <array>

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
#define oGPU_TRAIT_TEXTURE_ARRAY (oGPU_TRAIT_TEXTURE_CUBE << 1)
#define oGPU_TRAIT_TEXTURE_MIPS (oGPU_TRAIT_TEXTURE_ARRAY << 1)
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
	oGPU_BUFFER, // generic RO, WO or RW non-texture buffer, or index/vertex buffer
	oGPU_MESH, // description of a geometry using index and vertex buffers to form triangles
	oGPU_TEXTURE, // buffer able to be bound as a rasterization target or HW sampled

	oGPU_RESOURCE_TYPE_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGPU_RESOURCE_TYPE)

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
	// an ouro::surface::format to be specified.
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
	oGPU_TEXTURE_1D_MAP_ARRAY = oGPU_TRAIT_TEXTURE_1D | oGPU_TRAIT_TEXTURE_ARRAY,
	oGPU_TEXTURE_1D_MAP_ARRAY_MIPS = oGPU_TRAIT_TEXTURE_1D | oGPU_TRAIT_TEXTURE_ARRAY | oGPU_TRAIT_TEXTURE_MIPS,
	oGPU_TEXTURE_1D_RENDER_TARGET = oGPU_TRAIT_TEXTURE_1D | oGPU_TRAIT_TEXTURE_RENDER_TARGET,
	oGPU_TEXTURE_1D_RENDER_TARGET_MIPS = oGPU_TRAIT_TEXTURE_1D | oGPU_TRAIT_TEXTURE_MIPS | oGPU_TRAIT_TEXTURE_RENDER_TARGET,
	oGPU_TEXTURE_1D_READBACK = oGPU_TRAIT_TEXTURE_1D | oGPU_TRAIT_RESOURCE_READBACK,
	oGPU_TEXTURE_1D_READBACK_MIPS = oGPU_TRAIT_TEXTURE_1D | oGPU_TRAIT_TEXTURE_MIPS | oGPU_TRAIT_RESOURCE_READBACK,
	oGPU_TEXTURE_1D_READBACK_ARRAY = oGPU_TRAIT_TEXTURE_1D | oGPU_TRAIT_TEXTURE_ARRAY | oGPU_TRAIT_RESOURCE_READBACK,
	oGPU_TEXTURE_1D_READBACK_ARRAY_MIPS = oGPU_TRAIT_TEXTURE_1D | oGPU_TRAIT_TEXTURE_ARRAY | oGPU_TRAIT_TEXTURE_MIPS | oGPU_TRAIT_RESOURCE_READBACK,

	// "normal" 2D texture.
	oGPU_TEXTURE_2D_MAP = oGPU_TRAIT_TEXTURE_2D,
	oGPU_TEXTURE_2D_MAP_MIPS = oGPU_TRAIT_TEXTURE_2D | oGPU_TRAIT_TEXTURE_MIPS,
	oGPU_TEXTURE_2D_MAP_ARRAY = oGPU_TRAIT_TEXTURE_2D | oGPU_TRAIT_TEXTURE_ARRAY,
	oGPU_TEXTURE_2D_MAP_ARRAY_MIPS = oGPU_TRAIT_TEXTURE_2D | oGPU_TRAIT_TEXTURE_ARRAY | oGPU_TRAIT_TEXTURE_MIPS,
	oGPU_TEXTURE_2D_RENDER_TARGET = oGPU_TRAIT_TEXTURE_2D | oGPU_TRAIT_TEXTURE_RENDER_TARGET,
	oGPU_TEXTURE_2D_RENDER_TARGET_MIPS = oGPU_TRAIT_TEXTURE_2D | oGPU_TRAIT_TEXTURE_MIPS | oGPU_TRAIT_TEXTURE_RENDER_TARGET,
	oGPU_TEXTURE_2D_READBACK = oGPU_TRAIT_TEXTURE_2D | oGPU_TRAIT_RESOURCE_READBACK,
	oGPU_TEXTURE_2D_READBACK_MIPS = oGPU_TRAIT_TEXTURE_2D | oGPU_TRAIT_TEXTURE_MIPS | oGPU_TRAIT_RESOURCE_READBACK,
	oGPU_TEXTURE_2D_READBACK_ARRAY = oGPU_TRAIT_TEXTURE_2D | oGPU_TRAIT_TEXTURE_ARRAY | oGPU_TRAIT_RESOURCE_READBACK,
	oGPU_TEXTURE_2D_READBACK_ARRAY_MIPS = oGPU_TRAIT_TEXTURE_2D | oGPU_TRAIT_TEXTURE_ARRAY | oGPU_TRAIT_TEXTURE_MIPS | oGPU_TRAIT_RESOURCE_READBACK,

	// a "normal" 2D texture, no mips, configured for unordered access. Currently
	// all GPGPU access to such buffers are one subresource at a time, so there is 
	// no spec that describes unordered access to arbitrary mipped memory.
	oGPU_TEXTURE_2D_MAP_UNORDERED = oGPU_TRAIT_TEXTURE_2D | oGPU_TRAIT_RESOURCE_UNORDERED,
	
	// 6- 2D slices that form the faces of a cube that is sampled from its center.
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

enum oGPU_QUERY_TYPE
{
	oGPU_QUERY_TIMER,
	oGPU_QUERY_TYPE_COUNT
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGPU_QUERY_TYPE);

inline bool oGPUTextureTypeHasMips(oGPU_TEXTURE_TYPE _Type) { return 0 != ((int)_Type & oGPU_TRAIT_TEXTURE_MIPS); }
inline bool oGPUTextureTypeIsReadback(oGPU_TEXTURE_TYPE _Type) { return 0 != ((int)_Type & oGPU_TRAIT_RESOURCE_READBACK); }
inline bool oGPUTextureTypeIsRenderTarget(oGPU_TEXTURE_TYPE _Type) { return 0 != ((int)_Type & oGPU_TRAIT_TEXTURE_RENDER_TARGET); }
inline bool oGPUTextureTypeIsArray(oGPU_TEXTURE_TYPE _Type) { return 0 != ((int)_Type & oGPU_TRAIT_TEXTURE_ARRAY); }
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
	// No depth or stencil operation.
	oGPU_DEPTH_STENCIL_NONE,

	// normal z-buffering mode where if occluded or same value (<= less_equal 
	// comparison), exit else render and write new Z value. No stencil operation.
	oGPU_DEPTH_TEST_AND_WRITE,
	
	// test depth only using same method as test-and-write but do not write. No 
	// stencil operation.
	oGPU_DEPTH_TEST,
	
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

// _____________________________________________________________________________
// Structures that encapsulate parameters for common GPU-related operations

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

#define oGPU_VERTEX_ELEMENT_NULL { ouro::fourcc(0), ouro::surface::unknown, 0, false }
struct oGPU_VERTEX_ELEMENT
{
	// Generic description of an element in a heterogeneous (AOS) vertex. This 
	// struct is 16 bytes, thus 4-byte/8-byte aligned. This struct has no ctor so 
	// it can be used in const static declarations.

	//oGPU_VERTEX_ELEMENT()
	//	: InputSlot(0)
	//	, Format(ouro::surface::unknown)
	//	, Instanced(false)
	//{}

	// Name, such as 'POS0' for position. This code should be fit for writing to 
	// disk and uniquely identifying a semantic channel and its index. The 
	// following rules should be applied for index: if the right-most char in the 
	// ouro::fourcc is numeric, use that value, else the value is zero. ('TANG' for 
	// example would be index 0).
	ouro::fourcc Semantic;

	// The format of the data, i.e. a float4 would be ouro::surface::r32g32b32a32_float
	oResizedType<ouro::surface::format, short> Format;

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

// Break out all right-most digits to fill *_pIndex and all left-most remaining
// characters are turned into a string in _Name.
oAPI bool oGPUParseSemantic(const ouro::fourcc& _FourCC, char _Name[5], uint* _pIndex);

// Returns the size in bytes of the sum of all vertex elements for the specified 
// input slot.
oAPI uint oGPUCalcVertexSize(const threadsafe oGPU_VERTEX_ELEMENT* _pElements, uint _NumElements, uint _InputSlot);
template<size_t size> uint oGPUCalcVertexSize(const threadsafe oGPU_VERTEX_ELEMENT (&_pElements)[size], uint _InputSlot) { return oGPUCalcVertexSize(_pElements, size, _InputSlot); }

// Returns the number of input slots used by the specified vertex elements.
oAPI uint oGPUCalcNumInputSlots(const oGPU_VERTEX_ELEMENT* _pElements, uint _NumElements);
template<size_t size> uint oGPUCalcNumInputSlots(const oGPU_VERTEX_ELEMENT (&_pElements)[size]) { return oGPUCalcNumInputSlots(_pElements, size); }

inline bool oGPUHas16BitIndices(uint _NumVertices) { return _NumVertices <= std::numeric_limits<ushort>::max(); }
inline uint oGPUGetIndexSize(uint _NumVertices) { return oGPUHas16BitIndices(_NumVertices) ? sizeof(ushort) : sizeof(uint); }

// _____________________________________________________________________________
// DESC-class structures: structs that describe large GPU-related buffers

struct oGPU_MESH_DESC
{
	oGPU_MESH_DESC()
		: NumRanges(0)
		, NumIndices(0)
		, NumVertices(0)
		, NumVertexElements(0)
	{
		oGPU_VERTEX_ELEMENT e = oGPU_VERTEX_ELEMENT_NULL;
		VertexElements.fill(e);
	}

	oAABoxf LocalSpaceBounds;
	uint NumRanges;
	uint NumIndices;
	uint NumVertices;
	uint NumVertexElements;
	std::array<oGPU_VERTEX_ELEMENT, oGPU_MAX_NUM_VERTEX_ELEMENTS> VertexElements;
};

struct oGPU_BUFFER_DESC
{
	// Description of a constant buffer (view, draw, material). Client code can 
	// defined whatever value are to be passed to a shader that expects them.
	// StructByteSize must be 16-byte aligned.
	// Instead of structured (StructByteSize=0), you can provide a Format

	oGPU_BUFFER_DESC()
		: Type(oGPU_BUFFER_DEFAULT)
		, Format(ouro::surface::unknown)
		, StructByteSize(oInvalid)
		, ArraySize(1)
	{}

	// Specifies the type of the constant buffer. Normally the final buffer size
	// is StructByteSize*ArraySize. If the type is specified as 
	// oGPU_BUFFER_UNORDERED_UNSTRUCTURED, then StructByteSize must be 
	// oInvalid and the size is calculated as (size of Format) * ArraySize.
	oGPU_BUFFER_TYPE Type;

	// This must be valid for oGPU_BUFFER_UNORDERED_UNSTRUCTURED types, and 
	// ouro::surface::unknown for all other types.
	ouro::surface::format Format;

	// This must be oInvalid for oGPU_BUFFER_UNORDERED_UNSTRUCTURED types,
	// but valid for all other types.
	uint StructByteSize;

	// The number of format elements or structures in the buffer.
	uint ArraySize;
};

struct oGPU_TEXTURE_DESC
{
	oGPU_TEXTURE_DESC()
		: Type(oGPU_TEXTURE_2D_MAP)
		, Format(ouro::surface::b8g8r8a8_unorm)
		, ArraySize(oInvalid)
		, Dimensions(oInvalid, oInvalid, oInvalid)
	{}

	oGPU_TEXTURE_TYPE Type;
	ouro::surface::format Format;
	int ArraySize;
	int3 Dimensions;
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

struct oGPU_QUERY_DESC
{
	oGPU_QUERY_TYPE Type;
};

struct oGPU_CLEAR_DESC
{
	oGPU_CLEAR_DESC()
		: DepthClearValue(1.0f)
		, StencilClearValue(0)
	{ ClearColor.fill(ouro::color(0)); }

	std::array<ouro::color, oGPU_MAX_NUM_MRTS> ClearColor;
	float DepthClearValue;
	uchar StencilClearValue;
};

struct oGPU_RENDER_TARGET_DESC
{
	oGPU_RENDER_TARGET_DESC()
		: Type(oGPU_TEXTURE_2D_RENDER_TARGET)
		, DepthStencilFormat(ouro::surface::unknown)
		, ArraySize(1)
		, MRTCount(1)
		, Dimensions(oInvalid, oInvalid, oInvalid)
	{ Format.fill(ouro::surface::unknown); }

	oGPU_TEXTURE_TYPE Type;
	std::array<ouro::surface::format, oGPU_MAX_NUM_MRTS> Format;
	ouro::surface::format DepthStencilFormat; // Use UNKNOWN for no depth
	int ArraySize;
	int MRTCount;
	int3 Dimensions;
	oGPU_CLEAR_DESC ClearDesc;
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
	ouro::sstring DebugName;

	// The minimum version of the driver required to successfully create the 
	// device. If the driver version is 0.0.0.0 (the default) then a hard-coded
	// internal value is used which represents a pivot point where QA was done at 
	// OOOii that showed significant problems or missing features before that 
	// default version.
	ouro::version MinDriverVersion;

	// The version of the underlying API to use.
	ouro::version Version;

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
		, Vendor(ouro::vendor::unknown)
		, IsSoftwareEmulation(false)
		, DebugReportingEnabled(false)
	{}

	// Name associated with this device in debug output
	ouro::sstring DebugName;

	// Description as provided by the device vendor
	ouro::mstring DeviceDescription;

	// Description as provided by the driver vendor
	ouro::mstring DriverDescription;

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
	ouro::version DriverVersion;

	// The feature level the device supports. This depends on the API type being 
	// used.
	ouro::version FeatureVersion; 

	// The driver/software interface version that might be different than the 
	// capabilities of the device. This depends on the API type being used.
	ouro::version InterfaceVersion;

	// The zero-based index of the adapter. This may be different than what is 
	// specified in oGPU_DEVICE_INIT in certain debug/development modes.
	int AdapterIndex;

	// Describes the API used to implement the oGPU API
	oGPU_API API;

	// Describes the company that made the device
	ouro::vendor::value Vendor;

	// True if the device was created in software emulation mode
	bool IsSoftwareEmulation;

	// True if the device was created with debug reporting enalbed.
	bool DebugReportingEnabled;
};

#endif
