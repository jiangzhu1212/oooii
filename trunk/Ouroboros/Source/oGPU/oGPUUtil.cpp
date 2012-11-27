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
#include <oGPU/oGPUUtil.h>
#include <oGPU/oGPUCompiler.h>
#include <oPlatform/oHLSLShaders.h> // for access to static shaders... can that be made more cross-platform?

int oGPUCalculateVertexSize(const oGPU_VERTEX_ELEMENT* _pElements, size_t _NumElements, int _InputSlot)
{
	bool IsFirstRun = true;
	bool IsInstanceList = false;

	int size = 0;
	for (size_t i = 0; i < _NumElements; i++)
	{
		if (_InputSlot == _pElements[i].InputSlot)
		{
			if (IsFirstRun)
			{
				IsInstanceList = _pElements[i].Instanced;
				IsFirstRun = false;
			}

			else
				oASSERT(IsInstanceList == _pElements[i].Instanced, "Elements in the same slot must either be all instanced or all not instanced.");

			size += oSurfaceFormatGetSize(_pElements[i].Format);
		}
	}

	return size;
}

int oGPUCalculateNumInputSlots(const oGPU_VERTEX_ELEMENT* _pElements, size_t _NumElements)
{
	int nSlots = 0;

	#ifdef _DEBUG
		int lastSlot = -1;
	#endif

	for (size_t i = 0; i < _NumElements; i++)
	{
		nSlots = __max(nSlots, _pElements[i].InputSlot+1);

		#ifdef _DEBUG
			oASSERT(lastSlot == _pElements[i].InputSlot || lastSlot == (_pElements[i].InputSlot - 1), "Non-packed elements");
			lastSlot = _pElements[i].InputSlot;
		#endif
	}

	return nSlots;
}

static void oGPUGetVertexSource(size_t _NumSemantics, const oFourCC* _pSupportedSemantics, const void** _ppData, const size_t* _pElementStrides, const oGPU_VERTEX_ELEMENT& _Element, oGPU_VERTEX_ELEMENT_DATA* _pElementData)
{
	for (size_t i = 0; i < _NumSemantics; i++)
		if (!_Element.Instanced && _Element.Semantic == _pSupportedSemantics[i])
		{
			_pElementData->pData = _ppData[i];
			_pElementData->Stride = _pElementStrides[i];
			break;
		}
}

static void oGPUGetVertexSource(const oGeometry::DESC& _Desc, oGeometry::CONST_MAPPED& GeoMapped, const oGPU_VERTEX_ELEMENT& _Element, oGPU_VERTEX_ELEMENT_DATA* _pElementData)
{
	static const oFourCC sSemantics[] =  { 'POS0', 'NML0', 'TAN0', 'TEX0', 'CLR0', 'CON0', };

	const void* sData[] = 
	{
		GeoMapped.pPositions,
		GeoMapped.pNormals,
		GeoMapped.pTangents,
		GeoMapped.pTexcoords,
		GeoMapped.pColors,
		nullptr,
	};
	const size_t sStrides[] = 
	{
		sizeof(*GeoMapped.pPositions),
		sizeof(*GeoMapped.pNormals),
		sizeof(*GeoMapped.pTangents),
		sizeof(*GeoMapped.pTexcoords),
		sizeof(*GeoMapped.pColors),
		0,
	};
	
	oGPUGetVertexSource(oCOUNTOF(sSemantics), sSemantics, sData, sStrides, _Element, _pElementData);
};

static void oGPUGetVertexSource(const threadsafe oOBJ* _pOBJ, const oGPU_VERTEX_ELEMENT& _Element, oGPU_VERTEX_ELEMENT_DATA* _pElementData)
{
	oOBJ_DESC d;
	_pOBJ->GetDesc(&d);
	static const oFourCC sSemantics[] = { 'POS0', 'NML0', 'TEX0', };
	const void* sData[] = { d.pPositions, d.pNormals, d.pTexcoords, };
	const size_t sStrides[] = { sizeof(*d.pPositions), sizeof(*d.pNormals), sizeof(*d.pTexcoords), };
	oGPUGetVertexSource(oCOUNTOF(sSemantics), sSemantics, sData, sStrides, _Element, _pElementData);
}

static bool oGPUReadVertexSource(oGPUDevice* _pDevice, oGPUMesh* _pMesh, uint _NumElements, const oGPU_VERTEX_ELEMENT* _pElements, const oGPU_VERTEX_ELEMENT_DATA* _ppElementData, const oFUNCTION<void(const oGPU_VERTEX_ELEMENT& _Element, oGPU_VERTEX_ELEMENT_DATA* _pElementData)>& _GetVertexSource)
{
	size_t offset[3];
	memset(offset, 0, sizeof(offset));

	size_t vertexStride[3];
	for (int i = 0; i < oCOUNTOF(vertexStride); i++)
		vertexStride[i] = oGPUCalculateVertexSize(_pElements, _NumElements, i);

	oRef<oGPUCommandList> ICL;
	_pDevice->GetImmediateCommandList(&ICL);

	oSURFACE_MAPPED_SUBRESOURCE msr[3];
	oFORI(i, msr)
	{
		if (vertexStride[i])
			ICL->Reserve(_pMesh, oGPU_MESH_SUBRESOURCE(oGPU_MESH_VERTICES0+i), &msr[i]);
		else
			memset(&msr[i], 0, sizeof(oSURFACE_MAPPED_SUBRESOURCE));
	}

	oOnScopeExit UnmapMSRs([&]
	{
		oFORI(i, msr)
		{
			if (msr[i].pData)
				ICL->Commit(_pMesh, oGPU_MESH_SUBRESOURCE(oGPU_MESH_VERTICES0+i), msr[i]);
		}
	});

	oGPUMesh::DESC d;
	_pMesh->GetDesc(&d);

	for (uint i = 0; i < _NumElements; i++)
	{
		int msrI = _pElements[i].InputSlot;
		if (msrI >= oCOUNTOF(msr))
			return oErrorSetLast(oERROR_INVALID_PARAMETER, "The %d%s element uses InputSlot %d, which is out of range for an oGPUMesh (must be 0 <= InputSlot <= 2)", oOrdinal(i), _pElements[i].InputSlot);

		oGPU_VERTEX_ELEMENT_DATA Source;
		size_t ElStride = oSurfaceFormatGetSize(_pElements[i].Format);
			
		// _ppElementData always overrides oGeometry, and then only look at 
		// oGeometry if index == 0 since oGeometry only has 1 channel for each
		// semantic it supports.
		if (_ppElementData && _ppElementData[i].pData)
			Source = _ppElementData[i];
		else
			_GetVertexSource(_pElements[i], &Source);

		void* pDestination = oByteAdd(msr[msrI].pData, offset[msrI]);
		offset[msrI] += ElStride;

		if (Source.pData)
			oMemcpy2d(pDestination, vertexStride[msrI], Source.pData, Source.Stride, ElStride, d.NumVertices);
		else
		{
			#ifdef _DEBUG
				char buf[5];
				oTRACE("No data for %s (%d%s IAElement) for mesh %s", oToString(buf, _pElements[i].Semantic), i, oOrdinal(i), _pMesh->GetName());
			#endif
			oMemset2d(pDestination, vertexStride[msrI], 0, ElStride, d.NumVertices);
		}
	}

	return true;
}

void oGPUCopyIndices(oSURFACE_MAPPED_SUBRESOURCE& _Destination, const oSURFACE_CONST_MAPPED_SUBRESOURCE& _Source, uint _NumIndices)
{
	if (_Destination.RowPitch == _Source.RowPitch)
		memcpy(_Destination.pData, _Source.pData, _NumIndices * _Destination.RowPitch);
	else if (_Destination.RowPitch == sizeof(uint) && _Source.RowPitch == sizeof(ushort))
		oMemcpyToUint((uint*)_Destination.pData, (const ushort*)_Source.pData, _NumIndices);
	else if (_Destination.RowPitch == sizeof(ushort) && _Source.RowPitch == sizeof(uint))
		oMemcpyToUshort((ushort*)_Destination.pData, (const uint*)_Source.pData, _NumIndices);
	else
		oASSERT(false, "Bad strides");
}

