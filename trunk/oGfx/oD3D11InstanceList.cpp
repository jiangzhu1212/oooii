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

	//if (_Desc.NumIndices)
	//	oVERIFY(oD3D11CreateIndexBuffer(D3DDevice, _Name, true, 0, _Desc.NumIndices, _Desc.NumIndices < 65535, &Indices));

	//if (_Desc.NumVertices)
	//{
	//	char name[oKB(1)];
	//	for (size_t i = 0; i < oCOUNTOF(Vertices); i++)
	//	{
	//		if (_Desc.VertexByteSize[i])
	//		{
	//			sprintf_s(name, "%sVertices[%02u]", _Name, i);
	//			oVERIFY(oD3D11CreateVertexBuffer(D3DDevice, name, true, 0, _Desc.NumVertices, _Desc.VertexByteSize[i], &Vertices[i]));
	//		}
	//	}
	//}

	*_pSuccess = true;
}
