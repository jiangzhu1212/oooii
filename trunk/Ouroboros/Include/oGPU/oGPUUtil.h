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
// Utility code that uses oGPU_ to implement extended functionality.
#pragma once
#ifndef oGPUUtil_h
#define oGPUUtil_h

#include <oPlatform/oImage.h>
#include <oGPU/oGPU.h>
#include <oGPU/oGPUMaterialConstants.h>

// Returns the size in bytes of the sum of all vertex elements for the specified 
// input slot.
oAPI int oGPUCalculateVertexSize(const oGPU_VERTEX_ELEMENT* _pElements, size_t _NumElements, int _InputSlot);
template<size_t size> int oGPUCalculateVertexSize(const oGPU_VERTEX_ELEMENT (&_pElements)[size], int _InputSlot) { return oGPUCalculateVertexSize(_pElements, size, _InputSlot); }

oAPI int oGPUCalculateNumInputSlots(const oGPU_VERTEX_ELEMENT* _pElements, size_t _NumElements);
template<size_t size> int oGPUCalculateNumInputSlots(const oGPU_VERTEX_ELEMENT (&_pElements)[size]) { return oGPUCalculateNumInputSlots(_pElements, size); }

oAPI void oGPUCopyIndices(oSURFACE_MAPPED_SUBRESOURCE& _Destination, const oSURFACE_CONST_MAPPED_SUBRESOURCE& _Source, uint _NumIndices);

struct oGPU_VERTEX_ELEMENT_DATA
{
	const void* pData;
	size_t Stride;
};

// Creates a mesh from the specified oGeometry using the specified elements.
// If semantics and input slots match, data is read from the file. An extra 
// array of void* pointers to vertex element data can be specified with
// _ppElementData to override or compliment data from the file. For example, if 
// you have a custom semantic that needs data - say COLOR2 - then you can assign 
// a pointer to a buffer with that data at the same index into _ppElementData as
// the associated IAElement. Because that data is not found in the oGeometry,
// the pointer specified will be used instead. Assign nullptr to all other 
// elements that don't need data overrides. If _NormalScale is negative, then 
// the bounding sphere inscribed in the LocalBounds of oGPUMesh::DESC is 
// calculated and then scaled by the absolute value of _NormalScale.
oAPI bool oGPUCreateMesh(oGPUDevice* _pDevice, const char* _MeshName, const oGPU_VERTEX_ELEMENT* _pElements, uint _NumElements, const oGPU_VERTEX_ELEMENT_DATA* _ppElementData, const oGeometry* _pGeometry, oGPUMesh** _ppMesh, oGPULineList** _ppNormalLines = nullptr, float _NormalScale = 1.0f, oColor _NormalColor = std::White);
oAPI bool oGPUCreateMesh(oGPUDevice* _pDevice, const char* _MeshName, const oGPU_VERTEX_ELEMENT* _pElements, uint _NumElements, const oGPU_VERTEX_ELEMENT_DATA* _ppElementData, const threadsafe oOBJ* _pOBJ, oGPUMesh** _ppMesh, oGPULineList** _ppNormalLines = nullptr, float _NormalScale = 1.0f, oColor _NormalColor = std::White);

// This overwrites select values that are supported by OBJ's MTL format. Any 
// OBJ-unsupported field is left untouched.
oAPI void oGPUInitMaterialConstants(const oOBJ_MATERIAL& _OBJMaterial, oGPUMaterialConstants* _pMaterialConstants);

// Creates a texture from the specified oImage(s), preserving its format and size.
oAPI bool oGPUCreateTexture1D(oGPUDevice* _pDevice, const oImage* _pSourceImage, oGPUTexture** _ppTexture);
oAPI bool oGPUCreateTexture2D(oGPUDevice* _pDevice, const oImage* _pSourceImage, oGPUTexture** _ppTexture);
oAPI bool oGPUCreateTexture3D(oGPUDevice* _pDevice, const oImage** _pSourceImages, uint _NumImages, oGPUTexture** _ppTexture);
oAPI bool oGPUCreateTextureCube(oGPUDevice* _pDevice, const oImage** _pSourceImages, uint _NumImages, oGPUTexture** _ppTexture);

// Creates a texture from the specified oBuffer, described by an oSURFACE_DESC.
oAPI bool oGPUCreateTexture1D(oGPUDevice* _pDevice, oSURFACE_DESC& _SurfaceDesc, const oBuffer* _pBuffer, oGPUTexture** _ppTexture);
oAPI bool oGPUCreateTexture2D(oGPUDevice* _pDevice, oSURFACE_DESC& _SurfaceDesc, const oBuffer* _pBuffer, oGPUTexture** _ppTexture);
oAPI bool oGPUCreateTexture3D(oGPUDevice* _pDevice, oSURFACE_DESC& _SurfaceDesc, const oBuffer* _pBuffer, oGPUTexture** _ppTexture);
oAPI bool oGPUCreateTextureCube(oGPUDevice* _pDevice, oSURFACE_DESC& _SurfaceDesc, const oBuffer* _pBuffer, oGPUTexture** _ppTexture);