static bool oGPUCreateNormalsList(oGPUDevice* _pDevice, const char* _Name, uint _NumVertices, const float3* _pPositions, const float3* _pNormals, float _Scale, oColor _Color, oGPULineList** _ppLineList)
{
	oGPULineList::DESC d;
	d.MaxNumLines = _NumVertices;

	oRef<oGPULineList> LineList;
	if (!_pDevice->CreateLineList(_Name, d, &LineList))
		return false; // pass through error

	oRef<oGPUCommandList> ICL;
	_pDevice->GetImmediateCommandList(&ICL);

	oSURFACE_MAPPED_SUBRESOURCE msr;
	ICL->Reserve(LineList, 0, &msr);

	oGPU_LINE* pLines = (oGPU_LINE*)msr.pData;
	for (uint i = 0; i < _NumVertices; i++)
	{
		pLines[i].Start = _pPositions[i];
		pLines[i].End = pLines[i].Start + (_pNormals[i] * _Scale);
		pLines[i].StartColor = pLines[i].EndColor = _Color;
	}

	ICL->Commit(LineList, 0, msr, oGPU_BOX(_NumVertices));

	LineList->Reference();
	*_ppLineList = LineList;
	return true;
}

static bool oGPUCreateVertexNormals(oGPUDevice* _pDevice, const oGPUMesh* _pMesh, const float3* _pPositions, const float3* _pNormals, float _NormalScale, oColor _NormalColor, oGPULineList** _ppNormalLines)
{
	oGPUMesh::DESC md;
	_pMesh->GetDesc(&md);
	
	if (_NormalScale < 0.0f)
	{
		float r = md.LocalSpaceBounds.GetBoundingRadius();
		_NormalScale = r * abs(_NormalScale);
	}

	oStringURI Name(_pMesh->GetName());
	oStrAppendf(Name, "#Normals");

	if (!oGPUCreateNormalsList(_pDevice, Name, md.NumVertices, _pPositions, _pNormals, _NormalScale, _NormalColor, _ppNormalLines))
		return false; // pass through error

	return true;
}

bool oGPUCreateMesh(oGPUDevice* _pDevice, const char* _MeshName, const oGPU_VERTEX_ELEMENT* _pElements, uint _NumElements, const oGPU_VERTEX_ELEMENT_DATA* _ppElementData, const oGeometry* _pGeometry, oGPUMesh** _ppMesh, oGPULineList** _ppNormalLines, float _NormalScale, oColor _NormalColor)
{
	oCONSTRUCT_CLEAR(_ppMesh);

	oGeometry::DESC GeoDesc;
	_pGeometry->GetDesc(&GeoDesc);

	oGPUMesh::DESC d;
	d.LocalSpaceBounds = GeoDesc.Bounds;
	d.pElements = _pElements;
	d.NumElements = _NumElements;
	d.NumIndices = GeoDesc.NumIndices;
	d.NumRanges = GeoDesc.NumRanges;
	d.NumVertices = GeoDesc.NumVertices;

	oGeometry::CONST_MAPPED GeoMapped;
	if (!_pGeometry->MapConst(&GeoMapped))
		return false; // pass through error

	oOnScopeExit GeoUnmap([&]{ _pGeometry->UnmapConst(); });

	oRef<oGPUMesh> Mesh;
	if (!_pDevice->CreateMesh(_MeshName, d, &Mesh))
		return false; // pass through error

	oRef<oGPUCommandList> ICL;
	_pDevice->GetImmediateCommandList(&ICL);

	// Ranges
	{
		oSURFACE_MAPPED_SUBRESOURCE msr;
		ICL->Reserve(Mesh, oGPU_MESH_RANGES, &msr);
		memcpy(msr.pData, GeoMapped.pRanges, sizeof(oGPU_RANGE) * d.NumRanges);
		ICL->Commit(Mesh, oGPU_MESH_RANGES, msr);
	}

	// Indices
	{
		oSURFACE_MAPPED_SUBRESOURCE msr;
		ICL->Reserve(Mesh, oGPU_MESH_INDICES, &msr);

		oSURFACE_CONST_MAPPED_SUBRESOURCE smsr;
		smsr.pData = GeoMapped.pIndices;
		smsr.RowPitch = sizeof(uint);
		smsr.DepthPitch = smsr.RowPitch * d.NumIndices;
		oGPUCopyIndices(msr, smsr, d.NumIndices);

		ICL->Commit(Mesh, oGPU_MESH_INDICES, msr);
	}

	if (!oGPUReadVertexSource(_pDevice, Mesh, _NumElements, _pElements, _ppElementData, oBIND(oGPUGetVertexSource, GeoDesc, GeoMapped, oBIND1, oBIND2)))
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "Failed reading vertices");

	if (_ppNormalLines && !oGPUCreateVertexNormals(_pDevice, Mesh, GeoMapped.pPositions, GeoMapped.pNormals, _NormalScale, _NormalColor, _ppNormalLines))
		return false; // pass through error

	Mesh->Reference();
	*_ppMesh = Mesh;
	return true;
}

bool oGPUCreateMesh(oGPUDevice* _pDevice, const char* _MeshName, const oGPU_VERTEX_ELEMENT* _pElements, uint _NumElements, const oGPU_VERTEX_ELEMENT_DATA* _ppElementData, const threadsafe oOBJ* _pOBJ, oGPUMesh** _ppMesh, oGPULineList** _ppNormalLines, float _NormalScale, oColor _NormalColor)
{
	oOBJ_DESC OBJDesc;
	_pOBJ->GetDesc(&OBJDesc);

	float3 Min, Max;
	oCalculateMinMaxPoints(OBJDesc.pPositions, OBJDesc.NumVertices, &Min, &Max);

	oGPUMesh::DESC d;
	d.LocalSpaceBounds = oAABoxf(Min, Max);
	d.pElements = _pElements;
	d.NumElements = _NumElements;
	d.NumIndices = OBJDesc.NumIndices;
	d.NumRanges = OBJDesc.NumGroups;
	d.NumVertices = OBJDesc.NumVertices;

	oRef<oGPUMesh> Mesh;
	if (!_pDevice->CreateMesh(_MeshName, d, &Mesh))
		return false; // pass through error

	oRef<oGPUCommandList> ICL;
	_pDevice->GetImmediateCommandList(&ICL);

	// Ranges
	{
		oSURFACE_MAPPED_SUBRESOURCE msr;
		ICL->Reserve(Mesh, oGPU_MESH_RANGES, &msr);
		oGPU_RANGE* r = (oGPU_RANGE*)msr.pData;
		for (uint i = 0; i < d.NumRanges; i++)
			memcpy(r++, &OBJDesc.pGroups[i].Range, sizeof(oGPU_RANGE));
		ICL->Commit(Mesh, oGPU_MESH_RANGES, msr);
	}

	// Indices
	{
		oSURFACE_MAPPED_SUBRESOURCE msr;
		ICL->Reserve(Mesh, oGPU_MESH_INDICES, &msr);

		oSURFACE_CONST_MAPPED_SUBRESOURCE smsr;
		smsr.pData = OBJDesc.pIndices;
		smsr.RowPitch = sizeof(uint);
		smsr.DepthPitch = smsr.RowPitch * d.NumIndices;
		oGPUCopyIndices(msr, smsr, d.NumIndices);

		ICL->Commit(Mesh, oGPU_MESH_INDICES, msr);
	}

	if (!oGPUReadVertexSource(_pDevice, Mesh, _NumElements, _pElements, _ppElementData, oBIND(oGPUGetVertexSource, _pOBJ, oBIND1, oBIND2)))
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "Failed reading vertices");

	if (_ppNormalLines && !oGPUCreateVertexNormals(_pDevice, Mesh, OBJDesc.pPositions, OBJDesc.pNormals, _NormalScale, _NormalColor, _ppNormalLines))
		return false; // pass through error

	Mesh->Reference();
	*_ppMesh = Mesh;
	return true;
}

