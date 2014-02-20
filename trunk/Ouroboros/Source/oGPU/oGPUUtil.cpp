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

namespace ouro {
	namespace gpu {

void commit_buffer(oGPUCommandList* _pCommandList, oGPUBuffer* _pBuffer, const void* _pStruct, uint _SizeofStruct, uint _NumStructs)
{
	oCHECK(byte_aligned(_SizeofStruct, 16), "structs must be aligned to 16 bytes (%u bytes specified %.02f of alignment)", _SizeofStruct, _SizeofStruct / 16.0f);
	surface::mapped_subresource msr;
	msr.data = const_cast<void*>(_pStruct);
	msr.row_pitch = _SizeofStruct;
	msr.depth_pitch = _NumStructs * _SizeofStruct;
	_pCommandList->Commit(_pBuffer, 0, msr);
}

void commit_buffer(oGPUDevice* _pDevice, oGPUBuffer* _pBuffer, const void* _pStruct, uint _SizeofStruct, uint _NumStructs)
{
	oCHECK(byte_aligned(_SizeofStruct, 16), "structs must be aligned to 16 bytes (%u bytes specified %.02f of alignment)", _SizeofStruct, _SizeofStruct / 16.0f);
	intrusive_ptr<oGPUCommandList> ICL;
	_pDevice->GetImmediateCommandList(&ICL);
	surface::mapped_subresource msr;
	msr.data = const_cast<void*>(_pStruct);
	msr.row_pitch = _SizeofStruct;
	msr.depth_pitch = _NumStructs * _SizeofStruct;
	ICL->Commit(_pBuffer, 0, msr);
}

void commit_index_buffer(oGPUCommandList* _pCommandList
	, const surface::const_mapped_subresource& _MappedSubresource
	, oGPUBuffer* _pIndexBuffer)
{
	buffer_info i = _pIndexBuffer->get_info();

	if (surface::element_size(i.format) == _MappedSubresource.row_pitch)
		_pCommandList->Commit(_pIndexBuffer, 0, _MappedSubresource);
	else
	{
		surface::mapped_subresource MSRTemp;
		_pCommandList->Reserve(_pIndexBuffer, 0, &MSRTemp);
		mesh::copy_indices(MSRTemp.data, MSRTemp.row_pitch, _MappedSubresource.data, _MappedSubresource.row_pitch, i.array_size);
		_pCommandList->Commit(_pIndexBuffer, 0, MSRTemp);
	}
}

void commit_vertex_buffer(oGPUCommandList* _pCommandList, const mesh::layout::value& _Layout, const mesh::source& _Source, oGPUBuffer* _pVertexBuffer)
{
	buffer_info Info = _pVertexBuffer->get_info();
	surface::mapped_subresource Destination;
	_pCommandList->Reserve(_pVertexBuffer, 0, &Destination);
	finally UnmapMSRs([&] { if (Destination.data) _pCommandList->Commit(_pVertexBuffer, 0, Destination); });
	
	copy_vertices(Destination.data, _Layout, _Source, Info.array_size);
}

intrusive_ptr<oGPUBuffer> make_index_buffer(oGPUDevice* _pDevice, const char* _Name, uint _NumIndices, uint _NumVertices
	, const surface::const_mapped_subresource& _MappedSubresource)
{
	buffer_info i = make_index_buffer_info(_NumIndices, _NumVertices);
	intrusive_ptr<oGPUBuffer> IndexBuffer = _pDevice->make_buffer(_Name, i);
	if (_MappedSubresource.data)
	{
		intrusive_ptr<oGPUCommandList> ICL;
		_pDevice->GetImmediateCommandList(&ICL);
		commit_index_buffer(ICL, _MappedSubresource, IndexBuffer);
	}
	return IndexBuffer;
}

intrusive_ptr<oGPUBuffer> make_vertex_buffer(oGPUDevice* _pDevice, const char* _Name, const mesh::layout::value& _Layout
	, uint _NumVertices, const mesh::source& _Source)
{
	if (_Layout == mesh::layout::none)
		oTHROW_INVARG("no vertex elements specified");

	buffer_info i = make_vertex_buffer_info(_NumVertices, _Layout);

	intrusive_ptr<oGPUBuffer> VertexBuffer = _pDevice->make_buffer(_Name, i);

	if (_Source != mesh::source())
		commit_vertex_buffer(_pDevice, _Layout, _Source, VertexBuffer);

	return VertexBuffer;
}

intrusive_ptr<oGPUBuffer> make_vertex_buffer(oGPUDevice* _pDevice, const char* _Name, const mesh::layout::value& _Layout
	, const oGeometry::DESC& _GeoDesc, const oGeometry::CONST_MAPPED& _GeoMapped)
{
	mesh::source Source;
	Source.positionsf = _GeoMapped.pPositions; Source.positionf_pitch = sizeof(float3);
	Source.normalsf = _GeoMapped.pNormals; Source.normalf_pitch = sizeof(float3);
	Source.tangentsf = _GeoMapped.pTangents; Source.tangentf_pitch = sizeof(float4);
	Source.uvw0sf = _GeoMapped.pTexcoords; Source.uvw0f_pitch = sizeof(float3);
	Source.colors = _GeoMapped.pColors; Source.color_pitch = sizeof(color);

	return make_vertex_buffer(_pDevice, _Name, _Layout, _GeoDesc.NumVertices, Source);
}

intrusive_ptr<oGPUBuffer> make_readback_copy(oGPUBuffer* _pSource)
{
	intrusive_ptr<oGPUDevice> Device;
	_pSource->GetDevice(&Device);
	buffer_info i = _pSource->get_info();
	i.type = buffer_type::readback;
	sstring Name;
	snprintf(Name, "%s.Readback", _pSource->GetName());
	return Device->make_buffer(Name, i);
}

intrusive_ptr<oGPUTexture> make_readback_copy(oGPUTexture* _pSource)
{
	intrusive_ptr<oGPUDevice> Device;
	_pSource->GetDevice(&Device);
	texture_info i = _pSource->get_info();
	i.type = make_readback(i.type);
	sstring Name;
	snprintf(Name, "%s.Readback", _pSource->GetName());
	return Device->make_texture(Name, i);
}

uint read_back_counter(oGPUBuffer* _pUnorderedBuffer, oGPUBuffer* _pPreallocatedReadbackBuffer)
{
	buffer_info i = _pUnorderedBuffer->get_info();
	if (i.type != buffer_type::unordered_structured_append && i.type != buffer_type::unordered_structured_counter)
		oTHROW_INVARG("the specified buffer must be of type buffer_type::unordered_structured_append or buffer_type::unordered_structured_counter");

	intrusive_ptr<oGPUDevice> Device;
	_pUnorderedBuffer->GetDevice(&Device);

	intrusive_ptr<oGPUBuffer> Counter = _pPreallocatedReadbackBuffer;
	if (!Counter)
	{
		sstring Name;
		snprintf(Name, "%s.Readback", _pUnorderedBuffer->GetName());

		buffer_info rb;
		rb.type = buffer_type::readback;
		rb.struct_byte_size = sizeof(uint);

		Counter = Device->make_buffer(Name, rb);
	}

	intrusive_ptr<oGPUCommandList> ICL;
	Device->GetImmediateCommandList(&ICL);
	ICL->CopyCounter(Counter, 0, _pUnorderedBuffer);

	surface::mapped_subresource msr;
	if (!Device->MapRead(Counter, 0, &msr, true))
		return ouro::invalid; // pass through error
	uint c = *(uint*)msr.data;
	Device->UnmapRead(Counter, 0);
	return c;
}

bool read(oGPUResource* _pSourceResource, int _Subresource, surface::mapped_subresource& _Destination, bool _FlipVertically)
{
	intrusive_ptr<oGPUDevice> Device;
	_pSourceResource->GetDevice(&Device);

	switch (_pSourceResource->GetType())
	{
		case resource_type::buffer:
		{
			oGPUBuffer::DESC i;
			static_cast<oGPUBuffer*>(_pSourceResource)->GetDesc(&i);
			if (i.type != buffer_type::readback)
				oTHROW_INVARG("The specified resource %p %s must be a readback type.", _pSourceResource, _pSourceResource->GetName());
			break;
		}

		case resource_type::texture:
		{
			oGPUTexture::DESC i;
			static_cast<oGPUTexture*>(_pSourceResource)->GetDesc(&i);
			if (!is_readback(i.type))
				oTHROW_INVARG("The specified resource %p %s must be a readback type.", _pSourceResource, _pSourceResource->GetName());
			break;
		}

		default:
			break;
	}

	surface::mapped_subresource source;
	if (Device->MapRead(_pSourceResource, _Subresource, &source))
	{
		int2 ByteDimensions = _pSourceResource->GetByteDimensions(_Subresource);
		memcpy2d(_Destination.data, _Destination.row_pitch, source.data, source.row_pitch, ByteDimensions.x, ByteDimensions.y, _FlipVertically);
		Device->UnmapRead(_pSourceResource, _Subresource);
		return true;
	}
	return false;
}

intrusive_ptr<oGPUTexture> make_texture(oGPUDevice* _pDevice, const char* _Name, const surface::buffer* const* _ppSourceImages, uint _NumImages, texture_type::value _Type)
{
	if (!_NumImages)
		oTHROW_INVARG("Need at least one source image");

	surface::info si = _ppSourceImages[0]->get_info();
	si.array_size = _NumImages;

	intrusive_ptr<oGPUTexture> Texture;
	texture_info i;
	i.format = si.format;
	i.type = _Type;
	i.dimensions = int3(si.dimensions.xy(), is_3d(_Type) ? _NumImages : 1);
	i.array_size = is_array(_Type) || is_cube(_Type) ? static_cast<ushort>(_NumImages) : 0;

	switch (get_type(_Type))
	{
		case texture_type::default_1d:
			if (si.dimensions.y != 1)
				oTHROW_INVARG("1D textures cannot have height");
			break;

		case texture_type::default_cube:
			if (((_NumImages) % 6) != 0)
				oTHROW_INVARG("Cube maps must be specified in sets of 6");
			break;

		default:
			break;
	}

	Texture = _pDevice->make_texture(_Name, i);

	intrusive_ptr<oGPUCommandList> ICL;
	_pDevice->GetImmediateCommandList(&ICL);

	const int NumMips = surface::num_mips(is_mipped(_Type), i.dimensions);
	surface::buffer::make_type MakeType = is_3d(_Type) 
		? (NumMips ? surface::buffer::mips3d : surface::buffer::image3d)
		: (NumMips ? surface::buffer::mips_array : surface::buffer::image_array);

	auto src = surface::buffer::make(_ppSourceImages, _NumImages, MakeType);
	auto src_si = src->get_info();
	const int nSubresources = surface::num_subresources(src_si);

	if (is_3d(_Type))
	{
		for (int subresource = 0; subresource < nSubresources; subresource++)
		{
			auto sri = surface::subresource(src_si, subresource);
			surface::box region;
			region.right = sri.dimensions.x;
			region.bottom = sri.dimensions.y;
			surface::shared_lock lock(src, subresource);
			for (int slice = 0; slice < sri.dimensions.z; slice++)
			{
				region.front = slice;
				region.back = slice + 1;
				ICL->Commit(Texture, subresource, lock.mapped, region);
			}
		}
	}

	else
	{
		for (int subresource = 0; subresource < nSubresources; subresource++)
		{
			surface::shared_lock lock(src, subresource);
			ICL->Commit(Texture, subresource, lock.mapped);
		}
	}

	return Texture;
}

std::shared_ptr<surface::buffer> copy_to_surface_buffer(oGPUTexture* _pSource, int _Subresource)
{
	intrusive_ptr<oGPUDevice> Device;
	_pSource->GetDevice(&Device);
	intrusive_ptr<oGPUCommandList> ICL;
	Device->GetImmediateCommandList(&ICL);

	intrusive_ptr<oGPUTexture> readback;
	texture_info i = _pSource->get_info();
	if (is_readback(i.type))
		readback = _pSource;
	else
	{
		readback = make_readback_copy(_pSource);
		i = readback->get_info();
		ICL->Copy(readback, _pSource);
	}

	surface::info si;
	si.dimensions = i.dimensions;
	si.format = i.format;
	si.layout = surface::tight;
	si.array_size = i.array_size;

	std::shared_ptr<surface::buffer> b;

	if (_Subresource == invalid)
	{
		b = surface::buffer::make(si);
		
		int nSubresources = surface::num_subresources(si);
		if (is_3d(i.type))
		{
			for (int subresource = 0; subresource < nSubresources; subresource++)
			{
				auto sri = surface::subresource(si, subresource);
				surface::box region;
				region.right = sri.dimensions.x;
				region.bottom = sri.dimensions.y;

				surface::mapped_subresource mapped;
				oCHECK(Device->MapRead(readback, subresource, &mapped, true), "MapRead failed");

				for (int slice = 0; slice < sri.dimensions.z; slice++)
				{
					region.front = slice;
					region.back = slice + 1;
					b->update_subresource(subresource, region, mapped);
				}

				Device->UnmapRead(readback, subresource);
			}
		}

		else
		{
			for (int subresource = 0; subresource < nSubresources; subresource++)
			{
				auto sri = surface::subresource(si, subresource);

				surface::mapped_subresource mapped;
				oCHECK(Device->MapRead(readback, subresource, &mapped, true), "MapRead failed");
				b->update_subresource(subresource, mapped);
				Device->UnmapRead(readback, subresource);
			}
		}
	}

	else
	{
		surface::subresource_info sri = surface::subresource(si, _Subresource);
		si.dimensions = sri.dimensions;
		si.format = sri.format;
		si.array_size = 0;
		si.layout = surface::image;
		b = surface::buffer::make(si);

		surface::mapped_subresource mapped;
		oCHECK(Device->MapRead(readback, _Subresource, &mapped, true), "MapRead failed");
		b->update_subresource(_Subresource, mapped);
		Device->UnmapRead(readback, _Subresource);
	}

	return b;
}


	} // namespace gpu
} // namespace ouro


