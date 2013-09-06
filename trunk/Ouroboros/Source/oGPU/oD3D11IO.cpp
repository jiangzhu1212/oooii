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
#include <oGPU/oGPU.h>
#include <oPlatform/Windows/oD3D11.h>
#include "oD3D11Texture.h"
#include "oD3D11Device.h"

// @oooii-tony: Here's a flavor that works on a pre-loaded source since we want 
// to support more than one shader entry. Right now to keep it simple, the 
// include system still hits the file system for each header. How can we get 
// around that? Until we can, loading source from a buffer doesn't add too much 
// benefit.
bool oGPUCompileShader(
	const char* _IncludePaths
	, const char* _CommonDefines
	, const char* _SpecificDefines
	, const oStd::version& _TargetShaderModel
	, oGPU_PIPELINE_STAGE _Stage
	, const char* _ShaderPath
	, const char* _EntryPoint
	, const char* _ShaderBody
	, oBuffer** _ppByteCode
	, oBuffer** _ppErrors)
{
	oStd::xxlstring IncludeSwitches, DefineSwitches;

	if (_IncludePaths && !oStrTokToSwitches(IncludeSwitches, " /I", _IncludePaths, ";"))
		return false; // pass through error

	if (_CommonDefines && !oStrTokToSwitches(DefineSwitches, " /D", _CommonDefines, ";"))
		return false; // pass through error

	if (_SpecificDefines && !oStrTokToSwitches(DefineSwitches, " /D", _SpecificDefines, ";"))
		return false; // pass through error

	D3D_FEATURE_LEVEL TargetFeatureLevel = D3D_FEATURE_LEVEL_10_0;
	if (!oD3D11GetFeatureLevel(_TargetShaderModel, &TargetFeatureLevel))
		return oErrorSetLast(std::errc::protocol_error, "Could not determine feature level from shader model %d.%d", _TargetShaderModel.major, _TargetShaderModel.minor);

	const char* Profile = oD3D11GetShaderProfile(TargetFeatureLevel, _Stage);
	if (!Profile)
	{
		oStd::sstring StrVer;
		return oErrorSetLast(std::errc::not_supported, "%s not supported by shader model %s", oStd::as_string(_Stage), oStd::to_string2(StrVer, _TargetShaderModel));
	}

	if (!_EntryPoint)
		_EntryPoint = "main";

	std::string FXCCommandLine;
	FXCCommandLine.reserve(oKB(2));
	FXCCommandLine.append(IncludeSwitches);
	FXCCommandLine.append(DefineSwitches);
	FXCCommandLine.append(" /O3");
	FXCCommandLine.append(" /E");
	FXCCommandLine.append(_EntryPoint);
	FXCCommandLine.append(" /T");
	FXCCommandLine.append(Profile);

	oBuffer* b = nullptr;
	if (!oFXC(FXCCommandLine.c_str(), _ShaderPath, _ShaderBody, &b))
	{
		*_ppByteCode = nullptr;
		*_ppErrors = b;
		return oErrorSetLast(std::errc::protocol_error, "Shader compilation failed: %s#%s", _ShaderPath, _EntryPoint);
	}

	*_ppByteCode = b;
	*_ppErrors = nullptr;

	return true;
}

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
		return oErrorSetLast(std::errc::no_such_file_or_directory, "%s not found", oSAFESTRN(_URIReference));

	if (oStricmp(URIParts.Scheme, "file"))
		return oErrorSetLast(std::errc::not_supported, "Currently only file schemed URIs are supported.");

	oRef<ID3D11Device> D3DDevice;
	oVERIFY(_pDevice->QueryInterface(&D3DDevice));

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
	oVERIFY(_pDevice->QueryInterface(&D3DDevice));

	oRef<ID3D11Texture2D> D3DTexture;
	if (!oD3D11Load(D3DDevice, _Desc, _DebugName, _pBuffer, _SizeofBuffer, (ID3D11Resource**)&D3DTexture))
		return false;

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
	return oD3D11Save(static_cast<oD3D11Texture*>(_pTexture)->Texture, oD3D11IFFFromFileFormat(_Format), _pBuffer, _SizeofBuffer);
}

bool oGPUTextureSave(oGPUTexture* _pTexture, oGPU_FILE_FORMAT _Format, const char* _Path)
{
	return oD3D11Save(static_cast<oD3D11Texture*>(_pTexture)->Texture, oD3D11IFFFromFileFormat(_Format), _Path);
}
