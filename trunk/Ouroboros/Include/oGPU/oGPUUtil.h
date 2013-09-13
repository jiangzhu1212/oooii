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
// Utility code that uses oGPU_ to implement extended functionality.
#pragma once
#ifndef oGPUUtil_h
#define oGPUUtil_h

#include <oPlatform/oImage.h>
#include <oGPU/oGPU.h>

// _____________________________________________________________________________
// Buffer convenience functions

// Fills a desc for an index buffer that decides on index size (ushort or 
// uint) based on the number of vertices to be indexed.
oAPI void oGPUGetIndexBufferDesc(uint _NumIndices, uint _NumVertices, oGPU_BUFFER_DESC* _pDesc);

// Fills a desc for a vertex buffer that can contain the specified elements
oAPI void oGPUGetVertexBufferDesc(uint _NumVertices, const oGPU_VERTEX_ELEMENT* _pElements, uint _NumElements, uint _InputSlot, oGPU_BUFFER_DESC* _pDesc);

// Copies indices to/from either ushorts or uints. This compares the RowPitches 
// and does the proper swizzle copy.
oAPI void oGPUCopyIndices(oSURFACE_MAPPED_SUBRESOURCE& _Destination
	, const oSURFACE_CONST_MAPPED_SUBRESOURCE& _Source
	, uint _NumIndices);

// Commit the specified mapped subresource to the specified index buffer, 
// regardless of whether its ushort or uint source/destinations.
oAPI void oGPUCommitIndexBuffer(oGPUCommandList* _pCommandList
	, const oSURFACE_CONST_MAPPED_SUBRESOURCE& _MappedSubresource
	, oGPUBuffer* _pIndexBuffer);

// Commit memory to the specified vertex buffer whose layout is described by the
// vertex elements. This is done by doing a swizzle copy into the buffer from a 
// source described by GetElementData. For each element that callback is called
// to get the copyable topology description. _GetElementData will receive an 
// element and the function should fill _pElementData with a pointer to the 
// first element to copy and RowPitch should be the stride to the next element.
// This is to support copying from array-of-structures (AoS) sources. DepthPitch
// is ignored.
oAPI bool oGPUCommitVertexBuffer(oGPUCommandList* _pCommandList
	, const oFUNCTION<void(const oGPU_VERTEX_ELEMENT& _Element, oSURFACE_CONST_MAPPED_SUBRESOURCE* _pElementData)>& _GetElementData
	, const oGPU_VERTEX_ELEMENT* _pElements
	, uint _NumElements
	, uint _InputSlot
	, oGPUBuffer* _pVertexBuffer);

// Commit the contents of regular memory to device memory. Mainly this is 
// intended to be used with the templated types below.
oAPI void oGPUCommitBuffer(oGPUCommandList* _pCommandList, oGPUBuffer* _pBuffer, const void* _pStruct, uint _SizeofStruct, uint _NumStructs = 1);
oAPI void oGPUCommitBuffer(oGPUDevice* _pDevice, oGPUBuffer* _pBuffer, const void* _pStruct, uint _SizeofStruct, uint _NumStructs = 1);

template<typename T> void oGPUCommitBuffer(oGPUCommandList* _pCommandList, oGPUBuffer* _pBuffer, const T& _Struct) { oGPUCommitBuffer(_pCommandList, _pBuffer, &_Struct, sizeof(_Struct), 1); }
template<typename T> void oGPUCommitBuffer(oGPUCommandList* _pCommandList, oGPUBuffer* _pBuffer, const T* _pStructArray, uint _NumStructs) { oGPUCommitBuffer(_pCommandList, _pBuffer, _pStructArray, sizeof(T), _NumStructs); }

template<typename T> void oGPUCommitBuffer(oGPUDevice* _pDevice, oGPUBuffer* _pBuffer, const T& _Struct) { oGPUCommitBuffer(_pDevice, _pBuffer, &_Struct, sizeof(_Struct), 1); }
template<typename T> void oGPUCommitBuffer(oGPUDevice* _pDevice, oGPUBuffer* _pBuffer, const T* _pStructArray, uint _NumStructs) { oGPUCommitBuffer(_pDevice, _pBuffer, _pStructArray, sizeof(T), _NumStructs); }

// Create an index buffer based on the parameters. If _MappedSubresource.pData
// is not null, oGPUCommitIndexBuffer() is called on that data. If it is null,
// then only the creation is done.
oAPI bool oGPUCreateIndexBuffer(oGPUDevice* _pDevice
	, const char* _Name
	, uint _NumIndices
	, uint _NumVertices
	, const oSURFACE_CONST_MAPPED_SUBRESOURCE& _MappedSubresource
	, oGPUBuffer** _ppIndexBuffer);

