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
#include "d3d11_as_string.h"

namespace ouro {

const char* as_string(const D3D11_BIND_FLAG& _Flag)
{
	switch (_Flag)
	{
		case D3D11_BIND_VERTEX_BUFFER: return "D3D11_BIND_VERTEX_BUFFER";
		case D3D11_BIND_INDEX_BUFFER: return "D3D11_BIND_INDEX_BUFFER";
		case D3D11_BIND_CONSTANT_BUFFER: return "D3D11_BIND_CONSTANT_BUFFER";
		case D3D11_BIND_SHADER_RESOURCE: return "D3D11_BIND_SHADER_RESOURCE";
		case D3D11_BIND_STREAM_OUTPUT: return "D3D11_BIND_STREAM_OUTPUT";
		case D3D11_BIND_RENDER_TARGET: return "D3D11_BIND_RENDER_TARGET";
		case D3D11_BIND_DEPTH_STENCIL: return "D3D11_BIND_DEPTH_STENCIL";
		case D3D11_BIND_UNORDERED_ACCESS: return "D3D11_BIND_UNORDERED_ACCESS";
		//case D3D11_BIND_DECODER: return "D3D11_BIND_DECODER";
		//case D3D11_BIND_VIDEO_ENCODER: return "D3D11_BIND_VIDEO_ENCODER";
		default: break;
	}
	return "?";
}

const char* as_string(const D3D11_CPU_ACCESS_FLAG& _Flag)
{
	switch (_Flag)
	{
		case D3D11_CPU_ACCESS_WRITE: return "D3D11_CPU_ACCESS_WRITE";
		case D3D11_CPU_ACCESS_READ: return "D3D11_CPU_ACCESS_READ";
		default: break;
	}
	return "?";
}

const char* as_string(const D3D11_RESOURCE_MISC_FLAG& _Flag)
{
	switch (_Flag)
	{
		case D3D11_RESOURCE_MISC_GENERATE_MIPS: return "D3D11_RESOURCE_MISC_GENERATE_MIPS";
		case D3D11_RESOURCE_MISC_SHARED: return "D3D11_RESOURCE_MISC_SHARED";
		case D3D11_RESOURCE_MISC_TEXTURECUBE: return "D3D11_RESOURCE_MISC_TEXTURECUBE";
		case D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS: return "D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS";
		case D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS: return "D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS";
		case D3D11_RESOURCE_MISC_BUFFER_STRUCTURED: return "D3D11_RESOURCE_MISC_BUFFER_STRUCTURED";
		case D3D11_RESOURCE_MISC_RESOURCE_CLAMP: return "D3D11_RESOURCE_MISC_RESOURCE_CLAMP";
		case D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX: return "D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX";
		case D3D11_RESOURCE_MISC_GDI_COMPATIBLE: return "D3D11_RESOURCE_MISC_GDI_COMPATIBLE";
		//case D3D11_RESOURCE_MISC_SHARED_NTHANDLE: return "D3D11_RESOURCE_MISC_SHARED_NTHANDLE";
		//case D3D11_RESOURCE_MISC_RESTRICTED_CONTENT: return "D3D11_RESOURCE_MISC_RESTRICTED_CONTENT";
		//case D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE: return "D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE";
		//case D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE_DRIVER: return "D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE_DRIVER";
		default: break;
	}
	return "?";
}

const char* as_string(const D3D11_RESOURCE_DIMENSION& _Type)
{
	switch (_Type)
	{
		case D3D11_RESOURCE_DIMENSION_UNKNOWN: return "D3D11_RESOURCE_DIMENSION_UNKNOWN	=";
		case D3D11_RESOURCE_DIMENSION_BUFFER: return "D3D11_RESOURCE_DIMENSION_BUFFER	=";
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D: return "D3D11_RESOURCE_DIMENSION_TEXTURE1D	=";
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D: return "D3D11_RESOURCE_DIMENSION_TEXTURE2D	=";
		case D3D11_RESOURCE_DIMENSION_TEXTURE3D: return "D3D11_RESOURCE_DIMENSION_TEXTURE3D	=";
		default: break;
	}
	return "?";
}

const char* as_string(const D3D11_UAV_DIMENSION& _Type)
{
	switch (_Type)
	{
		case D3D11_UAV_DIMENSION_UNKNOWN: return "D3D11_UAV_DIMENSION_UNKNOWN";
		case D3D11_UAV_DIMENSION_BUFFER: return "D3D11_UAV_DIMENSION_BUFFER";
		case D3D11_UAV_DIMENSION_TEXTURE1D: return "D3D11_UAV_DIMENSION_TEXTURE1D";
		case D3D11_UAV_DIMENSION_TEXTURE1DARRAY: return "D3D11_UAV_DIMENSION_TEXTURE1DARRAY";
		case D3D11_UAV_DIMENSION_TEXTURE2D: return "D3D11_UAV_DIMENSION_TEXTURE2D";
		case D3D11_UAV_DIMENSION_TEXTURE2DARRAY: return "D3D11_UAV_DIMENSION_TEXTURE2DARRAY";
		case D3D11_UAV_DIMENSION_TEXTURE3D: return "D3D11_UAV_DIMENSION_TEXTURE3D";
		default: break;
	}
	return "?";
}

const char* as_string(const D3D11_USAGE& _Usage)
{
	switch (_Usage)
	{
		case D3D11_USAGE_DEFAULT: return "D3D11_USAGE_DEFAULT";
		case D3D11_USAGE_IMMUTABLE: return "D3D11_USAGE_IMMUTABLE";
		case D3D11_USAGE_DYNAMIC: return "D3D11_USAGE_DYNAMIC";
		case D3D11_USAGE_STAGING: return "D3D11_USAGE_STAGING";
		default: break;
	}
	return "?";
}

} // namespace ouro