void oGPUInitMaterialConstants(const oOBJ_MATERIAL& _OBJMaterial, oGPUMaterialConstants* _pMaterialConstants)
{
	_pMaterialConstants->EmissiveColor = _OBJMaterial.AmbientColor;
	_pMaterialConstants->DiffuseColor = _OBJMaterial.DiffuseColor;
	_pMaterialConstants->SpecularColor = _OBJMaterial.SpecularColor;
	_pMaterialConstants->TransmissionColor = _OBJMaterial.TransmissionColor;
	_pMaterialConstants->Specularity = _OBJMaterial.Specularity;
	_pMaterialConstants->Opacity = _OBJMaterial.Transparency;
	_pMaterialConstants->IndexOfRefraction = _OBJMaterial.RefractionIndex;
}

bool oGPUCreateTexture1D(oGPUDevice* _pDevice, const oImage* _pSourceImage, oGPUTexture** _ppTexture)
{
	oImage::DESC id;
	_pSourceImage->GetDesc(&id);

	oGPUTexture::DESC td;
	td.Dimensions = int3(id.Dimensions, 1);
	td.Format = oImageFormatToSurfaceFormat(id.Format);
	td.NumSlices = 1;
	td.Type = oGPU_TEXTURE_1D_MAP;
	if (!_pDevice->CreateTexture(_pSourceImage->GetName(), td, _ppTexture))
		return false; // pass through error

	oRef<oGPUCommandList> ICL;
	_pDevice->GetImmediateCommandList(&ICL);

	oSURFACE_MAPPED_SUBRESOURCE msrImage;
	msrImage.pData = const_cast<void*>(_pSourceImage->GetData());
	msrImage.RowPitch = id.RowPitch;
	msrImage.DepthPitch = oImageCalcSize(id.Format, id.Dimensions);
	ICL->Commit(*_ppTexture, 0, msrImage);

	return true;
}

bool oGPUCreateTexture2D(oGPUDevice* _pDevice, const oImage* _pSourceImage, oGPUTexture** _ppTexture)
{
	oImage::DESC id;
	_pSourceImage->GetDesc(&id);

	oGPUTexture::DESC td;
	td.Dimensions = int3(id.Dimensions, 1);
	td.Format = oImageFormatToSurfaceFormat(id.Format);
	td.NumSlices = 1;
	td.Type = oGPU_TEXTURE_2D_MAP;
	if (!_pDevice->CreateTexture(_pSourceImage->GetName(), td, _ppTexture))
		return false; // pass through error

	oRef<oGPUCommandList> ICL;
	_pDevice->GetImmediateCommandList(&ICL);

	oSURFACE_MAPPED_SUBRESOURCE msrImage;
	msrImage.pData = const_cast<void*>(_pSourceImage->GetData());
	msrImage.RowPitch = id.RowPitch;
	msrImage.DepthPitch = oImageCalcSize(id.Format, id.Dimensions);
	ICL->Commit(*_ppTexture, 0, msrImage);

	return true;
}

bool oGPUCreateTexture3D(oGPUDevice* _pDevice, const oImage** _pSourceImages, uint _NumImages, oGPUTexture** _ppTexture)
{
	if (!_NumImages)
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "Need at least one source image");

	oImage::DESC id;
	_pSourceImages[0]->GetDesc(&id);

	for (uint imageIndex=0; imageIndex<_NumImages; ++imageIndex)
	{
		oImage::DESC idslice;
		_pSourceImages[imageIndex]->GetDesc(&idslice);

		if (idslice.Dimensions != id.Dimensions || idslice.Format != id.Format)
			return oErrorSetLast(oERROR_INVALID_PARAMETER, "Source images don't have the same dimensions and/or format");
	}

	oGPUTexture::DESC td;
	td.Dimensions = int3(id.Dimensions, _NumImages);
	td.Format = oImageFormatToSurfaceFormat(id.Format);
	td.NumSlices = 1;
	td.Type = oGPU_TEXTURE_3D_MAP;
	if (!_pDevice->CreateTexture(_pSourceImages[0]->GetName(), td, _ppTexture))
		return false; // pass through error

	oRef<oGPUCommandList> ICL;
	_pDevice->GetImmediateCommandList(&ICL);

	for (uint imageIndex=0; imageIndex<_NumImages; ++imageIndex)
	{
		oImage::DESC idslice;
		_pSourceImages[imageIndex]->GetDesc(&idslice);

		oSURFACE_MAPPED_SUBRESOURCE msrImage;
		msrImage.pData = const_cast<void*>(_pSourceImages[imageIndex]->GetData());
		msrImage.RowPitch = idslice.RowPitch;
		msrImage.DepthPitch = oImageCalcSize(idslice.Format, idslice.Dimensions);

		oGPU_BOX region;
		region.Right = idslice.Dimensions.x;
		region.Bottom = idslice.Dimensions.y;
		region.Front = imageIndex;
		region.Back = imageIndex + 1;

		ICL->Commit(*_ppTexture, 0, msrImage, region);
	}

	return true;
}

bool oGPUCreateTextureCube(oGPUDevice* _pDevice, const oImage** _pSourceImages, uint _NumImages, oGPUTexture** _ppTexture)
{
	if (!_NumImages)
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "Need at least one source image");

	oImage::DESC id;
	_pSourceImages[0]->GetDesc(&id);

	for (uint imageIndex=0; imageIndex<_NumImages; ++imageIndex)
	{
		oImage::DESC idslice;
		_pSourceImages[imageIndex]->GetDesc(&idslice);

		if (idslice.Dimensions != id.Dimensions || idslice.Format != id.Format)
			return oErrorSetLast(oERROR_INVALID_PARAMETER, "Source images don't have the same dimensions and/or format");
	}

	oGPUTexture::DESC td;
	td.Dimensions = int3(id.Dimensions, 1);
	td.Format = oImageFormatToSurfaceFormat(id.Format);
	td.NumSlices = _NumImages;
	td.Type = oGPU_TEXTURE_CUBE_MAP;
	if (!_pDevice->CreateTexture(_pSourceImages[0]->GetName(), td, _ppTexture))
		return false; // pass through error

	oRef<oGPUCommandList> ICL;
	_pDevice->GetImmediateCommandList(&ICL);

	for (uint imageIndex=0; imageIndex<_NumImages; ++imageIndex)
	{
		oImage::DESC idslice;
		_pSourceImages[imageIndex]->GetDesc(&idslice);

		int sliceIndex = imageIndex;
		int subresource = oSurfaceCalcSubresource(0, sliceIndex, 1);

		oSURFACE_MAPPED_SUBRESOURCE msrImage;
		msrImage.pData = const_cast<void*>(_pSourceImages[imageIndex]->GetData());
		msrImage.RowPitch = idslice.RowPitch;
		msrImage.DepthPitch = oImageCalcSize(idslice.Format, idslice.Dimensions);

		ICL->Commit(*_ppTexture, subresource, msrImage);
	}

	return true;
}

