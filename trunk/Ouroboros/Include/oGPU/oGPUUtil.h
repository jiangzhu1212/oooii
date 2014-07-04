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
// Utility code that uses oGPU to implement extended functionality.
#pragma once
#ifndef oGPUUtil_h
#define oGPUUtil_h

#include <oGPU/oGPU.h>

#include <oMesh/obj.h>

namespace ouro {
	namespace gpu {

// _____________________________________________________________________________
// Buffer convenience functions

// Commit the contents of regular memory to device memory. Mainly this is 
// intended to be used with the templated types below.
void commit_buffer(command_list* _pCommandList, buffer* _pBuffer, const void* _pStruct, uint _SizeofStruct, uint _NumStructs);
void commit_buffer(device* _pDevice, buffer* _pBuffer, const void* _pStruct, uint _SizeofStruct, uint _NumStructs);
template<typename T> void commit_buffer(command_list* _pCommandList, buffer* _pBuffer, const T& _Struct) { commit_buffer(_pCommandList, _pBuffer, &_Struct, sizeof(_Struct), 1); }
template<typename T> void commit_buffer(command_list* _pCommandList, buffer* _pBuffer, const T* _pStructArray, uint _NumStructs) { commit_buffer(_pCommandList, _pBuffer, _pStructArray, sizeof(T), _NumStructs); }
template<typename T> void commit_buffer(device* _pDevice, buffer* _pBuffer, const T& _Struct) { commit_buffer(_pDevice, _pBuffer, &_Struct, sizeof(_Struct), 1); }
template<typename T> void commit_buffer(device* _pDevice, buffer* _pBuffer, const T* _pStructArray, uint _NumStructs) { commit_buffer(_pDevice, _pBuffer, _pStructArray, sizeof(T), _NumStructs); }

// Creates a readback buffer or texture sized to be able to completely contain the 
// specified source.
std::shared_ptr<buffer> make_readback_copy(buffer* _pSource);
std::shared_ptr<texture> make_readback_copy(texture* _pSource);

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
// is not an option, this must be called after Device::end_frame() to have valid 
// values.
uint read_back_counter(buffer* _pUnorderedBuffer, buffer* _pPreallocatedReadbackBuffer = nullptr);

// Reads the source resource into the memory pointed at in the destination struct. 
// Since this is often used for textures flip vertically can do an in-place/
// during-copy flip. This returns the result of map_read. 
bool read(resource* _pSourceResource, int _Subresource, ouro::surface::mapped_subresource& _Destination, bool _FlipVertically = false);

// _____________________________________________________________________________
// Texture convenience functions

// Creates a texture and fills it with source image data according to the type specified. 
// NOTE: render target and readback not tested.
std::shared_ptr<texture> make_texture(device* _pDevice, const char* _Name, const surface::buffer* const* _ppSourceImages, uint _NumImages, texture_type::value _Type);
inline std::shared_ptr<texture> make_texture(std::shared_ptr<device>& _pDevice, const char* _Name, const surface::buffer* const* _ppSourceImages, uint _NumImages, texture_type::value _Type) { return make_texture(_pDevice.get(), _Name, _ppSourceImages, _NumImages, _Type); }

// Creates a surface buffer and copies the specified subresource to it. If _Subresource is
// invalid all subresources are copied.
std::shared_ptr<surface::buffer> copy_to_surface_buffer(texture* _pSource, int _Subresource = invalid);

	} // namespace gpu
} // namespace ouro

#if 0

// Binds the readable (samplable) shader resources from a render target in order
inline void oGPURenderTargetSetShaderResources(command_list* _pCommandList, int _StartSlot, bool _IncludeDepthStencil, oGPURenderTarget* _pRenderTarget)
{
	oGPURenderTarget::DESC rtd;
	_pRenderTarget->GetDesc(&rtd);
	ouro::std::shared_ptr<texture> MRTs[ouro::gpu::max_num_mrts+1];
	int i = 0;
	for (; i < rtd.mrt_count; i++)
		_pRenderTarget->GetTexture(i, &MRTs[i]);
	if (_IncludeDepthStencil)
		_pRenderTarget->GetDepthTexture(&MRTs[i++]);
	_pCommandList->SetShaderResources(_StartSlot, i, (const resource* const*)MRTs);
}

// _____________________________________________________________________________
// Mesh convenience functions

bool oGPUReadVertexSource(int _Slot, int _NumVertices, ouro::surface::mapped_subresource& _Mapped, uint _NumElements, const oGPU_VERTEX_ELEMENT* _pElements, const oGeometry::DESC& _Desc, oGeometry::CONST_MAPPED& _GeoMapped);
bool oGPUReadVertexSource(int _Slot, int _NumVertices, ouro::surface::mapped_subresource& _Mapped, uint _NumElements, const oGPU_VERTEX_ELEMENT* _pElements, const threadsafe oOBJ* _pOBJ);

#endif
#endif