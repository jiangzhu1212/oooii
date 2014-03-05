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
// Simplify the complex state definition of D3D to be accessed by gpu enums.
#pragma once
#ifndef oGPU_d3d11_layout_h
#define oGPU_d3d11_layout_h

#include <oGPU/oGPU.h>
#include <d3d11.h>

namespace ouro {
	namespace gpu {
		namespace d3d11 {

const char* get_semantic(mesh::semantic::value& _Semantic);

// ouro::gpu assumes vertex buffers will be created by usage, so a single mesh might have 
// several vertex buffers. When this is set up D3D requires one input layout to describe
// it all, so here create an ID3D11InputLayout based on the vertex layouts of each usage
// type and the byte code for the vertex shader that will use it.
intrusive_ptr<ID3D11InputLayout> make_input_layout(ID3D11Device* _pDevice
	, const void* _pVertexShaderByteCode, const mesh::layout_array& _VertexLayouts);

		} // namespace d3d11
	} // namespace gpu
} // namespace ouro

#endif
