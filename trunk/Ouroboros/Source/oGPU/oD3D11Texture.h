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
#pragma once
#ifndef oGPU_Texture_h
#define oGPU_Texture_h

#include "oGPUCommon.h"

oGPU_NAMESPACE_BEGIN

oRESOURCE_CLASS(texture)
{
	//oRESOURCE_DECLARATION(oD3D11, Texture);
public:
	oRESOURCE_INTERFACE__
	d3d11_texture(std::shared_ptr<device>& _Device, const char* _Name, const texture_info& _Info, ID3D11Resource* _pTexture = nullptr);
	~d3d11_texture();

	union //intrusive_ptr<ID3D11Texture2D> Texture;
	{
		ID3D11Resource* pResource;
		ID3D11Texture1D* pTexture1D;
		ID3D11Texture2D* pTexture2D;
		ID3D11Texture3D* pTexture3D;
	};

	intrusive_ptr<ID3D11ShaderResourceView> SRV;
	intrusive_ptr<ID3D11UnorderedAccessView> UAV;

	// Texture2 is used to emulate YUV and other planar sized formats.
	std::shared_ptr<texture> Texture2;
};

oGPU_NAMESPACE_END

#endif
