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

#include <oGPU/oGPU.h>

#include <oBasis/oGeometry.h>
#include <oMesh/obj.h>

namespace ouro {
	namespace gpu {

// _____________________________________________________________________________
// Buffer convenience functions

// Commit the contents of regular memory to device memory. Mainly this is 
// intended to be used with the templated types below.
void commit_buffer(oGPUCommandList* _pCommandList, oGPUBuffer* _pBuffer, const void* _pStruct, uint _SizeofStruct, uint _NumStructs);
void commit_buffer(oGPUDevice* _pDevice, oGPUBuffer* _pBuffer, const void* _pStruct, uint _SizeofStruct, uint _NumStructs);
template<typename T> void commit_buffer(oGPUCommandList* _pCommandList, oGPUBuffer* _pBuffer, const T& _Struct) { commit_buffer(_pCommandList, _pBuffer, &_Struct, sizeof(_Struct), 1); }
template<typename T> void commit_buffer(oGPUCommandList* _pCommandList, oGPUBuffer* _pBuffer, const T* _pStructArray, uint _NumStructs) { commit_buffer(_pCommandList, _pBuffer, _pStructArray, sizeof(T), _NumStructs); }
template<typename T> void commit_buffer(oGPUDevice* _pDevice, oGPUBuffer* _pBuffer, const T& _Struct) { commit_buffer(_pDevice, _pBuffer, &_Struct, sizeof(_Struct), 1); }
template<typename T> void commit_buffer(oGPUDevice* _pDevice, oGPUBuffer* _pBuffer, const T* _pStructArray, uint _NumStructs) { commit_buffer(_pDevice, _pBuffer, _pStructArray, sizeof(T), _NumStructs); }

// Commit _MappedSubresource to _pIndexBuffer handling 16-bit to 32-bit and vice 
// versa converstions between the source and destination.
void commit_index_buffer(oGPUCommandList* _pCommandList
	, const surface::const_mapped_subresource& _MappedSubresource
	, oGPUBuffer* _pIndexBuffer);

inline void commit_index_buffer(oGPUDevice* _pDevice
	, const surface::const_mapped_subresource& _MappedSubresource
	, oGPUBuffer* _pIndexBuffer)
{
	intrusive_ptr<oGPUCommandList> ICL;
	_pDevice->GetImmediateCommandList(&ICL);
	commit_index_buffer(ICL, _MappedSubresource, _pIndexBuffer);
}

// Client code must define a function that given a vertex trait returns a mapping
// of source data. If the function returns a mapping with a null data pointer,
// values for that vertex trait will be set to 0 in _pVertexBuffer. No conversions
// will be done in this function so if a trait is asked for in a compressed format
// the returned mapping is expected to be in that compressed format.
void commit_vertex_buffer(oGPUCommandList* _pCommandList, const mesh::layout::value& _Layout, const mesh::source& _Source, oGPUBuffer* _pVertexBuffer);
inline void commit_vertex_buffer(oGPUDevice* _pDevice, const mesh::layout::value& _Layout, const mesh::source& _Source, oGPUBuffer* _pVertexBuffer)
{
	intrusive_ptr<oGPUCommandList> ICL;
	_pDevice->GetImmediateCommandList(&ICL);
	commit_vertex_buffer(ICL, _Layout, _Source, _pVertexBuffer);
}

// Create an index buffer based on the parameters. If _MappedSubresource.pData
// is not null commit_index_buffer() is called on that data. If it is null then 
// only the creation is done.
intrusive_ptr<oGPUBuffer> make_index_buffer(oGPUDevice* _pDevice, const char* _Name, uint _NumIndices, uint _NumVertices
	, const surface::const_mapped_subresource& _MappedSubresource = surface::const_mapped_subresource());

// Creates a vertex buffer based on the parameters. If _GetElementData is valid 
// then commit_vertex_buffer is called.
intrusive_ptr<oGPUBuffer> make_vertex_buffer(oGPUDevice* _pDevice, const char* _Name, const mesh::layout::value& _Layout
	, uint _NumVertices, const mesh::source& _Source = mesh::source());

// Creates a vertex buffer based on the parameters. If _GetElementData is valid 
// oGPUCommitVertexBuffer, else only do the creation.
intrusive_ptr<oGPUBuffer> make_vertex_buffer(oGPUDevice* _pDevice, const char* _Name, const mesh::layout::value& _Layout
	, const oGeometry::DESC& _GeoDesc, const oGeometry::CONST_MAPPED& _GeoMapped);

// Creates a readback buffer or texture sized to be able to completely contain the 
// specified source.
intrusive_ptr<oGPUBuffer> make_readback_copy(oGPUBuffer* _pSource);
intrusive_ptr<oGPUTexture> make_readback_copy(oGPUTexture* _pSource);

// Optionally allocates a new buffer and reads the counter from the specified 
// buffer into it. For performance a buffer can be specified to receive the 
// counter value. The value is the read back to a uint using the immediate 
// command list. The purpose of this utility code is primarily to wrap the 
// lengthy code it takes to get the counter out for debugging/inspection 
// purposes and should not be used in production code since it synchronizes/
// stalls the device. This returns ouro::invalid on failure. REMEMBER THAT THIS MUST 
// BE CALLED AFTER A FLUSH OF ALL COMMANDLISTS THAT WOULD UPDATE THE COUNTER. 
// i.e. the entire app should use the immediate command list, otherwise this 
// could be sampling stale data. If using the immediate command list everywhere 
// is not an option, this must be called after Device::EndFrame() to have valid 
// values.
uint read_back_counter(oGPUBuffer* _pUnorderedBuffer, oGPUBuffer* _pPreallocatedReadbackBuffer = nullptr);

// Reads the source resource into the memory pointed at in the destination struct. 
// Since this is often used for textures flip vertically can do an in-place/
// during-copy flip. This returns the result of MapRead. 
bool read(oGPUResource* _pSourceResource, int _Subresource, ouro::surface::mapped_subresource& _Destination, bool _FlipVertically = false);

// _____________________________________________________________________________
// Texture convenience functions

// Creates a texture and fills it with source image data according to the type specified. 
// NOTE: render target and readback not tested.
intrusive_ptr<oGPUTexture> make_texture(oGPUDevice* _pDevice, const char* _Name, const surface::buffer* const* _ppSourceImages, uint _NumImages, texture_type::value _Type);

// Creates a surface buffer and copies the specified subresource to it. If _Subresource is
// invalid all subresources are copied.
std::shared_ptr<surface::buffer> copy_to_surface_buffer(oGPUTexture* _pSource, int _Subresource = invalid);

	} // namespace gpu
} // namespace ouro

