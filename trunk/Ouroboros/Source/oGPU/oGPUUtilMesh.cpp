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
#include <oGPU/oGPUUtilMesh.h>
#include <oGPU/oGPUUtil.h>

namespace ouro {
	namespace gpu {

class util_mesh_impl : public util_mesh
{
public:
	util_mesh_impl(oGPUDevice* _pDevice, const char* _Name, const mesh::info& _Info)
		: MeshInfo(_Info)
	{
		if (_Info.num_ranges != 1)
			oTHROW_INVARG("mesh range must be 1");

		IB = make_index_buffer(_pDevice, _Name, MeshInfo.num_indices, MeshInfo.num_vertices);

		for (size_t i = 0; i < MeshInfo.vertex_layouts.size(); i++)
			if (MeshInfo.vertex_layouts[i] != mesh::layout::none)
				VBs[i] = make_vertex_buffer(_pDevice, _Name, MeshInfo.vertex_layouts[i], MeshInfo.num_vertices);
	}

	util_mesh_impl(oGPUDevice* _pDevice, const char* _Name, const mesh::layout_array& _VertexLayouts, const oGeometry* _pGeometry)
	{
		oGeometry::DESC gd;
		_pGeometry->GetDesc(&gd);

		MeshInfo.local_space_bound = aaboxf(aaboxf::min_max, gd.Bounds.Min, gd.Bounds.Max);
		MeshInfo.num_indices = gd.NumIndices;
		MeshInfo.num_vertices = gd.NumVertices;
		MeshInfo.vertex_layouts[0] = gd.Layout.AsVertexLayout();
		MeshInfo.primitive_type = gd.PrimitiveType;
		MeshInfo.face_type = gd.FaceType;
		MeshInfo.num_ranges = 1;
		MeshInfo.vertex_scale_shift = 0;

		buffer_info ii = make_index_buffer_info(gd.NumIndices, gd.NumVertices);

		oGeometry::CONST_MAPPED mapped;
		_pGeometry->MapConst(&mapped);
		finally UnMap([&] { _pGeometry->UnmapConst(); });

		surface::const_mapped_subresource msrIndices;
		msrIndices.data = mapped.pIndices;
		msrIndices.row_pitch = ii.struct_byte_size;
		IB = make_index_buffer(_pDevice, _Name, gd.NumIndices, gd.NumVertices, msrIndices);

		for (size_t i = 0; i < MeshInfo.vertex_layouts.size(); i++)
			if (MeshInfo.vertex_layouts[i] != mesh::layout::none)
				VBs[i] = make_vertex_buffer(_pDevice, _Name, MeshInfo.vertex_layouts[i], gd, mapped);
	}

	mesh::info get_info() const override { return MeshInfo; }
	const oGPUBuffer* index_buffer() const override { return IB; }
	oGPUBuffer* index_buffer() override { return IB; }
	const oGPUBuffer* vertex_buffer(uint _Index) const override { return VBs[_Index]; }
	oGPUBuffer* vertex_buffer(uint _Index) override { return VBs[_Index]; }
	void vertex_buffers(const oGPUBuffer* _Buffers[mesh::usage::count]) override
	{
		oFORI(i, _Buffers)
			_Buffers[i] = VBs[i].c_ptr();
	}

	void draw(oGPUCommandList* _pCommandList)
	{
		_pCommandList->Draw(IB, 0, mesh::usage::count, (const oGPUBuffer* const *)VBs.data(), 0, mesh::num_primitives(MeshInfo.primitive_type, MeshInfo.num_indices, MeshInfo.num_vertices));
	}

	mesh::info MeshInfo;
	intrusive_ptr<oGPUBuffer> IB;
	std::array<intrusive_ptr<oGPUBuffer>, mesh::usage::count> VBs;
	uint NumPrimitives;
};

std::shared_ptr<util_mesh> util_mesh::make(oGPUDevice* _pDevice, const char* _Name, const mesh::info& _Info)
{
	return std::make_shared<util_mesh_impl>(_pDevice, _Name, _Info);
}

std::shared_ptr<util_mesh> util_mesh::make(oGPUDevice* _pDevice, const char* _Name
		, const mesh::layout_array& _VertexLayouts, const oGeometry* _pGeometry)
{
	return std::make_shared<util_mesh_impl>(_pDevice, _Name, _VertexLayouts, _pGeometry);
}

std::shared_ptr<util_mesh> make_first_triangle(oGPUDevice* _pDevice)
{
	mesh::info mi;
	mi.local_space_bound = aaboxf(aaboxf::min_max, float3(-0.8f, -0.7f, -0.01f), float3(0.8f, 0.7f, 0.01f));
	mi.num_indices = 3;
	mi.num_vertices = 3;
	mi.num_ranges = 1;
	mi.vertex_layouts[0] = mesh::layout::pos;
	mi.face_type = mesh::face_type::front_ccw;
	mi.primitive_type = mesh::primitive_type::triangles;
	mi.vertex_scale_shift = 0;

	std::shared_ptr<util_mesh> m;
	m = util_mesh::make(_pDevice, "First Triangle", mi);

	intrusive_ptr<oGPUCommandList> ICL;
	_pDevice->GetImmediateCommandList(&ICL);

	surface::const_mapped_subresource msr;
	static const ushort Indices[] = { 0, 1, 2 };
	msr.data = (void*)Indices;
	msr.row_pitch = sizeof(ushort);
	msr.depth_pitch = sizeof(Indices);
	ICL->Commit(m->index_buffer(), 0, msr);

	static const float3 Positions[] = 
	{
		float3(-0.75f, -0.667f, 0.0f),
		float3(0.0f, 0.667f, 0.0f),
		float3(0.75f, -0.667f, 0.0f),
	};

	msr.data = Positions;
	msr.row_pitch = sizeof(float3);
	msr.depth_pitch = sizeof(Positions);
	ICL->Commit(m->vertex_buffer(0), 0, msr);

	return m;
}

std::shared_ptr<util_mesh> make_first_cube(oGPUDevice* _pDevice)
{
	oGeometry::LAYOUT layout;
	layout.Positions = true;
	layout.Texcoords = true;

	oGeometryFactory::BOX_DESC bd;
	bd.FaceType = mesh::face_type::front_ccw;
	bd.Bounds = aaboxf(aaboxf::min_max, float3(-1.0f), float3(1.0f));
	bd.Divide = 1;
	bd.Color = white;
	bd.FlipTexcoordV = false;

	intrusive_ptr<oGeometryFactory> Factory;
	oCHECK0(oGeometryFactoryCreate(&Factory));

	intrusive_ptr<oGeometry> geo;
	oCHECK0(Factory->Create(bd, layout, &geo));

	mesh::layout_array Layouts(mesh::layout::pos_uv0);
	return util_mesh::make(_pDevice, "First Cube", Layouts, geo);
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

#endif