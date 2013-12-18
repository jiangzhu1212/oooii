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
#include <oGPU/oGPUUtil.h>
#include <oBase/finally.h>

#include <oBasis/oMeshUtil.h>
#include <oBasis/oRefCount.h>

using namespace ouro;

void oGPUGetIndexBufferDesc(uint _NumIndices, uint _NumVertices, oGPU_BUFFER_DESC* _pDesc)
{
	_pDesc->Type = oGPU_BUFFER_INDEX;
	_pDesc->Format = oGPUHas16BitIndices(_NumVertices) ? surface::r16_uint : surface::r32_uint;
	_pDesc->ArraySize = _NumIndices;
	_pDesc->StructByteSize = oInvalid;
}

void oGPUGetVertexBufferDesc(uint _NumVertices, const oGPU_VERTEX_ELEMENT* _pElements, uint _NumElements, uint _InputSlot, oGPU_BUFFER_DESC* _pDesc)
{
	_pDesc->Type = oGPU_BUFFER_VERTEX;
	_pDesc->ArraySize = _NumVertices;
	_pDesc->StructByteSize = oGPUCalcVertexSize(_pElements, _NumElements, _InputSlot);
	_pDesc->Format = surface::unknown;
}

void oGPUCopyIndices(surface::mapped_subresource& _Destination, const surface::const_mapped_subresource& _Source, uint _NumIndices)
{
	if (_Destination.row_pitch == _Source.row_pitch)
		memcpy(_Destination.data, _Source.data, _NumIndices * _Destination.row_pitch);
	else if (_Destination.row_pitch == sizeof(uint) && _Source.row_pitch == sizeof(ushort))
		memcpyustoui((uint*)_Destination.data, (const ushort*)_Source.data, _NumIndices);
	else if (_Destination.row_pitch == sizeof(ushort) && _Source.row_pitch == sizeof(uint))
		memcpyuitous((ushort*)_Destination.data, (const uint*)_Source.data, _NumIndices);
	else
		oASSERT(false, "Bad strides");
}

void oGPUCommitIndexBuffer(oGPUCommandList* _pCommandList, const surface::const_mapped_subresource& _MappedSubresource, oGPUBuffer* _pIndexBuffer)
{
	oGPU_BUFFER_DESC d;
	_pIndexBuffer->GetDesc(&d);

	if (d.Format == surface::r32_uint)
		_pCommandList->Commit(_pIndexBuffer, 0, _MappedSubresource);
	else
	{
		surface::mapped_subresource MSRTemp;
		_pCommandList->Reserve(_pIndexBuffer, 0, &MSRTemp);
		oGPUCopyIndices(MSRTemp, _MappedSubresource, d.ArraySize);
		_pCommandList->Commit(_pIndexBuffer, 0, MSRTemp);
	}
}

bool oGPUCommitVertexBuffer(oGPUCommandList* _pCommandList
	, const std::function<void(const oGPU_VERTEX_ELEMENT& _Element, surface::const_mapped_subresource* _pElementData)>& _GetElementData
	, const oGPU_VERTEX_ELEMENT* _pElements
	, uint _NumElements
	, uint _InputSlot
	, oGPUBuffer* _pVertexBuffer)
{
	uint offset = 0;
	uint stride = oGPUCalcVertexSize(_pElements, _NumElements, _InputSlot);

	oGPU_BUFFER_DESC d;
	_pVertexBuffer->GetDesc(&d);

	surface::mapped_subresource msr;
	if (stride)
		_pCommandList->Reserve(_pVertexBuffer, 0, &msr);
	else
		memset(&msr, 0, sizeof(surface::mapped_subresource));

	finally UnmapMSRs([&]
	{
		if (msr.data)
			_pCommandList->Commit(_pVertexBuffer, 0, msr);
	});

	for (uint i = 0; i < _NumElements; i++)
	{
		if (_InputSlot == _pElements[i].InputSlot)
		{
			surface::const_mapped_subresource Source;
			_GetElementData(_pElements[i], &Source);
			uint ElStride = surface::element_size(_pElements[i].Format);
			oASSERT((offset + ElStride) <= stride, "write will be out-of-bounds");
			void* pDestination = byte_add(msr.data, offset);
			offset += ElStride;

			if (Source.data)
				memcpy2d(pDestination, stride, Source.data, Source.row_pitch, ElStride, d.ArraySize);
			else
			{
				#ifdef _DEBUG
					char buf[5];
					oTRACE("No data for %s (%d%s IAElement) for VB %s", to_string(buf, _pElements[i].Semantic), i, ordinal(i), _pVertexBuffer->GetName());
				#endif
				memset2d(pDestination, stride, 0, ElStride, d.ArraySize);
			}
		}
	}

	return true;
}

