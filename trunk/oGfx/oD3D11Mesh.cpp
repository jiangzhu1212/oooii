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
#include "oD3D11Mesh.h"
#include "oD3D11Device.h"

oDEFINE_GFXDEVICE_CREATE(oD3D11, Mesh);
oBEGIN_DEFINE_GFXRESOURCE_CTOR(oD3D11, Mesh)
	, p32BitIndices(nullptr)
	, Uses16BitIndices(_Desc.NumIndices < 65535)
{
	*_pSuccess = false;
	MappedIndices.pData = nullptr;

	oINIT_ARRAY(VertexStrides, 0);
	Ranges.resize(_Desc.NumRanges);

	oD3D11DEVICE();

	if (_Desc.NumIndices)
		oVERIFY(oD3D11CreateIndexBuffer(D3DDevice, _Name, true, 0, _Desc.NumIndices, Uses16BitIndices, &Indices));

	if (_Desc.NumVertices)
	{
		oStringL name;
		for (uint i = 0; i < oCOUNTOF(Vertices); i++)
		{
			VertexStrides[i] = oGfxCalcInterleavedVertexSize(_Desc.pElements, _Desc.NumElements, i);
			if (VertexStrides[i])
			{
				sprintf_s(name, "%sVertices[%02u]", _Name, i);
				oVERIFY(oD3D11CreateVertexBuffer(D3DDevice, name, true, 0, _Desc.NumVertices, VertexStrides[i], &Vertices[i]));
			}
		}
	}

	*_pSuccess = true;
}

bool oD3D11Mesh::Map(ID3D11DeviceContext* _pDeviceContext, size_t _SubresourceIndex, oGfxCommandList::MAPPED* _pMapped)
{
	switch (_SubresourceIndex)
	{
		case VERTICES0:
		case VERTICES1:
		case VERTICES2:
		{
			D3D11_MAPPED_SUBRESOURCE mapped;
			if (FAILED(_pDeviceContext->Map(Vertices[_SubresourceIndex-VERTICES0], 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
				return oWinSetLastError();
			_pMapped->pData = mapped.pData;
			_pMapped->RowPitch = mapped.RowPitch;
			_pMapped->SlicePitch = mapped.DepthPitch;
			return true;
		}
		
		case INDICES:
		{
			if (FAILED(_pDeviceContext->Map(Indices, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedIndices)))
				return oWinSetLastError();

			if (Uses16BitIndices)
			{
				p32BitIndices = new uint[Desc.NumIndices];
				_pMapped->pData = p32BitIndices;
				_pMapped->RowPitch = 0;
				_pMapped->SlicePitch = 0;
			}

			else
			{
				_pMapped->pData = MappedIndices.pData;
				_pMapped->RowPitch = MappedIndices.RowPitch;
				_pMapped->SlicePitch = MappedIndices.DepthPitch;
			}
			return true;
		}

		case RANGES:
		{
			RangesMutex.lock();
			_pMapped->pData = oGetData(Ranges);
			_pMapped->RowPitch = sizeof(oGfxMesh::RANGE);
			_pMapped->SlicePitch = 0;
			return true;
		}
		default:
			break;
	}
	
	return oErrorSetLast(oERROR_INVALID_PARAMETER, "Invalid Subresource");
}

void oD3D11Mesh::Unmap(ID3D11DeviceContext* _pDeviceContext, size_t _SubresourceIndex)
{
	switch (_SubresourceIndex)
	{
		case VERTICES0:
		case VERTICES1:
		case VERTICES2:
			_pDeviceContext->Unmap(Vertices[_SubresourceIndex-VERTICES0], 0);
			break;
		case INDICES:
		{
			if (Uses16BitIndices)
			{
				oASSERT(p32BitIndices, "");
				oMemcpyToUshort(static_cast<ushort*>(MappedIndices.pData), p32BitIndices, Desc.NumIndices);
			}
			
			_pDeviceContext->Unmap(Indices, 0);

			if (Uses16BitIndices)
			{
				delete [] p32BitIndices;
				p32BitIndices = nullptr;
				MappedIndices.pData = nullptr;
			}

			break;
		}

		case RANGES:
			RangesMutex.unlock();
			break;
		default:
			break;
	}
}