#if 0

#include <oBasis/oMeshUtil.h>
#include <oBasis/oRefCount.h>

using namespace ouro;

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
	return oGPUReadVertexSource(_Slot, _NumVertices, _Mapped, _NumElements, _pElements, std::bind(oGPUGetVertexSourceFromGeometry, _Desc, _GeoMapped, std::placeholders::_1, std::placeholders::_2));
}

bool oGPUReadVertexSource(int _Slot, int _NumVertices, surface::mapped_subresource& _Mapped, uint _NumElements, const oGPU_VERTEX_ELEMENT* _pElements, const threadsafe oOBJ* _pOBJ)
{
	oOBJExtraVertexData OBJExtra;
	return oGPUReadVertexSource(_Slot, _NumVertices, _Mapped, _NumElements, _pElements, std::bind(oGPUGetVertexSourceFromOBJ, _pOBJ, &OBJExtra, std::placeholders::_1, std::placeholders::_2));
}

struct ouro::gpu::util_meshImpl : ouro::gpu::util_mesh
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(ouro::gpu::util_mesh);

	ouro::gpu::util_meshImpl(oGPUDevice* _pDevice, const char* _Name, const oGPU_MESH_DESC& _Desc, uint _NumPrimitives, bool* _pSuccess)
		: Desc(_Desc)
		, NumPrimitives(_NumPrimitives)
	{
		*_pSuccess = false;

		// Make sure there's only one range
		if (_Desc.NumRanges != 1)
		{
			oErrorSetLast(std::errc::invalid_argument, "ouro::gpu::util_mesh only supports one range");
			return;
		}

		// Make sure there's only one VB slot
		for (uint i = 0; i < _Desc.NumVertexElements; i++)
		{
			if (_Desc.VertexElements[i].InputSlot != 0)
			{
				oErrorSetLast(std::errc::invalid_argument, "ouro::gpu::util_mesh only supports one range");
				return;
			}
		}

		oGPUBuffer::DESC i = oGPUGetIndexBufferDesc(_Desc.NumIndices, _Desc.NumVertices);
		if (!_pDevice->CreateBuffer(_Name, i, &IB))
			return; // pass through error

		i = oGPUGetVertexBufferDesc(_Desc.NumVertices, _Desc.VertexElements.data(), _Desc.NumVertexElements, 0);
		if (!_pDevice->CreateBuffer(_Name, i, &VB))
			return;
		*_pSuccess = true;
	}

	void GetDesc(ouro::gpu::util_mesh::DESC* _pDesc) const override { *_pDesc = Desc; }
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