bool oGPUGenerateMips(oGPUDevice* _pDevice, const oImage** _pMip0Images, uint _NumImages, oSURFACE_DESC& _SurfaceDesc, oGPU_TEXTURE_TYPE _Type, oBuffer* _pMipBuffer)
{
	oGPUTexture::DESC rbd;
	rbd.Dimensions = _SurfaceDesc.Dimensions;
	rbd.Format = _SurfaceDesc.Format;
	rbd.NumSlices = _SurfaceDesc.NumSlices;
	rbd.Type = oGPUTextureTypeGetReadbackType(oGPUTextureTypeGetMipMapType(_Type));
	oRef<oGPUTexture> ReadbackTexture;
	_pDevice->CreateTexture("oGPUGenerateMips temporary readback texture", rbd, &ReadbackTexture);

	oGPUGenerateMips(_pDevice, _pMip0Images, _NumImages, ReadbackTexture);

	// Copy from readback texture to the output buffer

	int numMipLevels = oSurfaceCalcNumMips(_SurfaceDesc.Layout, _SurfaceDesc.Dimensions);
	for (int mipLevel=0; mipLevel<numMipLevels; ++mipLevel)
	{
		uint numIterations = oGPUTextureTypeIs3DMap(_Type) ? oSurfaceMipCalcDimensionsNPOT(_SurfaceDesc.Format, _SurfaceDesc.Dimensions, mipLevel).z : _NumImages;
		for (uint i=0; i<numIterations; ++i)
		{
			uint sliceIndex = oGPUTextureTypeIs3DMap(_Type) ? 0 : i;
			uint depthIndex = oGPUTextureTypeIs3DMap(_Type) ? i : 0;

			int subresource = oSurfaceCalcSubresource(mipLevel, sliceIndex, numMipLevels);

			int2 byteDimensions;
			oSURFACE_MAPPED_SUBRESOURCE msrMipDest;
			oSurfaceCalcMappedSubresource(_SurfaceDesc, subresource, depthIndex, _pMipBuffer->GetData(), &msrMipDest, &byteDimensions);

			oSURFACE_MAPPED_SUBRESOURCE msrMipSrc;
			_pDevice->MapRead(ReadbackTexture, subresource, &msrMipSrc, true);
			oSurfaceMappedSubresourceOffsetDepthIndex(msrMipSrc, depthIndex, &msrMipSrc.pData);

			oMemcpy2d(msrMipDest.pData, msrMipDest.RowPitch, msrMipSrc.pData, msrMipSrc.RowPitch, byteDimensions.x, byteDimensions.y, false);

			_pDevice->UnmapRead(ReadbackTexture, subresource);
		}
	}

	return true;
}

bool oGPUGenerateMips(oGPUDevice* _pDevice, const oImage** _pMip0Images, uint _NumImages, oGPUTexture* _pOutputTexture)
{
	oGPUTexture::DESC td;
	_pOutputTexture->GetDesc(&td);

	if ((oGPUTextureTypeIs3DMap(td.Type) && td.Dimensions.z != oInt(_NumImages)) ||
		(oGPUTextureTypeIsCubeMap(td.Type) && td.NumSlices != oInt(_NumImages)))
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "Number of mip0 images doesn't match the amount needed in the output texture");

#ifdef _DEBUG
	oImage::DESC id;
	_pMip0Images[0]->GetDesc(&id);

	for (uint imageIndex=0; imageIndex<_NumImages; ++imageIndex)
	{
		oImage::DESC idslice;
		_pMip0Images[imageIndex]->GetDesc(&idslice);

		if (idslice.Dimensions != id.Dimensions || idslice.Format != id.Format)
			return oErrorSetLast(oERROR_INVALID_PARAMETER, "Source images don't have the same dimensions and/or format");
	}
#endif

	oRef<oGPURenderTarget> SurfaceRenderTarget;
	oGPURenderTarget::DESC rtDesc;
	rtDesc.Dimensions = td.Dimensions;
	rtDesc.NumSlices = td.NumSlices;
	rtDesc.MRTCount = 1;
	rtDesc.Format[0] = td.Format;
	rtDesc.Type = oGPUTextureTypeStripReadbackType(oGPUTextureTypeGetMipMapType(td.Type));
	_pDevice->CreateRenderTarget("oGPUGenerateMips temporary render target", rtDesc, &SurfaceRenderTarget);

	oRef<oGPUTexture> Mip0Texture;
	SurfaceRenderTarget->GetTexture(0, &Mip0Texture);

	oRef<oGPUCommandList> ICL;
	_pDevice->GetImmediateCommandList(&ICL);

	int numMipLevels = oSurfaceCalcNumMips(oSURFACE_LAYOUT_TIGHT, td.Dimensions);
	for (uint imageIndex=0; imageIndex<_NumImages; ++imageIndex)
	{
		uint sliceIndex = oGPUTextureTypeIs3DMap(td.Type) ? 0 : imageIndex;
		uint depthIndex = oGPUTextureTypeIs3DMap(td.Type) ? imageIndex : 0;

		oImage::DESC idSlice;
		_pMip0Images[imageIndex]->GetDesc(&idSlice);

		int subresource = oSurfaceCalcSubresource(0, sliceIndex, numMipLevels);

		oSURFACE_MAPPED_SUBRESOURCE msrImage;
		msrImage.pData = const_cast<void*>(_pMip0Images[imageIndex]->GetData());
		msrImage.RowPitch = idSlice.RowPitch;
		msrImage.DepthPitch = oImageCalcSize(idSlice.Format, idSlice.Dimensions); // Shouldn't this simply be rowPitch*Dimensions.y ?

		oGPU_BOX region;
		region.Right = td.Dimensions.x;
		region.Bottom = td.Dimensions.y;
		region.Front = depthIndex;
		region.Back = depthIndex + 1;

		ICL->Commit(Mip0Texture, subresource, msrImage, region);
	}

	ICL->GenerateMips(SurfaceRenderTarget);

	ICL->Copy(_pOutputTexture, Mip0Texture);

	return true;
}

bool oGPUCreateTexture1DMip(oGPUDevice* _pDevice, const oImage* _pMip0Image, oGPUTexture** _ppTexture)
{
	oImage::DESC id;
	_pMip0Image->GetDesc(&id);

	if (id.Dimensions.y != 1)
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "1d textures can't have height");

	oGPUTexture::DESC td;
	td.Dimensions = int3(id.Dimensions, 1);
	td.Format = oImageFormatToSurfaceFormat(id.Format);
	td.NumSlices = 1;
	td.Type = oGPU_TEXTURE_1D_MAP_MIPS;
	if (!_pDevice->CreateTexture(_pMip0Image->GetName(), td, _ppTexture))
		return false; // pass through error

 	if (!oGPUGenerateMips(_pDevice, &_pMip0Image, 1, *_ppTexture))
 		return false; // pass through error

	return true;
}

bool oGPUCreateTexture2DMip(oGPUDevice* _pDevice, const oImage* _pMip0Image, oGPUTexture** _ppTexture)
{
	oImage::DESC id;
	_pMip0Image->GetDesc(&id);

	oGPUTexture::DESC td;
	td.Dimensions = int3(id.Dimensions, 1);
	td.Format = oImageFormatToSurfaceFormat(id.Format);
	td.NumSlices = 1;
	td.Type = oGPU_TEXTURE_2D_MAP_MIPS;
	if (!_pDevice->CreateTexture(_pMip0Image->GetName(), td, _ppTexture))
		return false; // pass through error

	if (!oGPUGenerateMips(_pDevice, &_pMip0Image, 1, *_ppTexture))
		return false; // pass through error

	return true;
}

bool oGPUCreateTexture3DMip(oGPUDevice* _pDevice, const oImage** _pMip0Images, uint _NumImages, oGPUTexture** _ppTexture)
{
	if (!_NumImages)
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "Need at least one source image");

	oImage::DESC id;
	_pMip0Images[0]->GetDesc(&id);

	oGPUTexture::DESC td;
	td.Dimensions = int3(id.Dimensions, _NumImages);
	td.Format = oImageFormatToSurfaceFormat(id.Format);
	td.NumSlices = 1;
	td.Type = oGPU_TEXTURE_3D_MAP_MIPS;
	if (!_pDevice->CreateTexture(_pMip0Images[0]->GetName(), td, _ppTexture))
		return false; // pass through error

	if (!oGPUGenerateMips(_pDevice, _pMip0Images, _NumImages, *_ppTexture))
		return false; // pass through error

	return true;
}

