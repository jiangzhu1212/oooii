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
#include "oD3D11ComputeShader.h"
#include "oD3D11Device.h"

const oGUID& oGetGUID(threadsafe const oD3D11ComputeShader* threadsafe const *)
{
	// {17749C8B-0641-4A8B-A4A5-8456C6B7D586}
	static const oGUID oIID_D3D11ComputeShader = { 0x17749c8b, 0x641, 0x4a8b, { 0xa4, 0xa5, 0x84, 0x56, 0xc6, 0xb7, 0xd5, 0x86 } };
	return oIID_D3D11ComputeShader;
}

oDEFINE_GPUDEVICE_CREATE(oD3D11, ComputeShader);
oBEGIN_DEFINE_GPUDEVICECHILD_CTOR(oD3D11, ComputeShader)
	, DebugName(_Desc.DebugName)
{
	*_pSuccess = false;
	if (!_Desc.pComputeShader)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER, "A buffer of valid compute shader bytecode must be specified");
		return;
	}

	oD3D11DEVICE();
	oV(D3DDevice->CreateComputeShader(_Desc.pComputeShader, oHLSLGetByteCodeSize(_Desc.pComputeShader), nullptr, &ComputeShader));
	oVERIFY(oD3D11SetDebugName(ComputeShader, _Desc.DebugName));
	*_pSuccess = true;
}

void oD3D11ComputeShader::GetDesc(DESC* _pDesc) const threadsafe
{
	_pDesc->DebugName = DebugName;
	_pDesc->pComputeShader = nullptr;
}
