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
#ifndef oGPU_sampler_h
#define oGPU_sampler_h

#include <oBase/macros.h>
#include <oMesh/mesh.h>

namespace ouro {
	namespace gpu {

namespace sampler_state
{ oDECLARE_SMALL_ENUM(value, uchar) {

  point_clamp,
  point_wrap,
  linear_clamp,
  linear_wrap,
  aniso_clamp,
  aniso_wrap,

	count,

};}

oDECLARE_HANDLE(sampler);

class device;
class command_list;

sampler make_sampler_state(device* _pDevice, const sampler_state::value& _Sampler);

void unmake_vertex_layout(sampler _SamplerState);

void bind_samplers(command_list* _pCommandList, uint _StartSlot, const sampler* _pSamplerStates, uint _NumSamplerStates);

	} // namespace gpu
} // namespace ouro

#endif