bool oGPUCreateTextureCubeMip(oGPUDevice* _pDevice, const oImage** _pMip0Images, uint _NumImages, oGPUTexture** _ppTexture)
{
	if (_NumImages != 6)
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "Need 6 Mip0 images");

	oImage::DESC id;
	_pMip0Images[0]->GetDesc(&id);

	oGPUTexture::DESC td;
	td.Dimensions = int3(id.Dimensions, 1);
	td.Format = oImageFormatToSurfaceFormat(id.Format);
	td.NumSlices = _NumImages;
	td.Type = oGPU_TEXTURE_CUBE_MAP_MIPS;
	if (!_pDevice->CreateTexture(_pMip0Images[0]->GetName(), td, _ppTexture))
		return false; // pass through error

	if (!oGPUGenerateMips(_pDevice, _pMip0Images, _NumImages, *_ppTexture))
		return false; // pass through error

	return true;
}

bool oGPUCreateTexture1DMip(oGPUDevice* _pDevice, oSURFACE_DESC& _SurfaceDesc, const oBuffer* _pBuffer, oGPUTexture** _ppTexture)
{
	if (_SurfaceDesc.NumSlices != 1)
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "Slices not yet supported in this function");

	if (_SurfaceDesc.Dimensions.y != 1)
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "1d textures can't have height");

	oGPUTexture::DESC td;
	td.Dimensions = _SurfaceDesc.Dimensions;
	td.Format = _SurfaceDesc.Format;
	td.NumSlices = _SurfaceDesc.NumSlices;
	td.Type = oGPU_TEXTURE_1D_MAP_MIPS;
	if (!_pDevice->CreateTexture(_pBuffer->GetName(), td, _ppTexture))
		return false; // pass through error

	oRef<oGPUCommandList> ICL;
	_pDevice->GetImmediateCommandList(&ICL);

	int numMipLevels = oSurfaceCalcNumMips(_SurfaceDesc.Layout, _SurfaceDesc.Dimensions);
	for (int mipLevel=0; mipLevel<numMipLevels; ++mipLevel)
	{
		int subresource = oSurfaceCalcSubresource(mipLevel, 0, numMipLevels);

		oSURFACE_MAPPED_SUBRESOURCE msrMip;
		oSurfaceCalcMappedSubresource(_SurfaceDesc, subresource, 0, const_cast<void*>(_pBuffer->GetData()), &msrMip);

		ICL->Commit(*_ppTexture, subresource, msrMip);
	}

	return true;
}

bool oGPUCreateTexture2DMip(oGPUDevice* _pDevice, oSURFACE_DESC& _SurfaceDesc, const oBuffer* _pBuffer, oGPUTexture** _ppTexture)
{
	if (_SurfaceDesc.NumSlices != 1)
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "Slices not yet supported in this function");

	oGPUTexture::DESC td;
	td.Dimensions = _SurfaceDesc.Dimensions;
	td.Format = _SurfaceDesc.Format;
	td.NumSlices = _SurfaceDesc.NumSlices;
	td.Type = oGPU_TEXTURE_2D_MAP_MIPS;
	if (!_pDevice->CreateTexture(_pBuffer->GetName(), td, _ppTexture))
		return false; // pass through error

	oRef<oGPUCommandList> ICL;
	_pDevice->GetImmediateCommandList(&ICL);

	int numMipLevels = oSurfaceCalcNumMips(_SurfaceDesc.Layout, _SurfaceDesc.Dimensions);
	for (int mipLevel=0; mipLevel<numMipLevels; ++mipLevel)
	{
		int subresource = oSurfaceCalcSubresource(mipLevel, 0, numMipLevels);

		oSURFACE_MAPPED_SUBRESOURCE msrMip;
		oSurfaceCalcMappedSubresource(_SurfaceDesc, subresource, 0, const_cast<void*>(_pBuffer->GetData()), &msrMip);

		ICL->Commit(*_ppTexture, subresource, msrMip);
	}

	return true;
}

bool oGPUCreateTexture3DMip(oGPUDevice* _pDevice, oSURFACE_DESC& _SurfaceDesc, const oBuffer* _pBuffer, oGPUTexture** _ppTexture)
{
	if (_SurfaceDesc.NumSlices != 1)
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "3d textures can't have slices, use oSURFACE_DESC.Depth to specify the depth");

	oGPUTexture::DESC td;
	td.Dimensions = _SurfaceDesc.Dimensions;
	td.Format = _SurfaceDesc.Format;
	td.NumSlices = 1;
	td.Type = oGPU_TEXTURE_3D_MAP_MIPS;
	if (!_pDevice->CreateTexture(_pBuffer->GetName(), td, _ppTexture))
		return false; // pass through error

	oRef<oGPUCommandList> ICL;
	_pDevice->GetImmediateCommandList(&ICL);

	int numMipLevels = oSurfaceCalcNumMips(_SurfaceDesc.Layout, _SurfaceDesc.Dimensions);

	for (int mipLevel=0; mipLevel<numMipLevels; ++mipLevel)
	{
		int3 mipDimensions = oSurfaceMipCalcDimensionsNPOT(_SurfaceDesc.Format, _SurfaceDesc.Dimensions, mipLevel);

		for (int depthIndex=0; depthIndex<mipDimensions.z; ++depthIndex)
		{
			int subresource = oSurfaceCalcSubresource(mipLevel, 0, numMipLevels);

			oSURFACE_MAPPED_SUBRESOURCE msrMip;
			oSurfaceCalcMappedSubresource(_SurfaceDesc, subresource, depthIndex, const_cast<void*>(_pBuffer->GetData()), &msrMip);

			oGPU_BOX region;
			region.Right = mipDimensions.x;
			region.Bottom = mipDimensions.y;
			region.Front = depthIndex;
			region.Back = depthIndex + 1;

			ICL->Commit(*_ppTexture, subresource, msrMip, region);
		}
	}

	return true;
}

bool oGPUCreateTextureCubeMip(oGPUDevice* _pDevice, oSURFACE_DESC& _SurfaceDesc, const oBuffer* _pBuffer, oGPUTexture** _ppTexture)
{
	if (_SurfaceDesc.NumSlices != 6)
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "Cube textures need 6 slices");

	oGPUTexture::DESC td;
	td.Dimensions = _SurfaceDesc.Dimensions;
	td.Format = _SurfaceDesc.Format;
	td.NumSlices = _SurfaceDesc.NumSlices;
	td.Type = oGPU_TEXTURE_CUBE_MAP_MIPS;
	if (!_pDevice->CreateTexture(_pBuffer->GetName(), td, _ppTexture))
		return false; // pass through error

	oRef<oGPUCommandList> ICL;
	_pDevice->GetImmediateCommandList(&ICL);

	int numMipLevels = oSurfaceCalcNumMips(_SurfaceDesc.Layout, _SurfaceDesc.Dimensions);

	for (int mipLevel=0; mipLevel<numMipLevels; ++mipLevel)
	{
		for (int sliceIndex=0; sliceIndex < _SurfaceDesc.NumSlices; ++sliceIndex)
		{
			int subresource = oSurfaceCalcSubresource(mipLevel, sliceIndex, numMipLevels);

			oSURFACE_MAPPED_SUBRESOURCE msrMip;
			oSurfaceCalcMappedSubresource(_SurfaceDesc, subresource, 0, const_cast<void*>(_pBuffer->GetData()), &msrMip);

			ICL->Commit(*_ppTexture, subresource, msrMip);
		}
	}

	return true;
}


