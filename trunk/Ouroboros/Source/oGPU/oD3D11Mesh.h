/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#ifndef oD3D11Mesh_h
#define oD3D11Mesh_h

#include <oGPU/oGPU.h>
#include "oGPUCommon.h"
#include <oBasis/oAlgorithm.h>
#include <oPlatform/Windows/oD3D11.h>
#include <vector>

oDECLARE_GPURESOURCE_IMPLEMENTATION(oD3D11, Mesh, oGPU_MESH)
{
	oDEFINE_GPURESOURCE_INTERFACE();
	oDECLARE_GPURESOURCE_CTOR(oD3D11, Mesh);

	oMutex RangesMutex;
	std::vector<oGPU_RANGE> Ranges;
	oRef<oGPUBuffer> Indices;
	oRef<oGPUBuffer> Vertices[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	uint NumVertexBuffers;
	ID3D11Resource* GetSubresource(int _Subresource) threadsafe;
};

#endif