// Creates a vertex buffer based on the parameters. If _GetElementData is 
// valid (if (!!_GetElementData) { doit(); }) then use it to call 
// oGPUCommitVertexBuffer, else only do the creation.
oAPI bool oGPUCreateVertexBuffer(oGPUDevice* _pDevice
	, const char* _Name	
	, uint _NumVertices
	, const oFUNCTION<void(const oGPU_VERTEX_ELEMENT& _Element, oSURFACE_CONST_MAPPED_SUBRESOURCE* _pElementData)>& _GetElementData
	, uint _NumElements
	, const oGPU_VERTEX_ELEMENT* _pElements
	, uint _InputSlot
	, oGPUBuffer** _ppVertexBuffer);

// Creates a vertex buffer based on the parameters. If _GetElementData is 
// valid (if (!!_GetElementData) { doit(); }) then use it to call 
// oGPUCommitVertexBuffer, else only do the creation.
oAPI bool oGPUCreateVertexBuffer(oGPUDevice* _pDevice
	, const char* _Name	
	, uint _NumVertices
	, const oGeometry::DESC& _GeoDesc
	, const oGeometry::CONST_MAPPED& _GeoMapped
	, uint _NumElements
	, const oGPU_VERTEX_ELEMENT* _pElements
	, uint _InputSlot
	, oGPUBuffer** _ppVertexBuffer);

// Creates a readback buffer sized to be able to completely contain the 
// specified source.
oAPI bool oGPUCreateReadbackCopy(oGPUBuffer* _pSource, oGPUBuffer** _ppReadbackCopy);

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
oAPI uint oGPUReadbackCounter(oGPUBuffer* _pUnorderedBuffer, oGPUBuffer* _pPreallocatedReadbackBuffer = nullptr);

// Reads the source resource into the memory pointed at in the destination
// struct. Since this is often used for textures, flip vertically can do an
// in-place/during-copy flip.
oAPI bool oGPURead(oGPUResource* _pSourceResource, int _Subresource, oSURFACE_MAPPED_SUBRESOURCE& _Destination, bool _FlipVertically);

// _____________________________________________________________________________
// Texture convenience functions

// Creates a texture and fills it with source image data according to the type
// specified. NOTE: render target and readback not tested.
oAPI bool oGPUCreateTexture(oGPUDevice* _pDevice, const oImage* const* _ppSourceImages, uint _NumImages, oGPU_TEXTURE_TYPE _Type, oGPUTexture** _ppTexture);

// Fill in the rest of the mip chain for the specified texture. It's mip0 level
// should be fully initialized.
oAPI bool oGPUGenerateMips(oGPUDevice* _pDevice, oGPUTexture* _pTexture);

// Generate mips with the GPU from source oImage(s) to a preallocated oBuffer with oSURFACE_DESC and oGPU_TEXTURE_TYPE
oAPI bool oGPUGenerateMips(oGPUDevice* _pDevice, const oImage** _pMip0Images, uint _NumImages, oSURFACE_DESC& _SurfaceDesc, oGPU_TEXTURE_TYPE _Type, oBuffer* _pMipBuffer);

// Creates a texture with mips from the specified oBuffer, described by an oSURFACE_DESC.
oAPI bool oGPUCreateTexture1DMip(oGPUDevice* _pDevice, oSURFACE_DESC& _SurfaceDesc, const oBuffer* _pBuffer, oGPUTexture** _ppTexture);
oAPI bool oGPUCreateTexture2DMip(oGPUDevice* _pDevice, oSURFACE_DESC& _SurfaceDesc, const oBuffer* _pBuffer, oGPUTexture** _ppTexture);
oAPI bool oGPUCreateTexture3DMip(oGPUDevice* _pDevice, oSURFACE_DESC& _SurfaceDesc, const oBuffer* _pBuffer, oGPUTexture** _ppTexture);
oAPI bool oGPUCreateTextureCubeMip(oGPUDevice* _pDevice, oSURFACE_DESC& _SurfaceDesc, const oBuffer* _pBuffer, oGPUTexture** _ppTexture);

// Binds the readable (samplable) shader resources from a render target in order
inline void oGPURenderTargetSetShaderResources(oGPUCommandList* _pCommandList, int _StartSlot, bool _IncludeDepthStencil, oGPURenderTarget* _pRenderTarget)
{
	oGPU_RENDER_TARGET_DESC rtd;
	_pRenderTarget->GetDesc(&rtd);
	oStd::ref<oGPUTexture> MRTs[oGPU_MAX_NUM_MRTS+1];
	int i = 0;
	for (; i < rtd.MRTCount; i++)
		_pRenderTarget->GetTexture(i, &MRTs[i]);
	if (_IncludeDepthStencil)
		_pRenderTarget->GetDepthTexture(&MRTs[i++]);
	_pCommandList->SetShaderResources(_StartSlot, i, (const oGPUResource* const*)MRTs);
}