#if 0

// Binds the readable (samplable) shader resources from a render target in order
inline void oGPURenderTargetSetShaderResources(oGPUCommandList* _pCommandList, int _StartSlot, bool _IncludeDepthStencil, oGPURenderTarget* _pRenderTarget)
{
	oGPURenderTarget::DESC rtd;
	_pRenderTarget->GetDesc(&rtd);
	ouro::intrusive_ptr<oGPUTexture> MRTs[ouro::gpu::max_num_mrts+1];
	int i = 0;
	for (; i < rtd.mrt_count; i++)
		_pRenderTarget->GetTexture(i, &MRTs[i]);
	if (_IncludeDepthStencil)
		_pRenderTarget->GetDepthTexture(&MRTs[i++]);
	_pCommandList->SetShaderResources(_StartSlot, i, (const oGPUResource* const*)MRTs);
}

// _____________________________________________________________________________
// Mesh convenience functions

bool oGPUReadVertexSource(int _Slot, int _NumVertices, ouro::surface::mapped_subresource& _Mapped, uint _NumElements, const oGPU_VERTEX_ELEMENT* _pElements, const oGeometry::DESC& _Desc, oGeometry::CONST_MAPPED& _GeoMapped);
bool oGPUReadVertexSource(int _Slot, int _NumVertices, ouro::surface::mapped_subresource& _Mapped, uint _NumElements, const oGPU_VERTEX_ELEMENT* _pElements, const threadsafe oOBJ* _pOBJ);

#endif
#endif