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
#ifndef oD3D11Pipline_h
#define oD3D11Pipline_h

#include "oGPUCommon.h"
#include <oPlatform/Windows/oD3D11.h>

// {772E2A04-4C2D-447A-8DA8-91F258EFA68C}
oDECLARE_GPUDEVICECHILD_IMPLEMENTATION(oD3D11, Pipeline, 0x772e2a04, 0x4c2d, 0x447a, 0x8d, 0xa8, 0x91, 0xf2, 0x58, 0xef, 0xa6, 0x8c)
{
	oDEFINE_GPUDEVICECHILD_INTERFACE();
	oDECLARE_GPUDEVICECHILD_CTOR(oD3D11, Pipeline);
	~oD3D11Pipeline();

	void GetDesc(DESC* _pDesc) const threadsafe override;

	oStd::intrusive_ptr<ID3D11InputLayout> InputLayout;
	oStd::intrusive_ptr<ID3D11VertexShader> VertexShader;
	oStd::intrusive_ptr<ID3D11HullShader> HullShader;
	oStd::intrusive_ptr<ID3D11DomainShader> DomainShader;
	oStd::intrusive_ptr<ID3D11GeometryShader> GeometryShader;
	oStd::intrusive_ptr<ID3D11PixelShader> PixelShader;

	oGPU_VERTEX_ELEMENT* pElements;
	uint NumElements;
	D3D_PRIMITIVE_TOPOLOGY InputTopology;

	oStd::sstring DebugName;
};

#endif
