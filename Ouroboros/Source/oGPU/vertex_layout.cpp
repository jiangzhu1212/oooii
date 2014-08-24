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
#include <oGPU/vertex_layout.h>
#include <oCore/windows/win_util.h>
#include "d3d_debug.h"
#include "d3d_prim.h"
#include "d3d_util.h"
#include "dxgi_util.h"
#include <string>

using namespace ouro::gpu::d3d;

namespace ouro { namespace gpu {

Device* get_device(device& dev);
DeviceContext* get_dc(command_list& cl);

static const char* from_semantic(const surface::semantic& s)
{
	switch (s)
	{
		case surface::semantic::vertex_position: return "POSITION";
		case surface::semantic::vertex_normal: return "NORMAL";
		case surface::semantic::vertex_tangent: return "TANGENT";
		case surface::semantic::vertex_texcoord: return "TEXCOORD";
		case surface::semantic::vertex_color: return "COLOR";
		default: break;
	};
	return "?";
}

static D3D11_INPUT_ELEMENT_DESC from_element(const mesh::element& element)
{
	D3D11_INPUT_ELEMENT_DESC d;
	d.SemanticName = from_semantic(element.semantic());
	d.SemanticIndex = element.index();
	d.Format = dxgi::from_surface_format(element.format());
	d.InputSlot = element.slot();
	d.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	d.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	d.InstanceDataStepRate = 0;
	return d;
}

void vertex_layout::initialize(const char* name, device& dev, const mesh::element_array& elements, const void* vs_bytecode)
{
	D3D11_INPUT_ELEMENT_DESC Elements[max_vertex_elements];
	uint n = 0;
	for (const mesh::element& e : elements)
	{
		if (e.semantic() == surface::semantic::unknown || e.format() == surface::format::unknown)
			continue;
		Elements[n++] = from_element(e);
	}
	oV(get_device(dev)->CreateInputLayout(Elements, n, vs_bytecode, bytecode_size(vs_bytecode), (InputLayout**)&layout));
	debug_name((InputLayout*)layout, name);
}

void vertex_layout::deinitialize()
{
	if (layout)
		((DeviceChild*)layout)->Release();
	layout = nullptr;
}

void vertex_layout::set(command_list& cl, const mesh::primitive_type::value& prim_type) const
{
	DeviceContext* dc = get_dc(cl);
	dc->IASetPrimitiveTopology( from_primitive_type(prim_type) );
	dc->IASetInputLayout((InputLayout*)layout);
}

}}