// Generate mips with the GPU from source oImage(s) to an already created oGPUTexture with oSURFACE_DESC and oGPU_TEXTURE_TYPE
oAPI bool oGPUGenerateMips(oGPUDevice* _pDevice, const oImage** _pMip0Images, uint _NumImages, oGPUTexture* _pOutputTexture);

// Generate mips with the GPU from source oImage(s) to a preallocated oBuffer with oSURFACE_DESC and oGPU_TEXTURE_TYPE
oAPI bool oGPUGenerateMips(oGPUDevice* _pDevice, const oImage** _pMip0Images, uint _NumImages, oSURFACE_DESC& _SurfaceDesc, oGPU_TEXTURE_TYPE _Type, oBuffer* _pMipBuffer);

// Creates a texture with mips from the specified oImage(s), preserving its format and size and auto generates all the lower mip levels.
oAPI bool oGPUCreateTexture1DMip(oGPUDevice* _pDevice, const oImage* _pMip0Image, oGPUTexture** _ppTexture);
oAPI bool oGPUCreateTexture2DMip(oGPUDevice* _pDevice, const oImage* _pMip0Image, oGPUTexture** _ppTexture);
oAPI bool oGPUCreateTexture3DMip(oGPUDevice* _pDevice, const oImage** _pMip0Images, uint _NumImages, oGPUTexture** _ppTexture);
oAPI bool oGPUCreateTextureCubeMip(oGPUDevice* _pDevice, const oImage** _pMip0Images, uint _NumImages, oGPUTexture** _ppTexture);

// Creates a texture with mips from the specified oBuffer, described by an oSURFACE_DESC.
oAPI bool oGPUCreateTexture1DMip(oGPUDevice* _pDevice, oSURFACE_DESC& _SurfaceDesc, const oBuffer* _pBuffer, oGPUTexture** _ppTexture);
oAPI bool oGPUCreateTexture2DMip(oGPUDevice* _pDevice, oSURFACE_DESC& _SurfaceDesc, const oBuffer* _pBuffer, oGPUTexture** _ppTexture);
oAPI bool oGPUCreateTexture3DMip(oGPUDevice* _pDevice, oSURFACE_DESC& _SurfaceDesc, const oBuffer* _pBuffer, oGPUTexture** _ppTexture);
oAPI bool oGPUCreateTextureCubeMip(oGPUDevice* _pDevice, oSURFACE_DESC& _SurfaceDesc, const oBuffer* _pBuffer, oGPUTexture** _ppTexture);

oAPI void oGPUCommitBuffer(oGPUCommandList* _pCommandList, oGPUBuffer* _pBuffer, const void* _pStruct, uint _SizeofStruct, uint _NumStructs = 1);
oAPI void oGPUCommitBuffer(oGPUDevice* _pDevice, oGPUBuffer* _pBuffer, const void* _pStruct, uint _SizeofStruct, uint _NumStructs);

template<typename T> void oGPUCommitBuffer(oGPUCommandList* _pCommandList, oGPUBuffer* _pBuffer, const T& _Struct) { oGPUCommitBuffer(_pCommandList, _pBuffer, &_Struct, sizeof(_Struct), 1); }
template<typename T> void oGPUCommitBuffer(oGPUCommandList* _pCommandList, oGPUBuffer* _pBuffer, const T* _pStructArray, uint _NumStructs) { oGPUCommitBuffer(_pCommandList, _pBuffer, _pStructArray, sizeof(T), _NumStructs); }

template<typename T> void oGPUCommitBuffer(oGPUDevice* _pDevice, oGPUBuffer* _pBuffer, const T& _Struct) { oGPUCommitBuffer(_pDevice, _pBuffer, &_Struct, sizeof(_Struct), 1); }
template<typename T> void oGPUCommitBuffer(oGPUDevice* _pDevice, oGPUBuffer* _pBuffer, const T* _pStructArray, uint _NumStructs) { oGPUCommitBuffer(_pDevice, _pBuffer, _pStructArray, sizeof(T), _NumStructs); }

// Creates a readback buffer sized to be able to completely contain the 
// specified source.
bool oGPUCreateReadbackCopy(oGPUBuffer* _pSource, oGPUBuffer** _ppReadbackCopy);

