/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#include "oD3D11LineList.h"
#include "oD3D11Device.h"

oDEFINE_GPUDEVICE_CREATE(oD3D11, LineList);
oBEGIN_DEFINE_GPURESOURCE_CTOR(oD3D11, LineList)
{
	*_pSuccess = false;
	oD3D11DEVICE();
	oGPU_BUFFER_DESC d;
	d.Type = oGPU_BUFFER_VERTEX;
	d.StructByteSize = sizeof(oGPU_LINE);
	d.ArraySize = Desc.MaxNumLines;
	oVERIFY(oD3D11BufferCreate(D3DDevice, _Name, d, nullptr, &Lines));
	Desc.NumLines = 0;
	*_pSuccess = true;
}

int2 oD3D11LineList::GetByteDimensions(int _Subresource) const threadsafe
{
	return int2(sizeof(oGPU_LINE), Desc.MaxNumLines);
}

void oD3D11LineList::SetNumLines(uint _NumLines)
{
	Desc.NumLines = _NumLines;
}
