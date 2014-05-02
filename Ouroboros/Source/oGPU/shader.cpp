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

#include <oGPU/shader.h>
#include "d3d11_util.h"
#include "d3d_compile.h"
#include "d3d_debug.h"
#include <string>

using namespace ouro::gpu::d3d;

namespace ouro {
	namespace gpu {

Device* get_device(device* _pDevice);
DeviceContext* get_device_context(command_list* _pCommandList);

void bind_shader(command_list* _pCommandList, vertex_shader _Shader) { get_device_context(_pCommandList)->VSSetShader((VertexShader*)_Shader, nullptr, 0); }
void bind_shader(command_list* _pCommandList, hull_shader _Shader) { get_device_context(_pCommandList)->HSSetShader((HullShader*)_Shader, nullptr, 0); }
void bind_shader(command_list* _pCommandList, domain_shader _Shader) { get_device_context(_pCommandList)->DSSetShader((DomainShader*)_Shader, nullptr, 0); }
void bind_shader(command_list* _pCommandList, geometry_shader _Shader) { get_device_context(_pCommandList)->GSSetShader((GeometryShader*)_Shader, nullptr, 0); }
void bind_shader(command_list* _pCommandList, pixel_shader _Shader) { get_device_context(_pCommandList)->PSSetShader((PixelShader*)_Shader, nullptr, 0); }
void bind_shader(command_list* _pCommandList, compute_shader _Shader) { get_device_context(_pCommandList)->CSSetShader((ComputeShader*)_Shader, nullptr, 0); }
	
shader make_shader(device* _pDevice, const shader_type::value& _Type, const void* _pByteCode, const char* _DebugName)
{
	DeviceChild* pShader = nullptr;
	if (_pByteCode)
	{
		Device* pDevice = get_device(_pDevice);
		switch (_Type)
		{
			case shader_type::vertex: oV(pDevice->CreateVertexShader(_pByteCode, byte_code_size(_pByteCode), nullptr, (VertexShader**)&pShader)); break;
			case shader_type::hull: oV(pDevice->CreateHullShader(_pByteCode, byte_code_size(_pByteCode), nullptr, (HullShader**)&pShader)); break;
			case shader_type::domain: oV(pDevice->CreateDomainShader(_pByteCode, byte_code_size(_pByteCode), nullptr, (DomainShader**)&pShader)); break;
			case shader_type::geometry: oV(pDevice->CreateGeometryShader(_pByteCode, byte_code_size(_pByteCode), nullptr, (GeometryShader**)&pShader)); break;
			case shader_type::pixel: oV(pDevice->CreatePixelShader(_pByteCode, byte_code_size(_pByteCode), nullptr, (PixelShader**)&pShader)); break;
			case shader_type::compute: oV(pDevice->CreateComputeShader(_pByteCode, byte_code_size(_pByteCode), nullptr, (ComputeShader**)&pShader)); break;
			default: oTHROW_INVARG0();
		};

		if (_DebugName)
			debug_name(pShader, _DebugName);
	}
	return (shader)pShader;
}

void unmake_shader(shader _Shader)
{
	if (_Shader)
		((DeviceChild*)_Shader)->Release();
}

scoped_allocation compile_shader(const char* _IncludePaths
	, const char* _Defines
	, const char* _ShaderSourcePath
	, const pipeline_stage::value& _Stage
	, const char* _EntryPoint
	, const char* _ShaderSource
	, const allocator& _Allocator)
{
	std::string cmdline;
	cmdline.reserve(2048);
	cmdline = "/O3 /T ";
	cmdline += shader_profile(D3D_FEATURE_LEVEL_11_0, _Stage);
	cmdline += " /E ";
	cmdline += _EntryPoint;

	// Add defines
	cmdline += " /D oHLSL ";
	if (_Defines)
	{
		char* ctx = nullptr;
		std::string defs(_Defines);
		char* def = strtok_r((char*)defs.c_str(), ";", &ctx);
		while (def)
		{
			cmdline += " /D ";
			cmdline += def;
			def = strtok_r(nullptr, ";", &ctx);
		}
	}
	
	// Add includes
	if (_IncludePaths)
	{
		char* ctx = nullptr;
		std::string incs(_IncludePaths);
		char* inc = strtok_r((char*)incs.c_str(), ";", &ctx);
		while (inc)
		{
			cmdline += " /I ";
			cmdline += inc;
			inc = strtok_r(nullptr, ";", &ctx);
		}
	}

	return d3d::compile_shader(cmdline.c_str(), _ShaderSourcePath, _ShaderSource, _Allocator);
}

	} // namespace gpu
} // namespace ouro