void oGPUCommitBuffer(oGPUCommandList* _pCommandList, oGPUBuffer* _pBuffer, const void* _pStruct, uint _SizeofStruct, uint _NumStructs)
{
	oASSERT(byte_aligned(_SizeofStruct, 16), "Structs must be aligned to 16 bytes (%u bytes specified %.02f of alignment)", _SizeofStruct, _SizeofStruct / 16.0f);
	surface::mapped_subresource msr;
	msr.data = const_cast<void*>(_pStruct);
	msr.row_pitch = _SizeofStruct;
	msr.depth_pitch = _NumStructs * _SizeofStruct;
	_pCommandList->Commit(_pBuffer, 0, msr);
}

void oGPUCommitBuffer(oGPUDevice* _pDevice, oGPUBuffer* _pBuffer, const void* _pStruct, uint _SizeofStruct, uint _NumStructs)
{
	oASSERT(byte_aligned(_SizeofStruct, 16), "Structs must be aligned to 16 bytes");

	intrusive_ptr<oGPUCommandList> ICL;
	_pDevice->GetImmediateCommandList(&ICL);

	surface::mapped_subresource msr;
	msr.data = const_cast<void*>(_pStruct);
	msr.row_pitch = _SizeofStruct;
	msr.depth_pitch = _NumStructs * _SizeofStruct;
	ICL->Commit(_pBuffer, 0, msr);
}

bool oGPUCreateIndexBuffer(oGPUDevice* _pDevice
	, const char* _Name
	, uint _NumIndices
	, uint _NumVertices
	, const surface::const_mapped_subresource& _MappedSubresource
	, oGPUBuffer** _ppIndexBuffer)
{
	oGPU_BUFFER_DESC d;
	oGPUGetIndexBufferDesc(_NumIndices, _NumVertices, &d);
	if (!_pDevice->CreateBuffer(_Name, d, _ppIndexBuffer))
		return false;

	if (_MappedSubresource.data)
	{
		intrusive_ptr<oGPUCommandList> ICL;
		_pDevice->GetImmediateCommandList(&ICL);
		oGPUCommitIndexBuffer(ICL, _MappedSubresource, *_ppIndexBuffer);
	}

	return true;
}

bool oGPUCreateVertexBuffer(oGPUDevice* _pDevice
	, const char* _Name	
	, uint _NumVertices
	, const std::function<void(const oGPU_VERTEX_ELEMENT& _Element, surface::const_mapped_subresource* _pElementData)>& _GetElementData
	, uint _NumElements
	, const oGPU_VERTEX_ELEMENT* _pElements
	, uint _InputSlot
	, oGPUBuffer** _ppVertexBuffer)
{
	if (_NumElements > oGPU_MAX_NUM_VERTEX_ELEMENTS)
		return oErrorSetLast(std::errc::invalid_argument, "Too many vertex elements specified");

	oGPU_BUFFER_DESC d;
	d.Type = oGPU_BUFFER_VERTEX;
	d.ArraySize = _NumVertices;
	d.StructByteSize = oGPUCalcVertexSize(_pElements, _NumElements, _InputSlot);

	if (!d.StructByteSize)
		return oErrorSetLast(std::errc::protocol_error, "InputSlot %u has no entries", _InputSlot);

	if (!_pDevice->CreateBuffer(_Name, d, _ppVertexBuffer))
		return false; // pass through error

	if (!!_GetElementData)
	{
		intrusive_ptr<oGPUCommandList> ICL;
		_pDevice->GetImmediateCommandList(&ICL);

		if (!oGPUCommitVertexBuffer(ICL, _GetElementData, _pElements, _NumElements, _InputSlot, *_ppVertexBuffer))
			return false; // pass through error
	}

	return true;
}

static void oGPUGetVertexSource(uint _NumSemantics
	, const fourcc* _pSupportedSemantics
	, const void** _ppData
	, const uint* _pElementStrides
	, const oGPU_VERTEX_ELEMENT& _Element
	, surface::const_mapped_subresource* _pElementData)
{
	for (uint i = 0; i < _NumSemantics; i++)
		if (!_Element.Instanced && _Element.Semantic == _pSupportedSemantics[i])
		{
			_pElementData->data = _ppData[i];
			_pElementData->row_pitch = _pElementStrides[i];
			_pElementData->depth_pitch = 0;
			break;
		}
}

static void oGPUGetVertexSourceFromGeometry(const oGeometry::DESC& _Desc
	, oGeometry::CONST_MAPPED& _GeoMapped
	, const oGPU_VERTEX_ELEMENT& _Element
	, surface::const_mapped_subresource* _pElementData)
{
	static const fourcc sSemantics[] =  { 'POS0', 'NML0', 'TAN0', 'TEX0', 'CLR0', 'CON0', };

	const void* sData[] = 
	{
		_GeoMapped.pPositions,
		_GeoMapped.pNormals,
		_GeoMapped.pTangents,
		_GeoMapped.pTexcoords,
		_GeoMapped.pColors,
		nullptr,
	};
	const uint sStrides[] = 
	{
		sizeof(*_GeoMapped.pPositions),
		sizeof(*_GeoMapped.pNormals),
		sizeof(*_GeoMapped.pTangents),
		sizeof(*_GeoMapped.pTexcoords),
		sizeof(*_GeoMapped.pColors),
		0,
	};

	oGPUGetVertexSource(oCOUNTOF(sSemantics), sSemantics, sData, sStrides, _Element, _pElementData);
};