void oGPUCommitBuffer(oGPUCommandList* _pCommandList, oGPUBuffer* _pBuffer, const void* _pStruct, uint _SizeofStruct, uint _NumStructs)
{
	oASSERT(oIsByteAligned(_SizeofStruct, 16), "Structs must be aligned to 16 bytes");
	oSURFACE_MAPPED_SUBRESOURCE msr;
	msr.pData = const_cast<void*>(_pStruct);
	msr.RowPitch = _SizeofStruct;
	msr.DepthPitch = _NumStructs * _SizeofStruct;
	_pCommandList->Commit(_pBuffer, 0, msr);
}

void oGPUCommitBuffer(oGPUDevice* _pDevice, oGPUBuffer* _pBuffer, const void* _pStruct, uint _SizeofStruct, uint _NumStructs)
{
	oASSERT(oIsByteAligned(_SizeofStruct, 16), "Structs must be aligned to 16 bytes");

	oRef<oGPUCommandList> ICL;
	_pDevice->GetImmediateCommandList(&ICL);

	oSURFACE_MAPPED_SUBRESOURCE msr;
	msr.pData = const_cast<void*>(_pStruct);
	msr.RowPitch = _SizeofStruct;
	msr.DepthPitch = _NumStructs * _SizeofStruct;
	ICL->Commit(_pBuffer, 0, msr);
}

bool oGPUCreateReadbackCopy(oGPUBuffer* _pSource, oGPUBuffer** _ppReadbackCopy)
{
	oRef<oGPUDevice> Device;
	_pSource->GetDevice(&Device);
	oGPUBuffer::DESC d;
	_pSource->GetDesc(&d);
	d.Type = oGPU_BUFFER_READBACK;
	oStringS Name;
	oPrintf(Name, "%s.Readback", _pSource->GetName());
	return Device->CreateBuffer(Name, d, _ppReadbackCopy);
}

uint oGPUReadbackCounter(oGPUBuffer* _pUnorderedBuffer, oGPUBuffer* _pPreallocatedReadbackBuffer)
{
	oGPUBuffer::DESC d;
	_pUnorderedBuffer->GetDesc(&d);
	if (d.Type != oGPU_BUFFER_UNORDERED_STRUCTURED_APPEND && d.Type != oGPU_BUFFER_UNORDERED_STRUCTURED_COUNTER)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER, "The specified buffer must be of type oGPU_BUFFER_UNORDERED_STRUCTURED_APPEND or oGPU_BUFFER_UNORDERED_STRUCTURED_COUNTER");
		return oInvalid;
	}

	oRef<oGPUDevice> Device;
	_pUnorderedBuffer->GetDevice(&Device);

	oRef<oGPUBuffer> Counter = _pPreallocatedReadbackBuffer;
	if (!Counter)
	{
		oStringS Name;
		oPrintf(Name, "%s.Readback", _pUnorderedBuffer->GetName());

		oGPUBuffer::DESC rb;
		rb.Type = oGPU_BUFFER_READBACK;
		rb.StructByteSize = sizeof(uint);

		if (!Device->CreateBuffer(Name, rb, &Counter))
			return oInvalid; // pass through error
	}

	oRef<oGPUCommandList> ICL;
	Device->GetImmediateCommandList(&ICL);
	ICL->CopyCounter(Counter, 0, _pUnorderedBuffer);

	oSURFACE_MAPPED_SUBRESOURCE msr;
	if (!Device->MapRead(Counter, 0, &msr, true))
		return oInvalid; // pass through error
	uint c = *(uint*)msr.pData;
	Device->UnmapRead(Counter, 0);
	return c;
}

bool oGPURead(oGPUResource* _pSourceResource, int _Subresource, oSURFACE_MAPPED_SUBRESOURCE& _Destination, bool _FlipVertically)
{
	oRef<oGPUDevice> Device;
	_pSourceResource->GetDevice(&Device);

	switch (_pSourceResource->GetType())
	{
		case oGPU_BUFFER:
		{
			oGPUBuffer::DESC d;
			static_cast<oGPUBuffer*>(_pSourceResource)->GetDesc(&d);
			if (d.Type != oGPU_BUFFER_READBACK)
				return oErrorSetLast(oERROR_INVALID_PARAMETER, "The specified resource %p %s must be a readback type.", _pSourceResource, _pSourceResource->GetName());
			break;
		}

		case oGPU_TEXTURE_RGB:
		{
			oGPUTexture::DESC d;
			static_cast<oGPUTexture*>(_pSourceResource)->GetDesc(&d);
			if (!oGPUTextureTypeIsReadback(d.Type))
				return oErrorSetLast(oERROR_INVALID_PARAMETER, "The specified resource %p %s must be a readback type.", _pSourceResource, _pSourceResource->GetName());
			break;
		}

		default:
			break;
	}

	oGPUDevice* pDevice = thread_cast<oGPUDevice*>(Device.c_ptr()); // @oooii-tony: review this one...

	oSURFACE_MAPPED_SUBRESOURCE source;
	if (!pDevice->MapRead(_pSourceResource, _Subresource, &source))
		return false; // pass through error

	int2 ByteDimensions = _pSourceResource->GetByteDimensions(_Subresource);
	oMemcpy2d(_Destination.pData, _Destination.RowPitch, source.pData, source.RowPitch, ByteDimensions.x, ByteDimensions.y, _FlipVertically);

	pDevice->UnmapRead(_pSourceResource, _Subresource);
	return true;
}

bool oGPUSaveImage(oGPUTexture* _pTexture, int _Subresource, interface oImage** _ppImage)
{
	oRef<oGPUTexture> TextureToSave = _pTexture;
	
	oRef<oGPUDevice> Device;
	_pTexture->GetDevice(&Device);

	oGPUTexture::DESC d;
	_pTexture->GetDesc(&d);

	if (!oGPUTextureTypeIsReadback(d.Type))
	{
		oStringURI Name(_pTexture->GetName());
		oStrAppendf(Name, "#CPUCopy");
		oGPUTexture::DESC CPUCopyDesc(d);
		CPUCopyDesc.Type = oGPUTextureTypeGetReadbackType(d.Type);
		if (!Device->CreateTexture(Name, CPUCopyDesc, &TextureToSave))
			return false; // pass through error

		oRef<oGPUCommandList> ICL;
		Device->GetImmediateCommandList(&ICL);
		ICL->Copy(TextureToSave, _pTexture);
	}

	oSURFACE_DESC sd;
	sd.Dimensions = d.Dimensions;
	sd.Format = d.Format;
	sd.Layout = oSURFACE_LAYOUT_IMAGE;
	sd.NumSlices = 1;

	oSURFACE_SUBRESOURCE_DESC ssrd;
	oSurfaceSubresourceGetDesc(sd, _Subresource, &ssrd);

	oRef<oImage> Image;
	oImage::DESC idesc;
	idesc.RowPitch = oImageCalcRowPitch(oImageFormatFromSurfaceFormat(sd.Format), ssrd.Dimensions.x);
	idesc.Dimensions = ssrd.Dimensions.xy;
	idesc.Format = oImageFormatFromSurfaceFormat(sd.Format);
	if (!oImageCreate("Temp Image", idesc, &Image))
		return false; // pass through error

	oSURFACE_MAPPED_SUBRESOURCE msr;
	msr.pData = Image->GetData();
	msr.RowPitch = idesc.RowPitch;
	msr.DepthPitch = oUInt(Image->GetSize());
	if (!oGPURead(TextureToSave, _Subresource, msr, false))
		return false;

	Image->Reference();
	*_ppImage = Image;
	return true;
}

struct oGPUMosaicImpl : oGPUMosaic
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oGPUMosaicImpl(oGPUDevice* _pDevice, const void* _pPixelShaderByteCode, bool* _pSuccess);

	bool Rebuild(const oGeometryFactory::MOSAIC_DESC& _Desc) override;
	void Draw(oGPUCommandList* _pCommandList, oGPURenderTarget* _pRenderTarget, uint _TextureStartSlot, uint _NumTextures, const oGPUTexture* const* _ppTextures) override;

	void SetBlendState(oGPU_BLEND_STATE _BlendState) override { BlendState = _BlendState; }

