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
#include <oCore/filesystem.h>
#include "d3d11_util.h"
#include "d3dx11_util.h"

using namespace ouro;
using namespace ouro::gpu;
using namespace ouro::gpu::d3d11;
using namespace ouro::gpu::d3dx11;

#if 0

bool oGPUSurfaceConvert(
	void* oRESTRICT _pDestination
	, size_t _DestinationRowPitch
	, ouro::surface::format _DestinationFormat
	, const void* oRESTRICT _pSource
	, size_t _SourceRowPitch
	, ouro::surface::format _SourceFormat
	, const int2& _MipDimensions)
{
	ouro::gpu::device_init DeviceInit("oGPUSurfaceConvert Temp Device");
	intrusive_ptr<ID3D11Device> D3DDevice;
	try
	{
		D3DDevice = make_device(DeviceInit);
	}
	catch (std::exception&)
	{
		DeviceInit.use_software_emulation = true;
		D3DDevice = make_device(DeviceInit);
	}

	ouro::surface::mapped_subresource Destination;
	Destination.data = _pDestination;
	Destination.row_pitch = as_int(_DestinationRowPitch);
	Destination.depth_pitch = 0;

	ouro::surface::const_mapped_subresource Source;
	Source.data = _pSource;
	Source.row_pitch = as_int(_SourceRowPitch);
	Source.depth_pitch = 0;
	convert(D3DDevice, Destination, _DestinationFormat, Source, _SourceFormat, _MipDimensions);
	return true;
}

bool oGPUSurfaceConvert(texture* _pSourceTexture, surface::format _NewFormat, texture** _ppDestinationTexture)
{
	intrusive_ptr<ID3D11Texture2D> D3DDestinationTexture;
	convert(static_cast<d3d11_texture*>(_pSourceTexture)->pTexture2D, _NewFormat, &D3DDestinationTexture);

	std::shared_ptr<device> Device = _pSourceTexture->get_device();

	texture_info i = get_texture_info(D3DDestinationTexture);

	bool success = false;
	oCONSTRUCT(_ppDestinationTexture, oD3D11Texture(Device, i, _pSourceTexture->name(), D3DDestinationTexture));
	return success;
}

bool oGPUTextureLoad(oGPUDevice* _pDevice, const ouro::gpu::texture_info& _Desc, const char* _URIReference, const char* _DebugName, oGPUTexture** _ppTexture)
{
	uri u(_URIReference);
	if (_stricmp(u.scheme(), "file"))
		return oErrorSetLast(std::errc::not_supported, "Currently only file schemed URIs are supported.");

	path p(u.path());

	if (!ouro::filesystem::exists(p))
		return oErrorSetLast(std::errc::no_such_file_or_directory, "%s not found", oSAFESTRN(_URIReference));

	intrusive_ptr<ID3D11Device> D3DDevice;
	oVERIFY(_pDevice->QueryInterface(&D3DDevice));

	intrusive_ptr<ID3D11Resource> D3DTexture = load(D3DDevice, _Desc, _DebugName, p);

	bool success = false;
	oCONSTRUCT(_ppTexture, oD3D11Texture(_pDevice, (const oGPUTexture::DESC&)_Desc, _URIReference, &success, D3DTexture));
	return success;
}

bool oGPUTextureLoad(oGPUDevice* _pDevice, const ouro::gpu::texture_info& _Desc, const char* _DebugName, const void* _pBuffer, size_t _SizeofBuffer, oGPUTexture** _ppTexture)
{
	intrusive_ptr<ID3D11Device> D3DDevice;
	oVERIFY(_pDevice->QueryInterface(&D3DDevice));
	intrusive_ptr<ID3D11Resource> D3DTexture = load(D3DDevice, _Desc, _DebugName, _pBuffer, _SizeofBuffer);
	bool success = false;
	oCONSTRUCT(_ppTexture, oD3D11Texture(_pDevice, (const oGPUTexture::DESC&)_Desc, _DebugName, &success, D3DTexture));
	return success;
}

D3DX11_IMAGE_FILE_FORMAT oD3D11IFFFromFileFormat(oGPU_FILE_FORMAT _Format)
{
	switch (_Format)
	{
		case oGPU_FILE_FORMAT_DDS: return D3DX11_IFF_DDS;
		case oGPU_FILE_FORMAT_JPG: return D3DX11_IFF_JPG;
		case oGPU_FILE_FORMAT_PNG: return D3DX11_IFF_PNG;
		oNODEFAULT;
	}
}

bool oGPUTextureSave(oGPUTexture* _pTexture, oGPU_FILE_FORMAT _Format, void* _pBuffer, size_t _SizeofBuffer)
{
	save(static_cast<oD3D11Texture*>(_pTexture)->Texture, oD3D11IFFFromFileFormat(_Format), _pBuffer, _SizeofBuffer);
	return true;
}

bool oGPUTextureSave(oGPUTexture* _pTexture, oGPU_FILE_FORMAT _Format, const char* _Path)
{
	save(static_cast<oD3D11Texture*>(_pTexture)->Texture, _Path);
	return true;
}
#endif