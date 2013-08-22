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
#ifndef oD3D11Buffer_h
#define oD3D11Buffer_h

#include <oGPU/oGPU.h>
#include "oGPUCommon.h"
#include <oPlatform/Windows/oD3D11.h>

// {6CB53116-9751-4EB0-864C-A87D3E66AEB0}
oDECLARE_GPURESOURCE_IMPLEMENTATION(oD3D11, Buffer, oGPU_BUFFER, 0x6cb53116, 0x9751, 0x4eb0, 0x86, 0x4c, 0xa8, 0x7d, 0x3e, 0x66, 0xae, 0xb0)
{
	oDEFINE_GPURESOURCE_INTERFACE();
	oDECLARE_GPURESOURCE_CTOR(oD3D11, Buffer);
	oRef<ID3D11Buffer> Buffer;
	oRef<ID3D11UnorderedAccessView> UAV;
	oRef<ID3D11UnorderedAccessView> UAVAppend;
	oRef<ID3D11ShaderResourceView> SRV;
};

#endif
