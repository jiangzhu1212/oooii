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
#ifndef oD3D11ComputeShader_h
#define oD3D11ComputeShader_h

#include "oGPUCommon.h"
#include <d3d11.h>

// {17749C8B-0641-4A8B-A4A5-8456C6B7D586}
oDECLARE_GPUDEVICECHILD_IMPLEMENTATION(oD3D11, ComputeShader, 0x17749c8b, 0x641, 0x4a8b, 0xa4, 0xa5, 0x84, 0x56, 0xc6, 0xb7, 0xd5, 0x86)
{
	oDEFINE_GPUDEVICECHILD_INTERFACE();
	oDECLARE_GPUDEVICECHILD_CTOR(oD3D11, ComputeShader);
	void GetDesc(DESC* _pDesc) const threadsafe override;
	ouro::intrusive_ptr<ID3D11ComputeShader> ComputeShader;
	ouro::sstring DebugName;
};

#endif