bool oGPUCreateVertexBuffer(oGPUDevice* _pDevice
	, const char* _Name	
	, uint _NumVertices
	, const oGeometry::DESC& _GeoDesc, const oGeometry::CONST_MAPPED& _GeoMapped
	, uint _NumElements
	, const oGPU_VERTEX_ELEMENT* _pElements
	, uint _InputSlot
	, oGPUBuffer** _ppVertexBuffer)
{
	return oGPUCreateVertexBuffer(_pDevice
		, _Name
		, _NumVertices
		, std::bind(oGPUGetVertexSourceFromGeometry, _GeoDesc, _GeoMapped, oBIND1, oBIND2)
		, _NumElements
		, _pElements
		, _InputSlot
		, _ppVertexBuffer);
}

bool oGPUCreateReadbackCopy(oGPUBuffer* _pSource, oGPUBuffer** _ppReadbackCopy)
{
	intrusive_ptr<oGPUDevice> Device;
	_pSource->GetDevice(&Device);
	oGPUBuffer::DESC d;
	_pSource->GetDesc(&d);
	d.Type = oGPU_BUFFER_READBACK;
	sstring Name;
	snprintf(Name, "%s.Readback", _pSource->GetName());
	return Device->CreateBuffer(Name, d, _ppReadbackCopy);
}

uint oGPUReadbackCounter(oGPUBuffer* _pUnorderedBuffer, oGPUBuffer* _pPreallocatedReadbackBuffer)
{
	oGPUBuffer::DESC d;
	_pUnorderedBuffer->GetDesc(&d);
	if (d.Type != oGPU_BUFFER_UNORDERED_STRUCTURED_APPEND && d.Type != oGPU_BUFFER_UNORDERED_STRUCTURED_COUNTER)
	{
		oErrorSetLast(std::errc::invalid_argument, "The specified buffer must be of type oGPU_BUFFER_UNORDERED_STRUCTURED_APPEND or oGPU_BUFFER_UNORDERED_STRUCTURED_COUNTER");
		return oInvalid;
	}

	intrusive_ptr<oGPUDevice> Device;
	_pUnorderedBuffer->GetDevice(&Device);

	intrusive_ptr<oGPUBuffer> Counter = _pPreallocatedReadbackBuffer;
	if (!Counter)
	{
		sstring Name;
		snprintf(Name, "%s.Readback", _pUnorderedBuffer->GetName());

		oGPUBuffer::DESC rb;
		rb.Type = oGPU_BUFFER_READBACK;
		rb.StructByteSize = sizeof(uint);

		if (!Device->CreateBuffer(Name, rb, &Counter))
			return oInvalid; // pass through error
	}

	intrusive_ptr<oGPUCommandList> ICL;
	Device->GetImmediateCommandList(&ICL);
	ICL->CopyCounter(Counter, 0, _pUnorderedBuffer);

	surface::mapped_subresource msr;
	if (!Device->MapRead(Counter, 0, &msr, true))
		return oInvalid; // pass through error
	uint c = *(uint*)msr.data;
	Device->UnmapRead(Counter, 0);
	return c;
}

bool oGPURead(oGPUResource* _pSourceResource, int _Subresource, surface::mapped_subresource& _Destination, bool _FlipVertically)
{
	intrusive_ptr<oGPUDevice> Device;
	_pSourceResource->GetDevice(&Device);

	switch (_pSourceResource->GetType())
	{
	case oGPU_BUFFER:
		{
			oGPUBuffer::DESC d;
			static_cast<oGPUBuffer*>(_pSourceResource)->GetDesc(&d);
			if (d.Type != oGPU_BUFFER_READBACK)
				return oErrorSetLast(std::errc::invalid_argument, "The specified resource %p %s must be a readback type.", _pSourceResource, _pSourceResource->GetName());
			break;
		}

	case oGPU_TEXTURE:
		{
			oGPUTexture::DESC d;
			static_cast<oGPUTexture*>(_pSourceResource)->GetDesc(&d);
			if (!oGPUTextureTypeIsReadback(d.Type))
				return oErrorSetLast(std::errc::invalid_argument, "The specified resource %p %s must be a readback type.", _pSourceResource, _pSourceResource->GetName());
			break;
		}

	default:
		break;
	}

	oGPUDevice* pDevice = thread_cast<oGPUDevice*>(Device.c_ptr()); // @tony: review this one...

	surface::mapped_subresource source;
	if (!pDevice->MapRead(_pSourceResource, _Subresource, &source))
		return false; // pass through error

	int2 ByteDimensions = _pSourceResource->GetByteDimensions(_Subresource);
	memcpy2d(_Destination.data, _Destination.row_pitch, source.data, source.row_pitch, ByteDimensions.x, ByteDimensions.y, _FlipVertically);

	pDevice->UnmapRead(_pSourceResource, _Subresource);
	return true;
}

