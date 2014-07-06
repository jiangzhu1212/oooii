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
#pragma once
#ifndef oGPU_pipeline_h
#define oGPU_pipeline_h

#include "oGPUCommon.h"
#include <oGPU/vertex_layout.h>

oGPU_NAMESPACE_BEGIN

oDEVICE_CHILD_CLASS(pipeline1)
{
	oDEVICE_CHILD_DECLARATION(pipeline1)
	~d3d11_pipeline1();
	pipeline1_info get_info() const override;
	vertex_layout VertexLayout;
	vertex_shader VertexShader;
	hull_shader HullShader;
	domain_shader DomainShader;
	geometry_shader GeometryShader;
	pixel_shader PixelShader;
	D3D_PRIMITIVE_TOPOLOGY InputTopology;
	mesh::element_array VertexElements;
	sstring DebugName;
};

oGPU_NAMESPACE_END

#endif