// Creates an oImage from the specified texture. If the specified texture is not
// a readback texture, then a copy is made.
bool oGPUSaveImage(oGPUTexture* _pTexture, int _Subresource, interface oImage** _ppImage);

// Given a list of bytecode buffers create a pipeline. This is most useful when
// compiling shaders dynamically rather than using the static-compilation path.
bool oGPUPipelineCreate(oGPUDevice* _pDevice, const char* _Name, oStd::ref<oBuffer> _ByteCode[oGPU_PIPELINE_STAGE_COUNT], const oGPU_VERTEX_ELEMENT* _pElements, size_t _NumElements, oGPU_PRIMITIVE_TYPE _InputType, oGPUPipeline** _ppPipeline);

// _____________________________________________________________________________
// Mesh convenience functions

oAPI bool oGPUReadVertexSource(int _Slot, int _NumVertices, oSURFACE_MAPPED_SUBRESOURCE& _Mapped, uint _NumElements, const oGPU_VERTEX_ELEMENT* _pElements, const oGeometry::DESC& _Desc, oGeometry::CONST_MAPPED& _GeoMapped);
oAPI bool oGPUReadVertexSource(int _Slot, int _NumVertices, oSURFACE_MAPPED_SUBRESOURCE& _Mapped, uint _NumElements, const oGPU_VERTEX_ELEMENT* _pElements, const threadsafe oOBJ* _pOBJ);

// {D12BFBB0-28EE-4E0A-BFF1-EAC858BB3AD3}
oDEFINE_GUID_I(oGPUUtilMesh, 0xd12bfbb0, 0x28ee, 0x4e0a, 0xbf, 0xf1, 0xea, 0xc8, 0x58, 0xbb, 0x3a, 0xd3);
interface oGPUUtilMesh : oInterface
{
	typedef oGPU_MESH_DESC DESC;
	virtual void GetDesc(DESC* _pDesc) const = 0;
	virtual uint GetNumPrimitives() const = 0;
	virtual const oGPUBuffer* GetIndexBuffer() const = 0;
	virtual oGPUBuffer* GetIndexBuffer() = 0;
	virtual const oGPUBuffer* GetVertexBuffer() const = 0;
	virtual oGPUBuffer* GetVertexBuffer() = 0;
};

oAPI bool oGPUUtilMeshCreate(oGPUDevice* _pDevice, const char* _Name, const oGPUUtilMesh::DESC& _Desc, uint _NumPrimitives, oGPUUtilMesh** _ppMesh);
oAPI void oGPUUtilMeshDraw(oGPUCommandList* _pCommandList, const oGPUUtilMesh* _pMesh);

// Creates a mesh from the specified oGeometry using the specified elements.
// If semantics and input slots match, data is read from the file. An extra 
// array of void* pointers to vertex element data can be specified with
// _ppElementData to override or compliment data from the file. For example, if 
// you have a custom semantic that needs data - say COLOR2 - then you can assign 
// a pointer to a buffer with that data at the same index into _ppElementData as
// the associated IAElement. Because that data is not found in the oGeometry,
// the pointer specified will be used instead. Assign nullptr to all other 
// elements that don't need data overrides. If _NormalScale is negative, then 
// the bounding sphere inscribed in the LocalBounds of oGPUUtilMesh::DESC is 
// calculated and then scaled by the absolute value of _NormalScale.
oAPI bool oGPUUtilMeshCreate(oGPUDevice* _pDevice, const char* _MeshName, const oGPU_VERTEX_ELEMENT* _pElements, uint _NumElements, const oGeometry* _pGeometry, oGPUUtilMesh** _ppMesh);

// Creates a very simple front-facing triangle that can be rendered with all-
// identify world, view, projection matrices. This is useful for very simple 
// tests and first bring-up.
oAPI bool oGPUUtilCreateFirstTriangle(oGPUDevice* _pDevice
	, const oGPU_VERTEX_ELEMENT* _pElements
	, uint _NumElements
	, oGPUUtilMesh** _ppFirstTriangle);

// Creates a very simple unit cube. This is useful for bringing up world, view,
// projection transforms quickly.
oAPI bool oGPUUtilCreateFirstCube(oGPUDevice* _pDevice
	, const oGPU_VERTEX_ELEMENT* _pElements
	, uint _NumElements
	, oGPUUtilMesh** _ppFirstCube);

#endif