bool oGPUGenerateMips(oGPUDevice* _pDevice, oGPUTexture* _pTexture)
{
	oGPUTexture::DESC td;
	_pTexture->GetDesc(&td);

	intrusive_ptr<oGPURenderTarget> RT;
	oGPURenderTarget::DESC RTDesc;
	RTDesc.Dimensions = td.Dimensions;
	RTDesc.ArraySize = td.ArraySize;
	RTDesc.MRTCount = 1;
	RTDesc.Format[0] = td.Format;
	RTDesc.Type = oGPUTextureTypeStripReadbackType(oGPUTextureTypeGetMipMapType(td.Type));
	oVERIFY(_pDevice->CreateRenderTarget("oGPUGenerateMips.TempRT", RTDesc, &RT));

	intrusive_ptr<oGPUTexture> RTTexture;
	RT->GetTexture(0, &RTTexture);

	intrusive_ptr<oGPUCommandList> ICL;
	_pDevice->GetImmediateCommandList(&ICL);

	ICL->Copy(RTTexture, _pTexture);
	ICL->GenerateMips(RT);
	ICL->Copy(_pTexture, RTTexture);

	return true;
}

bool oGPUCreateTexture(oGPUDevice* _pDevice, const surface::buffer* const* _ppSourceImages, uint _NumImages, oGPU_TEXTURE_TYPE _Type, oGPUTexture** _ppTexture)
{
	if (!_NumImages)
		return oErrorSetLast(std::errc::invalid_argument, "Need at least one source image");

	surface::info si = _ppSourceImages[0]->get_info();

	intrusive_ptr<oGPUTexture> Texture;
	oGPUTexture::DESC td;
	td.Format = si.format;
	td.Type = _Type;
	td.Dimensions = int3(si.dimensions.xy(), oGPUTextureTypeIs3DMap(_Type) ? _NumImages : 1);
	td.ArraySize = oGPUTextureTypeIs3DMap(_Type) ? 1 : _NumImages;

	switch (oGPUTextureTypeGetBasicType(_Type))
	{
		case oGPU_TEXTURE_1D_MAP:
			if (si.dimensions.y != 1)
				return oErrorSetLast(std::errc::invalid_argument, "1D textures cannot have height");
			break;

		case oGPU_TEXTURE_CUBE_MAP:
			if (((_NumImages) % 6) != 0)
				return oErrorSetLast(std::errc::invalid_argument, "Cube maps must be specified in sets of 6");
			break;

		default:
			break;
	}

	if (!_pDevice->CreateTexture("oGPUCreateTexture", td, &Texture))
		return false; // pass through error

	intrusive_ptr<oGPUCommandList> ICL;
	_pDevice->GetImmediateCommandList(&ICL);

	const int NumMips = surface::num_mips(oGPUTextureTypeHasMips(_Type), td.Dimensions);

	for (uint i = 0; i < _NumImages; i++)
	{
		surface::info sislice = _ppSourceImages[i]->get_info();
		if (any(sislice.dimensions != si.dimensions) || sislice.format != si.format)
			return oErrorSetLast(std::errc::invalid_argument, "Source images don't have the same dimensions and/or format");

		int RegionFront = oGPUTextureTypeIs3DMap(_Type) ? i : 0;

		surface::box region;
		region.right = sislice.dimensions.x;
		region.bottom = sislice.dimensions.y;
		region.front = RegionFront;
		region.back = region.front + 1;

		int ArraySlice = oGPUTextureTypeIs3DMap(_Type) ? 0 : i;

		int subresource = surface::calc_subresource(0, ArraySlice, 0, NumMips, td.ArraySize);
		surface::shared_lock lock(_ppSourceImages[i]);
		ICL->Commit(Texture, subresource, lock.mapped, region);
	}

	if (oGPUTextureTypeHasMips(_Type) && !oGPUGenerateMips(_pDevice, Texture))
		return false; // pass through error

	*_ppTexture = Texture;
	(*_ppTexture)->Reference();
	return true;
}

