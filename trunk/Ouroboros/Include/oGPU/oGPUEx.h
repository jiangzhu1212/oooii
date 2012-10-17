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
#ifndef oGPUEx_h
#define oGPUEx_h

// @oooii-tony: These are common concepts, but for cross-platform code. oGPU 
// should be the minimum platform code a user must implemment per GPU API. We
// should find a different home for this stuff as a layer on top of oGPU. This
// may also be where common structs like view constants and draw constants go.

#include <oBasis/oGPUEnums.h>
#include <oGPU/oGPU.h>
#include <oGPU/oGPUConstants.h>

// @oooii-tony: This is too specific for the lowest-level of do-anything-
// sensible abstraction. Because the main renderer isn't factored out enough yet 
// keep this here, but be careful when using it. oGPU should minimally abstract 
// HW/APIs, not enforce *too* much policy, but some sanity.
enum oGPU_MATERIAL_CHANNEL
{
	oGPU_AMBIENT,
	oGPU_DIFFUSE,
	oGPU_SPECULAR,
	oGPU_EMISSIVE,
	oGPU_TRANSMISSIVE,
	oGPU_NORMAL,

	oGPU_MATERIAL_NUM_CHANNELS,
};

struct oGPU_DRAW_FLAGS
{
	oGPU_DRAW_FLAGS()
		: RespectSurfaceState(true)
		, RespectBlend(true)
		, RespectMaterialConstants(true)
		, RespectSamplerStates(true)
		, RespectTextures(true)
	{}

	bool RespectSurfaceState:1;
	bool RespectBlend:1;
	bool RespectMaterialConstants:1;
	bool RespectSamplerStates:1;
	bool RespectTextures:1;
};

template<typename TextureT> struct oGPU_ENVIRONMENT
{
	// Values that come from the system/are procedurally generated

	oGPU_ENVIRONMENT()
	#ifdef _DEBUG
			: DebugName("unnamed")
	#endif
	{
		oINIT_ARRAY(pTextures, nullptr);
		oINIT_ARRAY(Samplers, oGPU_LINEAR_WRAP);
	}

	TextureT* pTextures[oGPU_MAX_NUM_ENVIRONMENT_TEXTURES];
	oGPU_SAMPLER_STATE Samplers[oGPU_MAX_NUM_ENVIRONMENT_TEXTURES];

	#ifdef _DEBUG
		oStringS DebugName;
	#endif
};

template<typename MeshT, typename MaterialT, typename TextureT>
struct oGPU_DRAWABLE
{
	// Geometry and materials that are relatively static within an environment

	oGPU_DRAWABLE()
		: pMesh(nullptr)
		, Transform(float4x4::Identity)
		, ObjectID(0)
		, MeshRangeIndex(oInvalid)
		, pMaterial(nullptr)
		, SurfaceState(oGPU_FRONT_FACE)
		, BlendState(oGPU_OPAQUE)
		#ifdef _DEBUG
			, DebugName("unnamed")
		#endif
	{
		oINIT_ARRAY(pTextures, nullptr);
		oINIT_ARRAY(Samplers, oGPU_LINEAR_WRAP);
	}

	MeshT* pMesh; // The mesh of which to draw a range
	float4x4 Transform; // world space transformation for the mesh's object space
	oAABoxf WorldSpaceBounds; // bounds already transformed into world space (i.e. any skinning or other vertex transformation is reflected in this bounds as well)
	int ObjectID; // ID identifying uniquely identifying an instance (i.e. for picking)
	int MeshRangeIndex; // The index into the range to be drawn for the specified mesh

	MaterialT* pMaterial; // pointer to a container for all the per-material values
	oGPU_SURFACE_STATE SurfaceState; // rasterizer state
	oGPU_BLEND_STATE BlendState; // output merge/blend state
	TextureT* pTextures[oGPU_MAX_NUM_MATERIAL_TEXTURES]; // list of textures that will be accessed by the pipeline setup
	oGPU_SAMPLER_STATE Samplers[oGPU_MAX_NUM_MATERIAL_TEXTURES]; // sampler state for each of the textures

