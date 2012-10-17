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
#include <oGPU/oGPUCompiler.h>
#include <oPlatform/Windows/oD3D11.h>

// @oooii-tony: Here's a flavor that works on a pre-loaded source since we want 
// to support more than one shader entry. Right now to keep it simple, the 
// include system still hits the file system for each header. How can we get 
// around that? Until we can, loading source from a buffer doesn't add too much 
// benefit.
bool oGPUCompileShader(
	const char* _IncludePaths
	, const char* _CommonDefines
	, const char* _SpecificDefines
	, const oVersion& _TargetShaderModel
	, oGPU_PIPELINE_STAGE _Stage
	, const char* _ShaderPath
	, const char* _EntryPoint
	, const char* _ShaderBody
	, oBuffer** _ppByteCode
	, oBuffer** _ppErrors)
{
	oStringXXL IncludeSwitches, DefineSwitches;

	if (_IncludePaths && !oStrTokToSwitches(IncludeSwitches, " /I", _IncludePaths, ";"))
		return false; // pass through error

	if (_CommonDefines && !oStrTokToSwitches(DefineSwitches, " /D", _CommonDefines, ";"))
		return false; // pass through error

	if (_SpecificDefines && !oStrTokToSwitches(DefineSwitches, " /D", _SpecificDefines, ";"))
		return false; // pass through error

	D3D_FEATURE_LEVEL TargetFeatureLevel = D3D_FEATURE_LEVEL_10_0;
	if (!oD3D11GetFeatureLevel(_TargetShaderModel, &TargetFeatureLevel))
		return oErrorSetLast(oERROR_GENERIC, "Could not determine feature level from shader model %d.%d", _TargetShaderModel.Major, _TargetShaderModel.Minor);

	const char* Profile = oD3D11GetShaderProfile(TargetFeatureLevel, _Stage);
	if (!Profile)
		return oErrorSetLast(oERROR_GENERIC, "%s not supported by shader model %d.%d", oAsString(_Stage), _TargetShaderModel.Major, _TargetShaderModel.Minor);

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
		return oErrorSetLast(oERROR_CORRUPT, "Shader compilation failed: %s#%s", _ShaderPath, _EntryPoint);
	}

	*_ppByteCode = b;
	*_ppErrors = nullptr;

	return true;
}
