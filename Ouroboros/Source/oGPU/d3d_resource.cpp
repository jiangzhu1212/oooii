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
#include "d3d_resource.h"
#include <oCore/windows/win_util.h>
#include "dxgi_util.h"

namespace ouro {
	namespace gpu {
		namespace d3d {

// {13BA565C-4766-49C4-8C1C-C1F459F00A65}
static const GUID oWKPDID_BUFFER_INFO = { 0x13ba565c, 0x4766, 0x49c4, { 0x8c, 0x1c, 0xc1, 0xf4, 0x59, 0xf0, 0xa, 0x65 } };

void set_info(Resource* _pBuffer, const buffer_info& _Desc)
{
	oV(_pBuffer->SetPrivateData(oWKPDID_BUFFER_INFO, sizeof(_Desc), &_Desc));
}

buffer_info get_info(const Resource* _pBuffer)
{
	unsigned int size = sizeof(buffer_info);
	buffer_info i;
	oV(const_cast<ID3D11Resource*>(_pBuffer)->GetPrivateData(oWKPDID_BUFFER_INFO, &size, &i));
	return i;
}

template<typename DescT> static void fill_non_dimensions(const DescT& _Desc, bool _AsArray, texture_type::value _BasicType, texture_info* _pInfo)
{
	if (_Desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE)
		_BasicType = texture_type::default_cube;

	_pInfo->format = dxgi::to_surface_format(_Desc.Format);

	_pInfo->type = _BasicType;
	if (_Desc.MipLevels > 1)
		_pInfo->type = add_mipped(_pInfo->type);

	if (_AsArray)
		_pInfo->type = add_array(_pInfo->type);
	else
		_pInfo->array_size = 0;

	if (_Desc.Usage == D3D11_USAGE_STAGING)
		_pInfo->type = make_readback(_pInfo->type);

	if (_Desc.BindFlags & (D3D11_BIND_RENDER_TARGET|D3D11_BIND_DEPTH_STENCIL))
		_pInfo->type = make_render_target(_pInfo->type);

	if (_Desc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
	{
		oASSERT(_pInfo->type == texture_type::default_2d, "Invalid/unhandled type");
		_pInfo->type = texture_type::unordered_2d;
	}
}

texture_info get_texture_info(Resource* _pResource, bool _AsArray, D3D11_USAGE* _pUsage)
{
	texture_info info;

	D3D11_RESOURCE_DIMENSION type = D3D11_RESOURCE_DIMENSION_UNKNOWN;
	_pResource->GetType(&type);
	switch (type)
	{
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		{
			D3D11_TEXTURE1D_DESC desc;
			static_cast<Texture1D*>(_pResource)->GetDesc(&desc);
			info.dimensions = ushort3(static_cast<unsigned short>(desc.Width), 1, 1);
			info.array_size = static_cast<unsigned short>(desc.ArraySize);
			fill_non_dimensions(desc, _AsArray || desc.ArraySize > 1, texture_type::default_1d, &info);
			if (_pUsage) *_pUsage = desc.Usage;
			break;
		}

		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		{
			D3D11_TEXTURE2D_DESC desc;
			static_cast<Texture2D*>(_pResource)->GetDesc(&desc);
			info.dimensions = ushort3(static_cast<unsigned short>(desc.Width)
				, static_cast<unsigned short>(desc.Height), 1);
			info.array_size = static_cast<unsigned short>(desc.ArraySize);
			fill_non_dimensions(desc, _AsArray || desc.ArraySize > 1, texture_type::default_2d, &info);
			if (_pUsage) *_pUsage = desc.Usage;
			break;
		}

		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
		{
			D3D11_TEXTURE3D_DESC desc;
			static_cast<Texture3D*>(_pResource)->GetDesc(&desc);
			info.dimensions = ushort3(static_cast<unsigned short>(desc.Width)
				, static_cast<unsigned short>(desc.Height)
				, static_cast<unsigned short>(desc.Depth));
			info.array_size = 1;
			fill_non_dimensions(desc, false, texture_type::default_3d, &info);
			if (_pUsage) *_pUsage = desc.Usage;
			break;
		}

		case D3D11_RESOURCE_DIMENSION_BUFFER:
		{
			buffer_info i = get_info(_pResource);
			D3D11_BUFFER_DESC desc;
			static_cast<Buffer*>(_pResource)->GetDesc(&desc);
			info.dimensions = ushort3(i.struct_byte_size, static_cast<ushort>(i.array_size), 1);
			info.array_size = i.array_size;
			info.format = i.format;
			if (_pUsage) *_pUsage = desc.Usage;
			break;
		};

		oNODEFAULT;
	}

	return info;
}


		} // namespace d3d
	} // namespace gpu
} // namespace ouro