bool ouro::gpu::util_meshCreate(oGPUDevice* _pDevice, const char* _Name, const oGPU_MESH_DESC& _Desc, uint _NumPrimitives, ouro::gpu::util_mesh** _ppMesh)
{
	bool success = false;
	oCONSTRUCT(_ppMesh, ouro::gpu::util_meshImpl(_pDevice, _Name, _Desc, _NumPrimitives, &success));
	return success;
}

void ouro::gpu::util_meshDraw(oGPUCommandList* _pCommandList, const ouro::gpu::util_mesh* _pMesh)
{
	const oGPUBuffer* VertexBuffer = _pMesh->GetVertexBuffer();
	_pCommandList->Draw(_pMesh->GetIndexBuffer(), 0, 1, &VertexBuffer, 0, _pMesh->GetNumPrimitives());
}

bool ouro::gpu::util_meshCreate(oGPUDevice* _pDevice, const char* _MeshName, const oGPU_VERTEX_ELEMENT* _pElements, uint _NumElements, const oGeometry* _pGeometry, ouro::gpu::util_mesh** _ppMesh)
{
	oCONSTRUCT_CLEAR(_ppMesh);

	oGeometry::DESC GeoDesc;
	_pGeometry->GetDesc(&GeoDesc);

	ouro::gpu::util_mesh::DESC d;
	d.LocalSpaceBounds = GeoDesc.Bounds;
	if (_NumElements > ouro::gpu::max_num_vertex_elements)
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

	std::shared_ptr<ouro::gpu::util_mesh> Mesh;
	if (!ouro::gpu::util_meshCreate(_pDevice, _MeshName, d, GeoMapped.pRanges[0].num_primitives, &Mesh))
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
	, ouro::gpu::util_mesh** _ppFirstTriangle)
{
	ouro::gpu::util_mesh::DESC md;
	md.NumIndices = 3;
	md.NumVertices = 3;
	md.NumRanges = 1;
	md.LocalSpaceBounds = aaboxf(aaboxf::min_max
		, float3(-0.8f, -0.7f, -0.01f)
		, float3(0.8f, 0.7f, 0.01f));
	md.NumVertexElements = _NumElements;

	std::copy(_pElements, _pElements + _NumElements, md.VertexElements.begin());
	std::shared_ptr<ouro::gpu::util_mesh> Mesh;
	if (!ouro::gpu::util_meshCreate(_pDevice, "First Triangle", md, 1, &Mesh))
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
	, ouro::gpu::util_mesh** _ppFirstCube)
{
	oGeometry::LAYOUT layout;
	layout.Positions = true;
	layout.Texcoords = true;

	oGeometryFactory::BOX_DESC bd;
	bd.FaceType = oGeometry::FRONT_CCW;
	bd.Bounds = aaboxf(aaboxf::min_max, float3(-1.0f), float3(1.0f));
	bd.Divide = 1;
	bd.Color = White;
	bd.FlipTexcoordV = false;

	intrusive_ptr<oGeometryFactory> Factory;
	if (!oGeometryFactoryCreate(&Factory))
		return false;

	intrusive_ptr<oGeometry> geo;
	if (!Factory->Create(bd, layout, &geo))
		return false;

	return ouro::gpu::util_meshCreate(_pDevice, "First Cube", _pElements, _NumElements, geo, _ppFirstCube);
}

#endif