static bool DEPRECATED_oGPUGenerateMips(oGPUDevice* _pDevice, const surface::buffer** _pMip0Images, uint _NumImages, oGPUTexture* _pOutputTexture)
{
	oGPUTexture::DESC td;
	_pOutputTexture->GetDesc(&td);

	if ((oGPUTextureTypeIs3DMap(td.Type) && td.Dimensions.z != oInt(_NumImages)) ||
		(oGPUTextureTypeIsCubeMap(td.Type) && td.ArraySize != oInt(_NumImages)))
		return oErrorSetLast(std::errc::invalid_argument, "Number of mip0 images doesn't match the amount needed in the output texture");

#ifdef _DEBUG
	surface::info si = _pMip0Images[0]->get_info();

	for (uint imageIndex=0; imageIndex<_NumImages; ++imageIndex)
	{
		surface::info sislice = _pMip0Images[imageIndex]->get_info();

		if (any(sislice.dimensions != si.dimensions) || sislice.format != si.format)
			return oErrorSetLast(std::errc::invalid_argument, "Source images don't have the same dimensions and/or format");
	}
#endif

	intrusive_ptr<oGPURenderTarget> SurfaceRenderTarget;
	oGPURenderTarget::DESC rtDesc;
	rtDesc.Dimensions = td.Dimensions;
	rtDesc.ArraySize = td.ArraySize;
	rtDesc.MRTCount = 1;
	rtDesc.Format[0] = td.Format;
	rtDesc.Type = oGPUTextureTypeStripReadbackType(oGPUTextureTypeGetMipMapType(td.Type));
	oVERIFY(_pDevice->CreateRenderTarget("oGPUGenerateMips temporary render target", rtDesc, &SurfaceRenderTarget));

	intrusive_ptr<oGPUTexture> Mip0Texture;
	SurfaceRenderTarget->GetTexture(0, &Mip0Texture);

	intrusive_ptr<oGPUCommandList> ICL;
	_pDevice->GetImmediateCommandList(&ICL);

	int numMipLevels = surface::num_mips(surface::tight, td.Dimensions);
	for (uint imageIndex = 0; imageIndex < _NumImages; imageIndex++)
	{
		uint sliceIndex = oGPUTextureTypeIs3DMap(td.Type) ? 0 : imageIndex;
		uint depthIndex = oGPUTextureTypeIs3DMap(td.Type) ? imageIndex : 0;

		int subresource = surface::calc_subresource(0, sliceIndex, 0, numMipLevels, td.ArraySize);
		surface::mapped_subresource msrImage;
		
		surface::box region;
		region.right = td.Dimensions.x;
		region.bottom = td.Dimensions.y;
		region.front = depthIndex;
		region.back = depthIndex + 1;

		surface::shared_lock lock(_pMip0Images[imageIndex]);
		ICL->Commit(Mip0Texture, subresource, lock.mapped, region);
	}

	ICL->GenerateMips(SurfaceRenderTarget);

	ICL->Copy(_pOutputTexture, Mip0Texture);

	return true;
}

bool oGPUGenerateMips(oGPUDevice* _pDevice, const surface::buffer** _pMip0Images, uint _NumImages, surface::info& _SurfaceInfo, oGPU_TEXTURE_TYPE _Type, surface::buffer* _pMipBuffer)
{
	oGPUTexture::DESC rbd;
	rbd.Dimensions = _SurfaceInfo.dimensions;
	rbd.Format = _SurfaceInfo.format;
	rbd.ArraySize = _SurfaceInfo.array_size;
	rbd.Type = oGPUTextureTypeGetReadbackType(oGPUTextureTypeGetMipMapType(_Type));
	intrusive_ptr<oGPUTexture> ReadbackTexture;
	_pDevice->CreateTexture("oGPUGenerateMips temporary readback texture", rbd, &ReadbackTexture);

	DEPRECATED_oGPUGenerateMips(_pDevice, _pMip0Images, _NumImages, ReadbackTexture);

	// Copy from readback texture to the output buffer

	int numMipLevels = surface::num_mips(_SurfaceInfo.layout, _SurfaceInfo.dimensions);
	for (int mipLevel=0; mipLevel<numMipLevels; ++mipLevel)
	{
		uint numIterations = oGPUTextureTypeIs3DMap(_Type) ? surface::dimensions_npot(_SurfaceInfo.format, _SurfaceInfo.dimensions, mipLevel).z : _NumImages;
		for (uint i=0; i<numIterations; ++i)
		{
			uint sliceIndex = oGPUTextureTypeIs3DMap(_Type) ? 0 : i;
			uint depthIndex = oGPUTextureTypeIs3DMap(_Type) ? i : 0;

			int subresource = surface::calc_subresource(mipLevel, sliceIndex, 0, numMipLevels, _SurfaceInfo.array_size);

			surface::mapped_subresource msrMipDest;
			surface::lock_guard lock(_pMipBuffer, subresource);
			void* dst_data = depth_index_offset(lock.mapped, depthIndex);
			
			surface::mapped_subresource msrMipSrc;
			_pDevice->MapRead(ReadbackTexture, subresource, &msrMipSrc, true);
			msrMipSrc.data = surface::depth_index_offset(msrMipSrc, depthIndex);

			memcpy2d(dst_data, lock.mapped.row_pitch, msrMipSrc.data, msrMipSrc.row_pitch, lock.byte_dimensions.x, lock.byte_dimensions.y, false);

			_pDevice->UnmapRead(ReadbackTexture, subresource);
		}
	}

	return true;
}

