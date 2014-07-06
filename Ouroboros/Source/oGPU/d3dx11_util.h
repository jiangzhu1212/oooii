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
// Simplify commonly used D3DX11 API.
#pragma once
#ifndef oGPU_d3dx11_util_h
#define oGPU_d3dx11_util_h

#include <oSurface/buffer.h>
#include <oGPU/oGPU.h>
#include <oCore/windows/win_util.h>
#include <d3dx11.h>

namespace ouro {
	namespace gpu {
		namespace d3dx11 {

// Returns an IFF based on the extension specified in the file path
D3DX11_IMAGE_FILE_FORMAT from_path(const path& _Path);

void trace_image_load_info(const D3DX11_IMAGE_LOAD_INFO& _ImageLoadInfo, const char* _Prefix = "\t");

// Saves image to the specified file. The format is derived from the path's 
// extension. If the specified resource is not CPU-accessible a temporary copy
// is made. Remember, more exotic formats like BC formats are only supported by 
// DDS.
void save(ID3D11Resource* _pResource, const path& _Path);
void save(const surface::buffer* _pSurface, const path& _Path);
void save(ID3D11Resource* _pTexture, D3DX11_IMAGE_FILE_FORMAT _Format, void* _pBuffer, size_t _SizeofBuffer);
void save(const surface::buffer* _pSurface, D3DX11_IMAGE_FILE_FORMAT _Format, void* _pBuffer, size_t _SizeofBuffer);

// Creates a new texture by parsing _pBuffer as a D3DX11-supported file format
// Specify surface::unknown and 0 for x, y or array_size in the _Info to use 
// values from the specified file. If the type has mips then mips will be 
// allocated but not filled in.
intrusive_ptr<ID3D11Resource> load(ID3D11Device* _pDevice
	, const gpu::texture1_info& _Info
	, const char* _DebugName
	, const path& _Path);

intrusive_ptr<ID3D11Resource> load(ID3D11Device* _pDevice
	, const gpu::texture1_info& _Info
	, const char* _DebugName
	, const void* _pBuffer
	, size_t _SizeofBuffer);

// Uses GPU acceleration for BC6H and BC7 conversions if the source is in the 
// correct format. All other conversions go through D3DX11LoadTextureFromTexture
void convert(ID3D11Texture2D* _pSourceTexture, surface::format _NewFormat
	, ID3D11Texture2D** _ppDestinationTexture);

// Uses the above convert(), but the parameters could be CPU buffers. This means 
// there's a copy-in and a copy-out to set up the source and destination
// textures for the internal call.
void convert(ID3D11Device* _pDevice
	, surface::mapped_subresource& _Destination
	, surface::format _DestinationFormat
	, surface::const_mapped_subresource& _Source
	, surface::format _SourceFormat
	, const int2& _MipDimensions);

		} // namespace d3dx11
	} // namespace gpu
} // namespace ouro

#endif
