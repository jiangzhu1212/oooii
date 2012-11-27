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
#include <oGPU/oGPUEx.h>

static_assert(oGPU_MATERIAL_NUM_CHANNELS <= oGPU_MAX_NUM_MATERIAL_TEXTURES, "too many material channels");

const char* sMaterialChannelEnumStrings[] =
{
	"ambient",
	"diffuse",
	"specular",
	"emissive",
	"transmissive",
	"normal",
};

bool oFromString(oGPU_MATERIAL_CHANNEL* _pValue, const char* _StrSource)
{
	return oEnumFromString<oGPU_MATERIAL_NUM_CHANNELS>(_pValue, _StrSource, sMaterialChannelEnumStrings);
}

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oGPU_MATERIAL_CHANNEL& _Channel)
{
	oStrcpy(_StrDestination, _SizeofStrDestination, sMaterialChannelEnumStrings[_Channel]);
	return _StrDestination;
}

const char* oAsString(const oGPU_MATERIAL_CHANNEL& _Channel)
{
	switch (_Channel)
	{
		case oGPU_AMBIENT: return "oGPU_AMBIENT";
		case oGPU_DIFFUSE: return "oGPU_DIFFUSE";
		case oGPU_SPECULAR: return "oGPU_SPECULAR";
		case oGPU_EMISSIVE: return "oGPU_EMISSIVE";
		case oGPU_TRANSMISSIVE: return "oGPU_TRANSMISSIVE";
		case oGPU_NORMAL: return "oGPU_NORMAL";
		oNODEFAULT;
	}
}

oColor oGPUGetRepresentativeColor(oGPU_MATERIAL_CHANNEL _Channel, bool _ObjectSpaceNormals)
{
	switch (_Channel)
	{
		case oGPU_AMBIENT: return std::Black;
		case oGPU_DIFFUSE: return std::White;
		case oGPU_SPECULAR: return std::White;
		case oGPU_EMISSIVE: return std::Black;
		case oGPU_TRANSMISSIVE: return std::Black;
		case oGPU_NORMAL: return _ObjectSpaceNormals ? std::ObjectSpaceNormalGreen : std::TangentSpaceNormalBlue;
		oNODEFAULT;
	}
}

// @oooii-tony: FIXME: I am not quite sure where this belongs... Convert 
// doesn't belong in oSurface.h at the moment because that's in oBasis, which is 
// platform-independent code and this convert implementation is decidedly 
// platform-dependent. Conversion happens in a tool, which makes putting the 
// conversion in the renderer less appealing, even though currently conversion 
// requires a D3DDevice to function. So from oImage to oGPUEx is a very small 
// improvement until a better home reveals itself...
#include <oPlatform/Windows/oD3D11.h>
#include "oD3D11Texture.h"
#include "oD3D11Device.h"
bool oGPUSurfaceConvert(
	void* oRESTRICT _pDestination
	, size_t _DestinationRowPitch
	, oSURFACE_FORMAT _DestinationFormat
	, const void* oRESTRICT _pSource
	, size_t _SourceRowPitch
	, oSURFACE_FORMAT _SourceFormat
	, const int2& _MipDimensions)
{
	oGPU_DEVICE_INIT DeviceInit("oGPUSurfaceConvert Temp Device");
	oRef<ID3D11Device> D3DDevice;
	if (!oD3D11CreateDevice(DeviceInit, true, &D3DDevice))
	{
		DeviceInit.UseSoftwareEmulation = true;
		if (!oD3D11CreateDevice(DeviceInit, true, &D3DDevice))
			return false; // pass through error
	}

	oSURFACE_MAPPED_SUBRESOURCE Destination;
	Destination.pData = _pDestination;
	Destination.RowPitch = oUInt(_DestinationRowPitch);
	Destination.DepthPitch = 0;

	oSURFACE_CONST_MAPPED_SUBRESOURCE Source;
	Source.pData = _pSource;
	Source.RowPitch = oULLong(_SourceRowPitch);
	Source.DepthPitch = 0;
	return oD3D11Convert(D3DDevice, Destination, _DestinationFormat, Source, _SourceFormat, _MipDimensions);
}