private:
	oRef<oGPUDevice> Device;
	oRef<oGPUPipeline> Pipeline;
	oRef<oGPUMesh> Mesh;
	oRefCount RefCount;

	oGPU_BLEND_STATE BlendState;
};

bool oGPUPipelineCreate(oGPUDevice* _pDevice, const char* _Name, oRef<oBuffer> _ByteCode[oGPU_PIPELINE_NUM_STAGES], const oGPU_VERTEX_ELEMENT* _pElements, size_t _NumElements, oGPUPipeline** _ppPipeline)
{
	oGPUPipeline::DESC d;
	d.DebugName = _Name;
	d.pElements = _pElements;
	d.NumElements = oUInt(_NumElements);
	if (_ByteCode[oGPU_VERTEX_SHADER]) d.pVertexShader = _ByteCode[oGPU_VERTEX_SHADER]->GetData();
	if (_ByteCode[oGPU_DOMAIN_SHADER]) d.pDomainShader = _ByteCode[oGPU_DOMAIN_SHADER]->GetData();
	if (_ByteCode[oGPU_HULL_SHADER]) d.pHullShader = _ByteCode[oGPU_HULL_SHADER]->GetData();
	if (_ByteCode[oGPU_GEOMETRY_SHADER]) d.pGeometryShader = _ByteCode[oGPU_GEOMETRY_SHADER]->GetData();
	if (_ByteCode[oGPU_PIXEL_SHADER]) d.pPixelShader = _ByteCode[oGPU_PIXEL_SHADER]->GetData();
	return _pDevice->CreatePipeline(_Name, d, _ppPipeline);
}

// Looks for macros (i.e. %MACRO% %DEV% etc.) and passes them to a user-
// specified function for resolution then replaces the macro
static bool oReplaceSystemValues(std::string& _String, oFUNCTION<char*(char* _StrDestination, size_t _SizeofStrDestination, const char* _Macro)> _Evaluator)
{
	static std::regex reMacro("%([A-Za-z0-9_]+)%", std::regex_constants::optimize);

	bool result = false;
	std::match_results<std::string::iterator> matches;

	while (std::regex_search(_String.begin(), _String.end(), matches, reMacro))
	{
		std::string macro = matches[1].str();
		oStringL resolved;
		if (!_Evaluator(resolved, resolved.capacity(), macro.c_str()))
			return oErrorSetLast(oERROR_NOT_FOUND, "Failed to resolve macro %s", macro.c_str());
		_String.replace(matches.position(), matches.length(), resolved);
	}

	return true;
}

static char* oResolvePathMacros(char* _StrDestination, size_t _SizeofStrDestination, const char* _Macro)
{
	oSYSPATH p;
	if (oFromString(&p, _Macro))
		return oSystemGetPath(_StrDestination, _SizeofStrDestination, p);
	oErrorSetLast(oERROR_NOT_FOUND, "Failed to resolve %%%s%% as an oSYSPATH", oSAFESTRN(_Macro));
	return nullptr;
}

static bool oGPUPipelineXMLCreate(const char* _ShaderPath, const char* _ShaderBody, threadsafe oXML** _ppXML)
{
	const char* start = strstr(_ShaderBody, "<oPL>");
	if (!start)
		return oErrorSetLast(oERROR_CORRUPT, "Could not find XML tag <oPL> in %s", _ShaderPath);
	const char* end = strstr(start, "</oPL>");
	if (!end)
		return oErrorSetLast(oERROR_CORRUPT, "Could not find matching closing XML tag </oPL> in %s", _ShaderPath);

	std::string StrXML(start, end+7);

	if (!oReplaceSystemValues(StrXML, oResolvePathMacros))
		return false; // pass through error

	oXML::DESC d;
	d.DocumentName = _ShaderPath;
	d.XMLString = (char*)StrXML.c_str();
	d.EstimatedNumNodes = 10;
	d.EstimatedNumAttributes = 100;
	d.CopyXMLString = true;

	if (!oXMLCreate(d, _ppXML))
		return false; // pass through error

	return true;
}

// @oooii-tony: Candidate for oSurface.h?
// i.e. converts the string "float3" to oSURFACE_R32G32B32_FLOAT
oSURFACE_FORMAT oSurfaceFromHLSLType(const char* _HLSLType)
{
	struct MAPPING { const char* Simple; oSURFACE_FORMAT Format; };
	static const MAPPING mapping[] = 
	{
		{ "float4", oSURFACE_R32G32B32A32_FLOAT },
		{ "float3", oSURFACE_R32G32B32_FLOAT },
		{ "float2", oSURFACE_R32G32_FLOAT },
		{ "float", oSURFACE_R32_FLOAT },

		{ "uint4", oSURFACE_R32G32B32A32_UINT },
		{ "uint3", oSURFACE_R32G32B32_UINT },
		{ "uint2", oSURFACE_R32G32_UINT },
		{ "uint", oSURFACE_R32_UINT },

		{ "int4", oSURFACE_R32G32B32A32_SINT },
		{ "int3", oSURFACE_R32G32B32_SINT },
		{ "int2", oSURFACE_R32G32_SINT },
		{ "int", oSURFACE_R32_SINT },

		{ "color", oSURFACE_B8G8R8A8_UNORM },
	};

	if (_HLSLType)
	{
		for (size_t i = 0; i < oCOUNTOF(mapping); i++)
			if (!oStricmp(_HLSLType, mapping[i].Simple))
				return mapping[i].Format;
	}
	return oSURFACE_UNKNOWN;
}

static bool oGPUPipelineParseIAs(const threadsafe oXML* _pXML, std::map<std::string, std::vector<oGPU_VERTEX_ELEMENT>>* _pIAs)
{
	oXML::HNODE hRoot = _pXML->GetFirstChild(0);
	oXML::HNODE hNode = _pXML->GetFirstChild(hRoot, "IA");

	while (hNode)
	{
		const char* NN = _pXML->FindAttributeValue(hNode, "Name");
		std::string NodeName = NN ? NN : "(null)";
		std::vector<oGPU_VERTEX_ELEMENT>& IA = (*_pIAs)[NodeName];
		oXML::HNODE hEl = _pXML->GetFirstChild(hNode, "El");
		while (hEl)
		{
			oGPU_VERTEX_ELEMENT e;
			e.Semantic = '????';
			e.Format = oSURFACE_UNKNOWN;
			e.InputSlot = 0;
			e.Instanced = false;

			const char* Semantic = _pXML->FindAttributeValue(hEl, "Semantic");
			if (!Semantic)
				return oErrorSetLast(oERROR_CORRUPT, "A vertex element must have a Semantic attribute that is a four-letter FOURCC code representing both the semantic name and index");

			if (!oFromString(&e.Semantic, Semantic))
				return oErrorSetLast(oERROR_CORRUPT, "Cannot interpret Semantic value \"%s\" as a fourcc code.", oSAFESTRN(Semantic));

			e.Format = oSurfaceFromHLSLType(_pXML->FindAttributeValue(hEl, "Format"));
			if (e.Format == oSURFACE_UNKNOWN)
				return oErrorSetLast(oERROR_CORRUPT, "Cannot convert Format=%s to an oSURFACE format", oSAFESTRN(_pXML->FindAttributeValue(hEl, "Format")));

			_pXML->GetTypedAttributeValue(hEl, "InputSlot", &e.InputSlot);
			if (e.InputSlot < 0 || e.InputSlot > 32)
				return oErrorSetLast(oERROR_CORRUPT, "InputSlot value must be between 0 and 32");

			_pXML->GetTypedAttributeValue(hEl, "Instanced", &e.Instanced);
			IA.push_back(e);
			hEl = _pXML->GetNextSibling(hEl, "El");
		}

		hNode = _pXML->GetNextSibling(hNode, "IA");
	}

	return true;
}

