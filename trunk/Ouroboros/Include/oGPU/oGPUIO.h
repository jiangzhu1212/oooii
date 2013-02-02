/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
// This header abstracts API-dependent operators such as compiling, loading,
// saving and other services that are provided out-of-box.
#pragma once
#ifndef oGPUIO_h
#define oGPUIO_h

#include <oGPU/oGPU.h>

// Compiles a shader to its driver-specific bytecode. To make parsing easy, 
// there are two lists of defines that can be specified. These are concatenated
// internally.
bool oGPUCompileShader(
	const char* _IncludePaths // semi-colon delimited list of paths to look in. Use %DEV% for oSYSPATH_DEV
	, const char* _CommonDefines // semi-colon delimited list of symbols (= value) 
	, const char* _SpecificDefines // semi-colon delimited list of symbols (= value)
	, const oVersion& _TargetShaderModel // shader model version to compile against
	, oGPU_PIPELINE_STAGE _Stage // type of shader to compile
	, const char* _ShaderPath // full path to shader - mostly for error reporting
	, const char* _EntryPoint // name of the top-level shader function to use
	, const char* _ShaderBody // a string of the loaded shader file (NOTE: This still goes out to includes, so will still touch the file system
	, oBuffer** _ppByteCode // if successful, this is a buffer filled with byte code
	, oBuffer** _ppErrors); // if failure, this is filled with a string of compile errors

// @oooii-tony: I'm not sure if these APIs belong here at all, in oGPU, or 
// oGPUUtil. I do know they don't belong where they were before, which was 
// nowhere.
// Basically rendering at oooii grew up on D3D and D3DX, which have very robust
// libraries for image format conversion and handling. We didn't spend time 
// reproducing this on our own, so we're relying on this stuff as placeholder.
// I suspect if we add support for a different library, such robust support for
// image formats won't be there and we'll have to implement a lot of this stuff,
// so defer that dev time until then and then bring it back to something that
// can be promoted out of oGPU. For now, at least hide the D3D part...

// Convert the format of a surface into another format in another surface. This
// uses GPU acceleration for BC6H/7 and is currently a pass-through to D3DX11's
// conversion functions at the moment. Check debug logs if this function seems
// to hang because if for whatever reason the CPU/SW version of the BC6H/7
// codec is used, it can take a VERY long time.
bool oGPUSurfaceConvert(
	void* oRESTRICT _pDestination
	, size_t _DestinationRowPitch
	, oSURFACE_FORMAT _DestinationFormat
	, const void* oRESTRICT _pSource
	, size_t _SourceRowPitch
	, oSURFACE_FORMAT _SourceFormat
	, const int2& _MipDimensions);

// Extract the parameters for the above call directly from textures
oAPI bool oGPUSurfaceConvert(oGPUTexture* _pSourceTexture, oSURFACE_FORMAT _NewFormat, oGPUTexture** _ppDestinationTexture);

// Loads a texture from disk. The _Desc specifies certain conversions/resizes
// that can occur on load. Use oDEFAULT or oSURFACE_UNKNOWN to use values as 
// they are found in the specified image resource/buffer.
// @oooii-tony: At this time the implementation does NOT use oImage loading 
// code plus a simple call to call oGPUCreateTexture(). Because this API 
// supports conversion for any oSURFACE_FORMAT, at this time we defer to 
// DirectX's .dds support for advanced formats like BC6 and BC7 as well as their
// internal conversion library. When it's time to go cross-platform, we'll 
// revisit this and hopefully call more generic code.
oAPI bool oGPUTextureLoad(oGPUDevice* _pDevice, const oGPU_TEXTURE_DESC& _Desc, const char* _URIReference, const char* _DebugName, oGPUTexture** _ppTexture);
oAPI bool oGPUTextureLoad(oGPUDevice* _pDevice, const oGPU_TEXTURE_DESC& _Desc, const char* _DebugName, const void* _pBuffer, size_t _SizeofBuffer, oGPUTexture** _ppTexture);

enum oGPUEX_FILE_FORMAT
{
	oGPUEX_FILE_FORMAT_DDS,
	oGPUEX_FILE_FORMAT_JPG,
	oGPUEX_FILE_FORMAT_PNG,
};

// Saves a texture to disk. The format will be determined from the contents of 
// the texture. If the specified format does not support the contents of the 
// texture the function will return false - check oErrorGetLast() for extended
// information.
oAPI bool oGPUTextureSave(oGPUTexture* _pTexture, oGPUEX_FILE_FORMAT _Format, void* _pBuffer, size_t _SizeofBuffer);
oAPI bool oGPUTextureSave(oGPUTexture* _pTexture, oGPUEX_FILE_FORMAT _Format, const char* _Path);

#endif