std::shared_ptr<surface::buffer> oGPUSaveImage(oGPUTexture* _pTexture, int _Subresource)
{
	intrusive_ptr<oGPUTexture> TextureToSave = _pTexture;
	
	intrusive_ptr<oGPUDevice> Device;
	_pTexture->GetDevice(&Device);

	oGPUTexture::DESC d;
	_pTexture->GetDesc(&d);

	if (!oGPUTextureTypeIsReadback(d.Type))
	{
		uri_string Name(_pTexture->GetName());
		sncatf(Name, "#CPUCopy");
		oGPUTexture::DESC CPUCopyDesc(d);
		CPUCopyDesc.Type = oGPUTextureTypeGetReadbackType(d.Type);
		if (!Device->CreateTexture(Name, CPUCopyDesc, &TextureToSave))
			return false; // pass through error

		intrusive_ptr<oGPUCommandList> ICL;
		Device->GetImmediateCommandList(&ICL);
		ICL->Copy(TextureToSave, _pTexture);
	}

	surface::info si;
	si.dimensions = d.Dimensions;
	si.format = d.Format;
	si.layout = surface::image;

	surface::subresource_info sri = surface::subresource(si, _Subresource);
	si.dimensions = sri.dimensions;
	auto s = surface::buffer::make(si);
	surface::lock_guard lock(s);
	if (!oGPURead(TextureToSave, _Subresource, lock.mapped, false))
		oThrowLastError();

	return s;
}

struct oOBJExtraVertexData
{
	std::vector<float4> Tangents;
};

static void oGPUGetVertexSourceFromOBJ(const threadsafe oOBJ* _pOBJ, oOBJExtraVertexData* _pExtra, const oGPU_VERTEX_ELEMENT& _Element, surface::const_mapped_subresource* _pElementData)
{
	oOBJ_DESC d;
	_pOBJ->GetDesc(&d);

	if (_Element.Semantic == 'TAN0' && _pExtra->Tangents.empty())
	{
		_pExtra->Tangents.resize(d.NumVertices);

		if (d.pPositions && d.pNormals && d.pTexcoords)
		{
			oTRACE("Calculating Tangents...");
			oCalcTangents(data(_pExtra->Tangents), d.pIndices, d.NumIndices, d.pPositions, d.pNormals, d.pTexcoords, d.NumVertices);
		}

		else
			oTRACE("Cannot calculate tangents, missing:%s%s%s", d.pPositions ? "" : " Positions", d.pNormals ? "" : " Normals", d.pTexcoords ? "" : " Texcoords");
	}

	static const fourcc sSemantics[] = { 'POS0', 'NML0', 'TEX0', 'TAN0' };
	const void* sData[] = { d.pPositions, d.pNormals, d.pTexcoords, data(_pExtra->Tangents) };
	const uint sStrides[] = { sizeof(*d.pPositions), sizeof(*d.pNormals), sizeof(*d.pTexcoords), sizeof(float4) };
	oGPUGetVertexSource(oCOUNTOF(sSemantics), sSemantics, sData, sStrides, _Element, _pElementData);
}

static bool oGPUReadVertexSource(int _Slot, int _NumVertices, surface::mapped_subresource& _Mapped, uint _NumElements, const oGPU_VERTEX_ELEMENT* _pElements, const std::function<void(const oGPU_VERTEX_ELEMENT& _Element, surface::const_mapped_subresource* _pElementData)>& _GetVertexSource)
{
	size_t offset = 0;
	size_t vertexStride = oGPUCalcVertexSize(_pElements, _NumElements, _Slot);

	for (uint i = 0; i < _NumElements; i++)
	{
		int msrI = _pElements[i].InputSlot;
		if (msrI >= 3) // TODO: Define 3
			return oErrorSetLast(std::errc::invalid_argument, "The %d%s element uses InputSlot %d, which is out of range for an oGfxMesh (must be 0 <= InputSlot <= 2)", ordinal(i), _pElements[i].InputSlot);
		if (msrI != _Slot)
			continue;

		surface::const_mapped_subresource Source;
		size_t ElStride = surface::element_size(_pElements[i].Format);

		// _ppElementData always overrides oGeometry, and then only look at 
		// oGeometry if index == 0 since oGeometry only has 1 channel for each
		// semantic it supports.
		_GetVertexSource(_pElements[i], &Source);

		void* pDestination = byte_add(_Mapped.data, offset);
		offset += ElStride;

		if (Source.data)
			memcpy2d(pDestination, vertexStride, Source.data, Source.row_pitch, ElStride, _NumVertices);
		else
		{
			#ifdef _DEBUG
				char buf[5];
				oTRACE("No data for %s (%d%s IAElement) for mesh", to_string(buf, _pElements[i].Semantic), i, ordinal(i));
			#endif
			memset2d(pDestination, vertexStride, 0, ElStride, _NumVertices);
		}
	}

	return true;
}

