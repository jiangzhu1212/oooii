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
#pragma once
#ifndef oD3D11Pipline_h
#define oD3D11Pipline_h

#include "oGPUCommon.h"
#include <oPlatform/Windows/oD3D11.h>

oDECLARE_GPUDEVICECHILD_IMPLEMENTATION(oD3D11, Pipeline)
{
	oDEFINE_GPUDEVICECHILD_INTERFACE();
	oDECLARE_GPUDEVICECHILD_CTOR(oD3D11, Pipeline);
	~oD3D11Pipeline();

	void GetDesc(DESC* _pDesc) const threadsafe override;

	oRef<ID3D11InputLayout> InputLayout;
	oRef<ID3D11VertexShader> VertexShader;
	oRef<ID3D11HullShader> HullShader;
	oRef<ID3D11DomainShader> DomainShader;
	oRef<ID3D11GeometryShader> GeometryShader;
	oRef<ID3D11PixelShader> PixelShader;

	oGPU_VERTEX_ELEMENT* pElements;
	uint NumElements;

	oStringS DebugName;
};

#endif
