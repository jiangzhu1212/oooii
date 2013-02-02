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

#include "oD3D11Mesh.h"
#include "oD3D11Device.h"
#include "oD3D11Buffer.h"
#include <oBasis/oLimits.h>
oDEFINE_GPUDEVICE_CREATE(oD3D11, Mesh);
oBEGIN_DEFINE_GPURESOURCE_CTOR(oD3D11, Mesh)
	, NumVertexBuffers(0)
{
	*_pSuccess = false;
	Ranges.resize(_Desc.NumRanges);

	oD3D11DEVICE();

	if (_Desc.NumIndices)
	{
		oGPU_BUFFER_DESC d;
		d.Type = oGPU_BUFFER_INDEX;
		d.Format = oGPUHas16BitIndices(_Desc.NumVertices) ? oSURFACE_R16_UINT : oSURFACE_R32_UINT;
		d.ArraySize = _Desc.NumIndices;
		oVERIFY(_pDevice->CreateBuffer(_Name, d, &Indices));
	}

	if (_Desc.NumVertices)
	{
		oStringL name;
		oGPU_BUFFER_DESC d;
		d.Type = oGPU_BUFFER_VERTEX;
		d.ArraySize = _Desc.NumVertices;
		for (uint i = 0; i < oCOUNTOF(Vertices); i++)
		{
			d.StructByteSize = oGPUCalcVertexSize(_Desc.VertexElements, _Desc.NumVertexElements, i);
			if (d.StructByteSize)
			{
				oPrintf(name, "%sVertices[%02u]", _Name, i);
				oVERIFY(_pDevice->CreateBuffer(_Name, d, &Vertices[i]));
				NumVertexBuffers = i + 1; // always include the latest non-null value
			}
		}
	}

	*_pSuccess = true;
}

int2 oD3D11Mesh::GetByteDimensions(int _Subresource) const threadsafe
{
	switch (_Subresource)
	{
		case oGPU_MESH_VERTICES0: case oGPU_MESH_VERTICES1: case oGPU_MESH_VERTICES2: 
		{
			oGPU_BUFFER_DESC d;
			Vertices[_Subresource-oGPU_MESH_VERTICES0]->GetDesc(&d);
			return int2(d.StructByteSize, d.ArraySize);
		}
		
		case oGPU_MESH_INDICES:
		{
			oGPU_BUFFER_DESC d;
			Indices->GetDesc(&d);
			return int2(d.StructByteSize, d.ArraySize);
		}

		case oGPU_MESH_RANGES: 
			return int2(sizeof(oGPU_RANGE), Desc.NumRanges);

		oNODEFAULT;
	}
}

ID3D11Resource* oD3D11Mesh::GetSubresource(int _Subresource) threadsafe
{
	switch (_Subresource)
	{
		case oGPU_MESH_VERTICES0: case oGPU_MESH_VERTICES1: case oGPU_MESH_VERTICES2: return const_cast<ID3D11Buffer*>(static_cast<const oD3D11Buffer*>(Vertices[_Subresource-oGPU_MESH_VERTICES0].c_ptr())->Buffer.c_ptr());
		case oGPU_MESH_INDICES: return const_cast<ID3D11Buffer*>(static_cast<const oD3D11Buffer*>(Indices.c_ptr())->Buffer.c_ptr());
		case oGPU_MESH_RANGES: 
		oNODEFAULT;
	}
}