bool oGPUReadVertexSource(int _Slot, int _NumVertices, surface::mapped_subresource& _Mapped, uint _NumElements, const oGPU_VERTEX_ELEMENT* _pElements, const oGeometry::DESC& _Desc, oGeometry::CONST_MAPPED& _GeoMapped)
{
	return oGPUReadVertexSource(_Slot, _NumVertices, _Mapped, _NumElements, _pElements, std::bind(oGPUGetVertexSourceFromGeometry, _Desc, _GeoMapped, oBIND1, oBIND2));
}

bool oGPUReadVertexSource(int _Slot, int _NumVertices, surface::mapped_subresource& _Mapped, uint _NumElements, const oGPU_VERTEX_ELEMENT* _pElements, const threadsafe oOBJ* _pOBJ)
{
	oOBJExtraVertexData OBJExtra;
	return oGPUReadVertexSource(_Slot, _NumVertices, _Mapped, _NumElements, _pElements, std::bind(oGPUGetVertexSourceFromOBJ, _pOBJ, &OBJExtra, oBIND1, oBIND2));
}

struct oGPUUtilMeshImpl : oGPUUtilMesh
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGPUUtilMesh);

	oGPUUtilMeshImpl(oGPUDevice* _pDevice, const char* _Name, const oGPU_MESH_DESC& _Desc, uint _NumPrimitives, bool* _pSuccess)
		: Desc(_Desc)
		, NumPrimitives(_NumPrimitives)
	{
		*_pSuccess = false;

		// Make sure there's only one range
		if (_Desc.NumRanges != 1)
		{
			oErrorSetLast(std::errc::invalid_argument, "oGPUUtilMesh only supports one range");
			return;
		}

		// Make sure there's only one VB slot
		for (uint i = 0; i < _Desc.NumVertexElements; i++)
		{
			if (_Desc.VertexElements[i].InputSlot != 0)
			{
				oErrorSetLast(std::errc::invalid_argument, "oGPUUtilMesh only supports one range");
				return;
			}
		}

		oGPUBuffer::DESC d;
		oGPUGetIndexBufferDesc(_Desc.NumIndices, _Desc.NumVertices, &d);
		if (!_pDevice->CreateBuffer(_Name, d, &IB))
			return; // pass through error

		oGPUGetVertexBufferDesc(_Desc.NumVertices, _Desc.VertexElements.data(), _Desc.NumVertexElements, 0, &d);
		if (!_pDevice->CreateBuffer(_Name, d, &VB))
			return;
		*_pSuccess = true;
	}

	void GetDesc(oGPUUtilMesh::DESC* _pDesc) const override { *_pDesc = Desc; }
	uint GetNumPrimitives() const override { return NumPrimitives; }
	const oGPUBuffer* GetIndexBuffer() const override { return IB; }
	oGPUBuffer* GetIndexBuffer() override { return IB; }
	const oGPUBuffer* GetVertexBuffer() const override { return VB; }
	oGPUBuffer* GetVertexBuffer() override { return VB; }

	oGPU_MESH_DESC Desc;
	intrusive_ptr<oGPUBuffer> IB;
	intrusive_ptr<oGPUBuffer> VB;
	oRefCount RefCount;
	uint NumPrimitives;
};

bool oGPUUtilMeshCreate(oGPUDevice* _pDevice, const char* _Name, const oGPU_MESH_DESC& _Desc, uint _NumPrimitives, oGPUUtilMesh** _ppMesh)
{
	bool success = false;
	oCONSTRUCT(_ppMesh, oGPUUtilMeshImpl(_pDevice, _Name, _Desc, _NumPrimitives, &success));
	return success;
}

void oGPUUtilMeshDraw(oGPUCommandList* _pCommandList, const oGPUUtilMesh* _pMesh)
{
	const oGPUBuffer* VertexBuffer = _pMesh->GetVertexBuffer();
	_pCommandList->Draw(_pMesh->GetIndexBuffer(), 0, 1, &VertexBuffer, 0, _pMesh->GetNumPrimitives());
}

