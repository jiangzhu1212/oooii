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
	util_mesh_impl(device* _pDevice, const char* _Name, const mesh::info& _Info)
		: MeshInfo(_Info)
	{
		if (_Info.num_ranges != 1)
			oTHROW_INVARG("mesh range must be 1");

		IB = make_index_buffer(_pDevice, _Name, MeshInfo.num_indices, MeshInfo.num_vertices);

		for (size_t i = 0; i < MeshInfo.vertex_layouts.size(); i++)
			if (MeshInfo.vertex_layouts[i] != mesh::layout::none)
				VBs[i] = make_vertex_buffer(_pDevice, _Name, MeshInfo.vertex_layouts[i], MeshInfo.num_vertices);
	}

	util_mesh_impl(device* _pDevice, const char* _Name, const mesh::layout_array& _VertexLayouts, const oGeometry* _pGeometry)
	{
		MeshInfo = _pGeometry->get_info();

		buffer_info ii = make_index_buffer_info(MeshInfo.num_indices, MeshInfo.num_vertices);

		mesh::source source = _pGeometry->get_source();
		surface::const_mapped_subresource msrIndices;
		msrIndices.data = source.indicesi;
		msrIndices.row_pitch = source.indexi_pitch;
		IB = make_index_buffer(_pDevice, _Name, MeshInfo.num_indices, MeshInfo.num_vertices, msrIndices);

		for (size_t i = 0; i < _VertexLayouts.size(); i++)
			if (_VertexLayouts[i] != mesh::layout::none)
				VBs[i] = make_vertex_buffer(_pDevice, _Name, _VertexLayouts[i], MeshInfo.num_indices, source);
	}

	mesh::info get_info() const override { return MeshInfo; }
	const buffer* index_buffer() const override { return IB.get(); }
	buffer* index_buffer() override { return IB.get(); }
	const buffer* vertex_buffer(uint _Index) const override { return VBs[_Index].get(); }
	buffer* vertex_buffer(uint _Index) override { return VBs[_Index].get(); }
	void vertex_buffers(const buffer* _Buffers[mesh::usage::count]) override
	{
		oFORI(i, _Buffers)
			_Buffers[i] = VBs[i].get();
	}

	void draw(command_list* _pCommandList)
	{
		_pCommandList->draw(IB, 0, mesh::usage::count, VBs.data(), 0, mesh::num_primitives(MeshInfo.primitive_type, MeshInfo.num_indices, MeshInfo.num_vertices));
	}

	mesh::info MeshInfo;
	std::shared_ptr<buffer> IB;
	std::array<std::shared_ptr<buffer>, mesh::usage::count> VBs;
	uint NumPrimitives;
};

std::shared_ptr<util_mesh> util_mesh::make(device* _pDevice, const char* _Name, const mesh::info& _Info)
{
	return std::make_shared<util_mesh_impl>(_pDevice, _Name, _Info);
}

std::shared_ptr<util_mesh> util_mesh::make(device* _pDevice, const char* _Name
		, const mesh::layout_array& _VertexLayouts, const oGeometry* _pGeometry)
{
	return std::make_shared<util_mesh_impl>(_pDevice, _Name, _VertexLayouts, _pGeometry);
}

std::shared_ptr<util_mesh> make_first_triangle(device* _pDevice)
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

	surface::const_mapped_subresource msr;
	static const ushort Indices[] = { 0, 1, 2 };
	msr.data = (void*)Indices;
	msr.row_pitch = sizeof(ushort);
	msr.depth_pitch = sizeof(Indices);
	_pDevice->immediate()->commit(m->index_buffer(), 0, msr);

	static const float3 Positions[] = 
	{
		float3(-0.75f, -0.667f, 0.0f),
		float3(0.0f, 0.667f, 0.0f),
		float3(0.75f, -0.667f, 0.0f),
	};

	msr.data = Positions;
	msr.row_pitch = sizeof(float3);
	msr.depth_pitch = sizeof(Positions);
	_pDevice->immediate()->commit(m->vertex_buffer(0), 0, msr);

	return m;
}

std::shared_ptr<util_mesh> make_first_cube(device* _pDevice)
{
	mesh::layout::value Layout = mesh::layout::pos_uv0;

	oGeometryFactory::BOX_DESC bd;
	bd.FaceType = mesh::face_type::front_ccw;
	bd.Bounds = aaboxf(aaboxf::min_max, float3(-1.0f), float3(1.0f));
	bd.Divide = 1;
	bd.Color = white;
	bd.FlipTexcoordV = false;

	intrusive_ptr<oGeometryFactory> Factory;
	oCHECK0(oGeometryFactoryCreate(&Factory));

	intrusive_ptr<oGeometry> geo;
	oCHECK0(Factory->Create(bd, Layout, &geo));

	mesh::layout_array Layouts(mesh::layout::pos_uv0);
	return util_mesh::make(_pDevice, "First Cube", Layouts, geo);
}

	} // namespace gpu
} // namespace ouro