	oGPU_ENVIRONMENT<TextureT> Environment;

	oGPU_DRAW_FLAGS DrawFlags;
	#ifdef _DEBUG
		oStringS DebugName;
	#endif
};

// Returns a color that best represents a texture of the specified channel. If 
// _ObjectSpaceNormals is false, then tangent space normals are assumed.
oAPI oColor oGPUGetRepresentativeColor(oGPU_MATERIAL_CHANNEL _Channel, bool _ObjectSpaceNormals = true);

oAPI bool oFromString(oGPU_MATERIAL_CHANNEL* _pValue, const char* _StrSource);
oAPI char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oGPU_MATERIAL_CHANNEL& _Channel);
oAPI const char* oAsString(const oGPU_MATERIAL_CHANNEL& _Channel);


// @oooii-tony: I'm not sure if these APIs belong here at all, in oGPU, or 
// oGPUUtil. I do know they don't belong where they were before, which was 
// nowhere.
// Basically rendering at oooii grew up on D3D and D3DX, which have very robust
// libraries for image format conversion and handling. We didn't spend time 
// reproducing this on our own, so we're relying on this stuff as placeholder.
// I suspect if we add support for a different library, such robust support for
// image formats won't be there and we'll have to implement a lot of this stuff,
// so defer that dev time until then and then bring it back to something that
// can be promoted out of oGPU. For now, at least hide the D3D part...

// Convert the format of a surface into another format in another surface. This
// uses GPU acceleration for BC6H/7 and is currently a pass-through to D3DX11's
// conversion functions at the moment. Check debug logs if this function seems
// to hang because if for whatever reason the CPU/SW version of the BC6H/7
// codec is used, it can take a VERY long time.
bool oGPUSurfaceConvert(
	void* oRESTRICT _pDestination
	, size_t _DestinationRowPitch
	, oSURFACE_FORMAT _DestinationFormat
	, const void* oRESTRICT _pSource
	, size_t _SourceRowPitch
	, oSURFACE_FORMAT _SourceFormat
	, const int2& _MipDimensions);

// Extract the parameters for the above call directly from textures
bool oGPUSurfaceConvert(oGPUTexture* _pSourceTexture, oSURFACE_FORMAT _NewFormat, oGPUTexture** _ppDestinationTexture);

// Loads a texture from disk. The _Desc specifies certain conversions/resizes
// that can occur on load. Use oDEFAULT or oSURFACE_UNKNOWN to use values as 
// they are found in the specified image resource/buffer.
// @oooii-tony: At this time the implementation does NOT use oImage loading 
// code plus a simple call to call oGPUCreateTexture(). Because this API 
// supports conversion for any oSURFACE_FORMAT, at this time we defer to 
// DirectX's .dds support for advanced formats like BC6 and BC7 as well as their
// internal conversion library. When it's time to go cross-platform, we'll 
// revisit this and hopefully call more generic code.
bool oGPUTextureLoad(oGPUDevice* _pDevice, const oGPU_TEXTURE_DESC& _Desc, const char* _URIReference, const char* _DebugName, oGPUTexture** _ppTexture);
bool oGPUTextureLoad(oGPUDevice* _pDevice, const oGPU_TEXTURE_DESC& _Desc, const char* _DebugName, const void* _pBuffer, size_t _SizeofBuffer, oGPUTexture** _ppTexture);

enum oGPUEX_FILE_FORMAT
{
	oGPUEX_FILE_FORMAT_DDS,
	oGPUEX_FILE_FORMAT_JPG,
	oGPUEX_FILE_FORMAT_PNG,
};

// Saves a texture to disk. The format will be determined from the contents of 
// the texture. If the specified format does not support the contents of the 
// texture the function will return false - check oErrorGetLast() for extended
// information.
bool oGPUTextureSave(oGPUTexture* _pTexture, oGPUEX_FILE_FORMAT _Format, void* _pBuffer, size_t _SizeofBuffer);
bool oGPUTextureSave(oGPUTexture* _pTexture, oGPUEX_FILE_FORMAT _Format, const char* _Path);

#endif
