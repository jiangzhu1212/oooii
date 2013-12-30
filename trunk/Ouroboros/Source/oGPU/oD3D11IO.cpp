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
#include <oCore/filesystem.h>
#include "oGPUCommon.h"
#include <d3d11.h>
#include "oD3D11Texture.h"
#include "oD3D11Device.h"

using namespace ouro;
using namespace ouro::d3d11;

#if 0
static char* oStrTokToSwitches(char* _StrDestination
	, size_t _SizeofStrDestination
	, const char* _Switch
	, const char* _Tokens
	, const char* _Separator)
{
	size_t len = strlen(_StrDestination);
	_StrDestination += len;
	_SizeofStrDestination -= len;

	char* ctx = nullptr;
	const char* tok = oStrTok(_Tokens, _Separator, &ctx);
	while (tok)
	{
		strlcpy(_StrDestination, _Switch, _SizeofStrDestination);
		size_t len = strlen(_StrDestination);
		_StrDestination += len;
		_SizeofStrDestination -= len;

		clean_path(_StrDestination, _SizeofStrDestination, tok);

		len = strlen(_StrDestination);
		_StrDestination += len;
		_SizeofStrDestination -= len;

		tok = oStrTok(nullptr, ";", &ctx);
	}

	return _StrDestination;
}
template<size_t size> char* oStrTokToSwitches(char (&_StrDestination)[size], const char* _Switch, const char* _Tokens, const char* _Separator) { return oStrTokToSwitches(_StrDestination, size, _Switch, _Tokens, _Separator); }
template<size_t capacity> char* oStrTokToSwitches(ouro::fixed_string<char, capacity>& _StrDestination, const char* _Switch, const char* _Tokens, const char* _Separator) { return oStrTokToSwitches(_StrDestination, _StrDestination.capacity(), _Switch, _Tokens, _Separator); }
#endif
// @tony: Here's a flavor that works on a pre-loaded source since we want 
// to support more than one shader entry. Right now to keep it simple, the 
// include system still hits the file system for each header. How can we get 
// around that? Until we can, loading source from a buffer doesn't add too much 
// benefit.
bool oGPUCompileShader(
	const char* _IncludePaths
	, const char* _CommonDefines
	, const char* _SpecificDefines
	, const version& _TargetShaderModel
	, ouro::gpu::pipeline_stage::value _Stage
	, const char* _ShaderPath
	, const char* _EntryPoint
	, const char* _ShaderBody
	, oBuffer** _ppByteCode
	, oBuffer** _ppErrors)
{
#if 1
	return oErrorSetLast(std::errc::not_supported);
#else

	xxlstring IncludeSwitches, DefineSwitches;

	if (_IncludePaths && !oStrTokToSwitches(IncludeSwitches, " /I", _IncludePaths, ";"))
		return false; // pass through error

	if (_CommonDefines && !oStrTokToSwitches(DefineSwitches, " /D", _CommonDefines, ";"))
		return false; // pass through error

	if (_SpecificDefines && !oStrTokToSwitches(DefineSwitches, " /D", _SpecificDefines, ";"))
		return false; // pass through error

	D3D_FEATURE_LEVEL TargetFeatureLevel = feature_level(_TargetShaderModel);

	const char* Profile = shader_profile(TargetFeatureLevel, _Stage);
	if (!Profile)
	{
		sstring StrVer;
		return oErrorSetLast(std::errc::not_supported, "%s not supported by shader model %s", ouro::as_string(_Stage), to_string2(StrVer, _TargetShaderModel));
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
#endif
}

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

bool oGPUSurfaceConvert(oGPUTexture* _pSourceTexture, ouro::surface::format _NewFormat, oGPUTexture** _ppDestinationTexture)
{
	intrusive_ptr<ID3D11Texture2D> D3DDestinationTexture;
	convert(static_cast<oD3D11Texture*>(_pSourceTexture)->Texture, _NewFormat, &D3DDestinationTexture);

	intrusive_ptr<oGPUDevice> Device;
	_pSourceTexture->GetDevice(&Device);

	oGPUTexture::DESC d = get_texture_info(D3DDestinationTexture);

	bool success = false;
	oCONSTRUCT(_ppDestinationTexture, oD3D11Texture(Device, d, _pSourceTexture->GetName(), &success, D3DDestinationTexture));
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
