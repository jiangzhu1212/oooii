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

#endif
#endif