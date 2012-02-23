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
#include "oD3D11InstanceList.h"
#include "oD3D11Device.h"

oDEFINE_GFXDEVICE_CREATE(oD3D11, InstanceList);
oBEGIN_DEFINE_GFXRESOURCE_CTOR(oD3D11, InstanceList)
{
	*_pSuccess = false;
	oD3D11DEVICE();
	uint InstanceStride = oGfxCalcInterleavedVertexSize(_Desc.pElements, _Desc.NumElements, _Desc.InputSlot);
	oVERIFY(oD3D11CreateVertexBuffer(D3DDevice, _Name, true, nullptr, _Desc.MaxNumInstances, InstanceStride, &Instances));
	Desc.NumElements = 0;
	*_pSuccess = true;
}

bool oD3D11InstanceList::Map(ID3D11DeviceContext* _pContext, oGfxCommandList::MAPPED* _pMapped)
{
	D3D11_MAPPED_SUBRESOURCE mapped;
	if (FAILED(_pContext->Map(Instances, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
		return oWinSetLastError();
	_pMapped->pData = mapped.pData;
	_pMapped->RowPitch = mapped.RowPitch;
	_pMapped->SlicePitch = mapped.DepthPitch;
	return true;
}

void oD3D11InstanceList::Unmap(ID3D11DeviceContext* _pContext, uint _NewNumInstances)
{
	_pContext->Unmap(Instances, 0);
	Desc.NumInstances = _NewNumInstances;
}