bool oGPUUtilMeshCreate(oGPUDevice* _pDevice, const char* _MeshName, const oGPU_VERTEX_ELEMENT* _pElements, uint _NumElements, const oGeometry* _pGeometry, oGPUUtilMesh** _ppMesh)
{
	oCONSTRUCT_CLEAR(_ppMesh);

	oGeometry::DESC GeoDesc;
	_pGeometry->GetDesc(&GeoDesc);

	oGPUUtilMesh::DESC d;
	d.LocalSpaceBounds = GeoDesc.Bounds;
	if (_NumElements > oGPU_MAX_NUM_VERTEX_ELEMENTS)
		return oErrorSetLast(std::errc::invalid_argument, "Too many vertex elements specified");

	std::copy(_pElements, _pElements + _NumElements, d.VertexElements.begin());
	d.NumRanges = GeoDesc.NumRanges;
	d.NumIndices = GeoDesc.NumIndices;
	d.NumVertices = GeoDesc.NumVertices;
	d.NumVertexElements = _NumElements;

	oGeometry::CONST_MAPPED GeoMapped;
	if (!_pGeometry->MapConst(&GeoMapped))
		return false; // pass through error

	finally GeoUnmap([&]{ _pGeometry->UnmapConst(); });

	intrusive_ptr<oGPUUtilMesh> Mesh;
	if (!oGPUUtilMeshCreate(_pDevice, _MeshName, d, GeoMapped.pRanges[0].NumPrimitives, &Mesh))
		return false; // pass through error

	intrusive_ptr<oGPUCommandList> ICL;
	_pDevice->GetImmediateCommandList(&ICL);

	// Ranges
	oASSERT(d.NumRanges == 1, "GPUUtilMesh only supports one range!");

	// Indices
	{
		surface::mapped_subresource msr;
		ICL->Reserve(Mesh->GetIndexBuffer(), 0, &msr);

		surface::const_mapped_subresource smsr;
		smsr.data = GeoMapped.pIndices;
		smsr.row_pitch = sizeof(uint);
		smsr.depth_pitch = smsr.row_pitch * d.NumIndices;
		oGPUCopyIndices(msr, smsr, d.NumIndices);

		ICL->Commit(Mesh->GetIndexBuffer(), 0, msr);
	}

	// Vertices
	{
		surface::mapped_subresource msr;
		ICL->Reserve(Mesh->GetVertexBuffer(), 0, &msr);
		bool success = oGPUReadVertexSource(0, d.NumVertices, msr, _NumElements, _pElements, GeoDesc, GeoMapped);
		ICL->Commit(Mesh->GetVertexBuffer(), 0, msr);
		if (!success)
			return oErrorSetLast(std::errc::invalid_argument, "Failed reading vertices");
	}

	Mesh->Reference();
	*_ppMesh = Mesh;
	return true;
}

bool oGPUUtilCreateFirstTriangle(oGPUDevice* _pDevice
	, const oGPU_VERTEX_ELEMENT* _pElements
	, uint _NumElements
	, oGPUUtilMesh** _ppFirstTriangle)
{
	oGPUUtilMesh::DESC md;
	md.NumIndices = 3;
	md.NumVertices = 3;
	md.NumRanges = 1;
	md.LocalSpaceBounds = oAABoxf(oAABoxf::min_max
		, float3(-0.8f, -0.7f, -0.01f)
		, float3(0.8f, 0.7f, 0.01f));
	md.NumVertexElements = _NumElements;

	std::copy(_pElements, _pElements + _NumElements, md.VertexElements.begin());
	intrusive_ptr<oGPUUtilMesh> Mesh;
	if (!oGPUUtilMeshCreate(_pDevice, "First Triangle", md, 1, &Mesh))
		return false; // pass through error

	intrusive_ptr<oGPUCommandList> ICL;
	_pDevice->GetImmediateCommandList(&ICL);

	surface::const_mapped_subresource msr;
	static const ushort Indices[] = { 0, 1, 2 };
	msr.data = (void*)Indices;
	msr.row_pitch = sizeof(ushort);
	msr.depth_pitch = sizeof(Indices);
	ICL->Commit(Mesh->GetIndexBuffer(), 0, msr);

	static const float3 Positions[] =
	{
		float3(-0.75f, -0.667f, 0.0f),
		float3(0.0f, 0.667f, 0.0f),
		float3(0.75f, -0.667f, 0.0f),
	};

	msr.data = (void*)Positions;
	msr.row_pitch = sizeof(float3);
	msr.depth_pitch = sizeof(Positions);
	ICL->Commit(Mesh->GetVertexBuffer(), 0, msr);

	Mesh->Reference();
	*_ppFirstTriangle = Mesh;
	return true;
}

bool oGPUUtilCreateFirstCube(oGPUDevice* _pDevice
	, const oGPU_VERTEX_ELEMENT* _pElements
	, uint _NumElements
	, oGPUUtilMesh** _ppFirstCube)
{
	oGeometry::LAYOUT layout;
	layout.Positions = true;
	layout.Texcoords = true;

	oGeometryFactory::BOX_DESC bd;
	bd.FaceType = oGeometry::FRONT_CCW;
	bd.Bounds = oAABoxf(oAABoxf::min_max, float3(-1.0f), float3(1.0f));
	bd.Divide = 1;
	bd.Color = White;
	bd.FlipTexcoordV = false;

	intrusive_ptr<oGeometryFactory> Factory;
	if (!oGeometryFactoryCreate(&Factory))
		return false;

	intrusive_ptr<oGeometry> geo;
	if (!Factory->Create(bd, layout, &geo))
		return false;

	return oGPUUtilMeshCreate(_pDevice, "First Cube", _pElements, _NumElements, geo, _ppFirstCube);
}
