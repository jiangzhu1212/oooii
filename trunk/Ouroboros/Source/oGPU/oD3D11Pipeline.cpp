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
#include <oGPU/shader.h>

oGPU_NAMESPACE_BEGIN

oDEFINE_DEVICE_MAKE(pipeline1)
oDEVICE_CHILD_CTOR(pipeline1)
	, InputTopology(from_primitive_type(_Info.primitive_type))
	, VertexElements(_Info.vertex_layout)
	, DebugName(_Info.debug_name)
{
	// Verify input against shaders
	if ((InputTopology == D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED || InputTopology < D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST) && (_Info.hs || _Info.ds))
		oTHROW_INVARG("%s inputs cannot have a hull or domain shader bound", as_string(_Info.primitive_type));

	switch (InputTopology)
	{
		case D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED:
		case D3D_PRIMITIVE_TOPOLOGY_LINELIST:
		case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP:
		case D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ:
		case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ:
		{
			// HS/DS handled above in if statement
			if (_Info.gs)
				oTHROW_INVARG("%s inputs cannot have a geometry shader bound", as_string(_Info.primitive_type));
			break;
		}
		
		default:
			break;
	}

	VertexLayout.initialize("layout", _Device.get(), VertexElements, _Info.vs);

	VertexShader.reset(make_vertex_shader(_Device.get(), _Info.vs, _Info.debug_name));
	HullShader.reset(make_hull_shader(_Device.get(), _Info.hs, _Info.debug_name));
	DomainShader.reset(make_domain_shader(_Device.get(), _Info.ds, _Info.debug_name));
	GeometryShader.reset(make_geometry_shader(_Device.get(), _Info.gs, _Info.debug_name));
	PixelShader.reset(make_pixel_shader(_Device.get(), _Info.ps, _Info.debug_name));
}

d3d11_pipeline1::~d3d11_pipeline1()
{
	VertexLayout.deinitialize();
}

pipeline1_info d3d11_pipeline1::get_info() const 
{
	pipeline1_info i;
	i.debug_name = DebugName;
	i.vertex_layout = VertexElements;
	i.primitive_type = to_primitive_type(InputTopology);
	return i;
}

oGPU_NAMESPACE_END