// Optionally allocates a new buffer and reads the counter from the specified 
// buffer into it. For performance a buffer can be specified to receive the 
// counter value. The value is the read back to a uint using the immediate 
// command list. The purpose of this utility code is primarily to wrap the 
// lengthy code it takes to get the counter out for debugging/inspection 
// purposes and should not be used in production code since it synchronizes/
// stalls the device. This returns oInvalid on failure. REMEMBER THAT THIS MUST 
// BE CALLED AFTER A FLUSH OF ALL COMMANDLISTS THAT WOULD UPDATE THE COUNTER. 
// i.e. the entire app should use the immediate command list, otherwise this 
// could be sampling stale data. If using the immediate command list everywhere 
// is not an option, this must be called after Device::EndFrame() to have valid 
// values.
uint oGPUReadbackCounter(oGPUBuffer* _pUnorderedBuffer, oGPUBuffer* _pPreallocatedReadbackBuffer = nullptr);

// Reads the source resource into the memory pointed at in the destination
// struct. Since this is often used for textures, flip vertically can do an
// in-place/during-copy flip.
bool oGPURead(oGPUResource* _pSourceResource, int _Subresource, oSURFACE_MAPPED_SUBRESOURCE& _Destination, bool _FlipVertically);

// Creates an oImage from the specified texture. If the specified texture is not
// a readback texture, then a copy is made.
bool oGPUSaveImage(oGPUTexture* _pTexture, int _Subresource, interface oImage** _ppImage);

// Given a list of bytecode buffers create a pipeline. This is most useful when
// compiling shaders dynamically rather than using the static-compilation path.
bool oGPUPipelineCreate(oGPUDevice* _pDevice, const char* _Name, oRef<oBuffer> _ByteCode[oGPU_PIPELINE_NUM_STAGES], const oGPU_VERTEX_ELEMENT* _pElements, size_t _NumElements, oGPUPipeline** _ppPipeline);


// Compiles an .opl file (like an .FX file)
// In otherwise-hlsl source, insert an XML file that self-describes how the
// source should be build. Integration of shader compilers into IDEs is still
// poor and doesn't support permutating well, so keep all the build info out
// of the main build system and controlled by something else.
// Example:
/*
	<oPL>
		<CompilationEnvironment
			IncludePaths="%DEV%/Ouroboros/Include;%DEV%/Ouroboros/Source/oGPU/"
			Defines="oHLSL;oUsePhong=0;oUseParallax=1"
			ShaderModel="4"
		/>

		<Pipeline 
			Name="FwdLine"
			VS="oGPULinesVS"
			PS="oGPULinesPS"
		/>

	</oPL>
*/
// Note that there is a main <oPL> section, then a CompilationEnvironment node
// that describes IncludePaths (semi-colon delimited), Defines (semi-colon 
// delimited) and the shader model to target. After that there are any number
// of Pipeline nodes that have a name, then VS, HS, DS, GS, PS attributes that
// define an entry point for each stage of the pipeline. Then pass the file to
// oGPU_CompilePipeline here with oGPU_VERTEX_ELEMENTs to create an array of 
// pipelines.
uint oGPUPipelineCompile(oGPUDevice* _pDevice, const char* _PipelineSourcePath, oGPUPipeline** _ppPipelines, uint _MaxNumPipelines);
inline uint oGPUPipelineCompile(oGPUDevice* _pDevice, const char* _PipelineSourcePath, oRef<oGPUPipeline>* _ppPipelines, uint _MaxNumPipelines) { return ::oGPUPipelineCompile(_pDevice, _PipelineSourcePath, (oGPUPipeline**)_ppPipelines, _MaxNumPipelines); }

interface oGPUMosaic : oInterface
{
	// Sets up an oGeometry::MOSAIC mesh and renders it in a simple clip-space,
	// non-z-tested, opaque state. This is useful in setting up image and video
	// players. This is more than a full-screen quad because of the more complex
	// requirements of video wall presentation where the logical screen might be 
	// made up of several physical screen.

	virtual bool Rebuild(const oGeometryFactory::MOSAIC_DESC& _Desc) = 0;
	virtual void SetBlendState(oGPU_BLEND_STATE _BlendState) = 0; //will be oGPU_OPAQUE by default

	virtual void Draw(oGPUCommandList* _pCommandList, oGPURenderTarget* _pRenderTarget, uint _TextureStartSlot, uint _NumTextures, const oGPUTexture* const* _ppTextures) = 0;
	inline void Draw(oGPUCommandList* _pCommandList, oGPURenderTarget* _pRenderTarget, uint _TextureStartSlot, uint _NumTextures, const oRef<oGPUTexture>* _ppTextures) { Draw(_pCommandList, _pRenderTarget, _TextureStartSlot, _NumTextures, (const oGPUTexture* const*)_ppTextures); }
};

bool oGPUMosaicCreate(oGPUDevice* _pDevice, const void* _pPixelShaderByteCode, oGPUMosaic** _ppMosaic);

#endif
