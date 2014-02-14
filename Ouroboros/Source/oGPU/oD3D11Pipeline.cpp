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
#include "oD3D11Pipeline.h"
#include "oD3D11Device.h"
#include "dxgi_util.h"
#include "d3d11_layout.h"

using namespace ouro;
using namespace ouro::d3d11;
using namespace ouro::gpu;

oDEFINE_GPUDEVICE_CREATE(oD3D11, Pipeline);
oBEGIN_DEFINE_GPUDEVICECHILD_CTOR(oD3D11, Pipeline)
	, InputTopology(from_primitive_type(_Desc.primitive_type))
	, DebugName(_Desc.debug_name)
	, VertexLayouts(_Desc.vertex_layouts)
{
	*_pSuccess = false;

	// Verify input against shaders
	if ((InputTopology == D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED || InputTopology < D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST) && (_Desc.hs || _Desc.ds))
		oTHROW_INVARG("%s inputs cannot have a hull or domain shader bound", as_string(_Desc.primitive_type));

	switch (InputTopology)
	{
		case D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED:
		case D3D_PRIMITIVE_TOPOLOGY_LINELIST:
		case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP:
		case D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ:
		case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ:
		{
			// HS/DS handled above in if statement
			if (_Desc.gs)
				oTHROW_INVARG("%s inputs cannot have a geometry shader bound", as_string(_Desc.primitive_type));
			break;
		}
		
		default:
			break;
	}

	oD3D11DEVICE();

	InputLayout = make_input_layout(D3DDevice, _Desc.vs, VertexLayouts);
	VertexShader = make_vertex_shader(D3DDevice, _Desc.vs, _Desc.debug_name);
	HullShader = make_hull_shader(D3DDevice, _Desc.hs, _Desc.debug_name);
	DomainShader = make_domain_shader(D3DDevice, _Desc.ds, _Desc.debug_name);
	GeometryShader = make_geometry_shader(D3DDevice, _Desc.gs, _Desc.debug_name);
	PixelShader = make_pixel_shader(D3DDevice, _Desc.ps, _Desc.debug_name);

	*_pSuccess = true;
}

oD3D11Pipeline::~oD3D11Pipeline()
{
}

void oD3D11Pipeline::GetDesc(DESC* _pDesc) const threadsafe
{
	memset(_pDesc, 0, sizeof(DESC));
	_pDesc->debug_name = DebugName;
	_pDesc->vertex_layouts = oThreadsafe(this)->VertexLayouts;
	_pDesc->primitive_type = to_primitive_type(InputTopology);
}