bool oGPUSurfaceConvert(oGPUTexture* _pSourceTexture, oSURFACE_FORMAT _NewFormat, oGPUTexture** _ppDestinationTexture)
{
	oRef<ID3D11Texture2D> D3DDestinationTexture;
	if (!oD3D11Convert(static_cast<oD3D11Texture*>(_pSourceTexture)->Texture, _NewFormat, &D3DDestinationTexture))
		return false; // pass through error

	oRef<oGPUDevice> Device;
	_pSourceTexture->GetDevice(&Device);

	oGPUTexture::DESC d;
	oD3D11GetTextureDesc(D3DDestinationTexture, &d);

	bool success = false;
	oCONSTRUCT(_ppDestinationTexture, oD3D11Texture(Device, d, _pSourceTexture->GetName(), &success, D3DDestinationTexture));
	return success;
}

bool oGPUTextureLoad(oGPUDevice* _pDevice, const oGPU_TEXTURE_DESC& _Desc, const char* _URIReference, const char* _DebugName, oGPUTexture** _ppTexture)
{
	oURIParts URIParts;
	oSTREAM_DESC sd;

	if (!oStreamGetDesc(_URIReference, &sd, &URIParts))
		return oErrorSetLast(oERROR_NOT_FOUND, "%s not found", oSAFESTRN(_URIReference));

	if (oStricmp(URIParts.Scheme, "file"))
		return oErrorSetLast(oERROR_NOT_FOUND, "Currently only file schemed URIs are supported.");

	oRef<ID3D11Device> D3DDevice;
	oVERIFY(_pDevice->QueryInterface(oGetGUID<ID3D11Device>(), &D3DDevice));

	oRef<ID3D11Texture2D> D3DTexture;
	if (!oD3D11Load(D3DDevice, _Desc, URIParts.Path, _DebugName, (ID3D11Resource**)&D3DTexture))
		return false;

	bool success = false;
	oCONSTRUCT(_ppTexture, oD3D11Texture(_pDevice, (const oGPUTexture::DESC&)_Desc, _URIReference, &success, D3DTexture));
	return success;
}

bool oGPUTextureLoad(oGPUDevice* _pDevice, const oGPU_TEXTURE_DESC& _Desc, const char* _DebugName, const void* _pBuffer, size_t _SizeofBuffer, oGPUTexture** _ppTexture)
{
	oRef<ID3D11Device> D3DDevice;
	oVERIFY(_pDevice->QueryInterface(oGetGUID<ID3D11Device>(), &D3DDevice));

	oRef<ID3D11Texture2D> D3DTexture;
	if (!oD3D11Load(D3DDevice, _Desc, _DebugName, _pBuffer, _SizeofBuffer, (ID3D11Resource**)&D3DTexture))
		return false;

	bool success = false;
	oCONSTRUCT(_ppTexture, oD3D11Texture(_pDevice, (const oGPUTexture::DESC&)_Desc, _DebugName, &success, D3DTexture));
	return success;
}

D3DX11_IMAGE_FILE_FORMAT oD3D11IFFFromFileFormat(oGPUEX_FILE_FORMAT _Format)
{
	switch (_Format)
	{
		case oGPUEX_FILE_FORMAT_DDS: return D3DX11_IFF_DDS;
		case oGPUEX_FILE_FORMAT_JPG: return D3DX11_IFF_JPG;
		case oGPUEX_FILE_FORMAT_PNG: return D3DX11_IFF_PNG;
		oNODEFAULT;
	}
}

bool oGPUTextureSave(oGPUTexture* _pTexture, oGPUEX_FILE_FORMAT _Format, void* _pBuffer, size_t _SizeofBuffer)
{
	return oD3D11Save(static_cast<oD3D11Texture*>(_pTexture)->Texture, oD3D11IFFFromFileFormat(_Format), _pBuffer, _SizeofBuffer);
}

bool oGPUTextureSave(oGPUTexture* _pTexture, oGPUEX_FILE_FORMAT _Format, const char* _Path)
{
	return oD3D11Save(static_cast<oD3D11Texture*>(_pTexture)->Texture, oD3D11IFFFromFileFormat(_Format), _Path);
}