uint oGPUPipelineCompile(oGPUDevice* _pDevice, const char* _PipelineSourcePath, oGPUPipeline** _ppPipelines, uint _MaxNumPipelines)
{
	uint nPipelines = 0;

	oStringPath SDKPath;
	oSystemGetPath(SDKPath, oSYSPATH_DEV);

	oRef<oBuffer> source;
	oVERIFY(oBufferLoad(_PipelineSourcePath, &source, true));

	// Copy the XML metadata out and then zero the XML chars
	oRef<threadsafe oXML> XML;
	if (!oGPUPipelineXMLCreate(_PipelineSourcePath, source->GetData<const char>(), &XML))
		return false; // pass through error
	oZeroSection((char*)strstr(source->GetData<const char>(), "<oPL>"), "<oPL>", "</oPL>");

	oXML::HNODE hRoot = XML->GetFirstChild(0);
	oXML::HNODE hEnv = XML->GetFirstChild(hRoot, "CompilationEnvironment");
	if (!hEnv)
		return oErrorSetLast(oERROR_CORRUPT, "Could not find node <CompilationEnvironment /> in %s", _PipelineSourcePath);

	const char* IncludePaths = XML->FindAttributeValue(hEnv, "IncludePaths");
	const char* CommonDefines = XML->FindAttributeValue(hEnv, "Defines");
	oVersion ShaderModel = oVersion(4,0);
	if (!XML->GetTypedAttributeValue(hEnv, "ShaderModel", &ShaderModel))
		oTRACE("ShaderModel key/val not found, defaulting to shader model %d.%d", ShaderModel.Major, ShaderModel.Minor);

	std::map<std::string, std::vector<oGPU_VERTEX_ELEMENT>> IAs;

	if (!oGPUPipelineParseIAs(XML, &IAs))
		return false; // pass through error

	oXML::HNODE hNode = XML->GetFirstChild(hRoot, "Pipeline");
	while (hNode)
	{
		if (nPipelines >= _MaxNumPipelines)
			return oErrorSetLast(oERROR_AT_CAPACITY, "The max number of pipelines (%u) has been exceeded", _MaxNumPipelines);

		const char* NodeType = XML->GetNodeName(hNode);
		const char* NN = XML->FindAttributeValue(hNode, "Name");
		std::string NodeName = NN ? NN : "(null)";

		static const char* sKeys[] = { "VS", "HS", "DS", "GS", "PS", };
		static_assert(oCOUNTOF(sKeys) == oGPU_PIPELINE_NUM_STAGES, "# pipeline stages mismatch");

		oRef<oBuffer> ByteCode[oGPU_PIPELINE_NUM_STAGES];
		for (size_t i = 0; i < oCOUNTOF(sKeys); i++)
		{
			const char* EntryPoint = XML->FindAttributeValue(hNode, sKeys[i]);
			const char* SpecificDefines = XML->FindAttributeValue(hNode, "Defines");

			oRef<oBuffer> Errors;
			if (EntryPoint && !oGPUCompileShader(IncludePaths, CommonDefines, SpecificDefines, ShaderModel, (oGPU_PIPELINE_STAGE)i, _PipelineSourcePath, EntryPoint, source->GetData<const char>(), &ByteCode[i], &Errors))
			{
				oTRACE("compilation errors: %s:\n%s", _PipelineSourcePath, Errors->GetData());
				return false; // pass through error
			}
		}

		const char* IAName = XML->FindAttributeValue(hNode, "IA");
		if (!IAName)
			return oErrorSetLast(oERROR_CORRUPT, "An IA must be specified to the name of an IA node with a valid input assembly series of vertex elements");

		const std::vector<oGPU_VERTEX_ELEMENT>& IA = IAs[IAName];
		const oGPU_VERTEX_ELEMENT* pElements = oGetData(IA);
		uint NumElements = oUInt(IA.size());
		if (!pElements || !NumElements)
			return oErrorSetLast(oERROR_CORRUPT, "IA=%s not defined", IAName);

		if (!oGPUPipelineCreate(_pDevice, NodeName.c_str(), ByteCode, pElements, NumElements, &_ppPipelines[nPipelines++]))
			return 0; // pass through error

		hNode = XML->GetNextSibling(hNode, "Pipeline");
	}

	return nPipelines;
}

static const oGPU_VERTEX_ELEMENT sMosaicElements[] = 
{
	{ 'POS0', oSURFACE_R32G32B32_FLOAT, 0, false, },
	{ 'TEX0', oSURFACE_R32G32_FLOAT, 0, false, },
};

oGPUMosaicImpl::oGPUMosaicImpl(oGPUDevice* _pDevice, const void* _pPixelShaderByteCode, bool* _pSuccess)
	: Device(_pDevice)
	, BlendState(oGPU_OPAQUE)
{
	*_pSuccess = false;

	oGPUPipeline::DESC d;
	d.DebugName = "Mosaic.Pipeline";
	d.pElements = sMosaicElements;
	d.NumElements = oCOUNTOF(sMosaicElements);
	d.pVertexShader = oHLSLGetByteCode(oHLSL_VS4_0_QUAD_PASSTHROUGH); // @oooii-tony: This is platform-specific... maybe this is oShaderGetByteCode(...) one day?
	d.pPixelShader = _pPixelShaderByteCode;

	if (!Device->CreatePipeline("Mosaic.Pipeline", d, &Pipeline))
		return; // pass through error

	*_pSuccess = true;
}

bool oGPUMosaicCreate(oGPUDevice* _pDevice, const void* _pPixelShaderByteCode, oGPUMosaic** _ppMosaic)
{
	bool success = false;
	oCONSTRUCT(_ppMosaic, oGPUMosaicImpl(_pDevice, _pPixelShaderByteCode, &success));
	return success;
}

bool oGPUMosaicImpl::Rebuild(const oGeometryFactory::MOSAIC_DESC& _Desc)
{
	oRef<oGeometryFactory> GeoFactory;
	if (!oGeometryFactoryCreate(&GeoFactory))
		return false; // pass through error

	oGeometry::LAYOUT Layout;
	memset(&Layout, 0, sizeof(Layout));
	Layout.Positions = true;
	Layout.Texcoords = true;

	oRef<oGeometry> Geo;
	if (!GeoFactory->Create(_Desc, Layout, &Geo))
		return false; // pass through error

	oGeometry::DESC GeoDesc;
	Geo->GetDesc(&GeoDesc);

	oGeometry::CONST_MAPPED GeoMapped;
	if (!Geo->MapConst(&GeoMapped))
		return false; // pass through error

	oOnScopeExit OSEGeoUnmap([&] { Geo->UnmapConst(); });

	if (!oGPUCreateMesh(Device, "Mosaic.Mesh", sMosaicElements, oCOUNTOF(sMosaicElements), nullptr, Geo, &Mesh))
		return false; // pass through error

	return true;
}

void oGPUMosaicImpl::Draw(oGPUCommandList* _pCommandList, oGPURenderTarget* _pRenderTarget, uint _TextureStartSlot, uint _NumTextures, const oGPUTexture* const* _ppTextures)
{
	oGPU_SAMPLER_STATE samplers[oGPU_MAX_NUM_SAMPLERS];
	oINIT_ARRAY(samplers, oGPU_LINEAR_CLAMP);

	_pCommandList->SetRenderTarget(_pRenderTarget);
	_pCommandList->SetBlendState(BlendState);
	_pCommandList->SetSurfaceState(oGPU_TWO_SIDED);
	_pCommandList->SetDepthStencilState(oGPU_DEPTH_STENCIL_NONE);
	_pCommandList->SetSamplers(0, oGPU_MAX_NUM_SAMPLERS, samplers);
	_pCommandList->SetShaderResources(_TextureStartSlot, _NumTextures, _ppTextures);
	_pCommandList->SetPipeline(Pipeline);
	_pCommandList->Draw(Mesh, 0);
}
