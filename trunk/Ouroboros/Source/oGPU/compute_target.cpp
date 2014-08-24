/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#include <oGPU/compute_target.h>
#include <oCore/windows/win_error.h>
#include <oCore/windows/win_util.h>
#include "d3d_debug.h"
#include "d3d_util.h"
#include "dxgi_util.h"

using namespace ouro::gpu::d3d;

namespace ouro { namespace gpu {

Device* get_device(device& dev);
DeviceContext* get_dc(command_list& cl);

void compute_target::deinitialize()
{
	oSAFE_RELEASEV(rw);
	oSAFE_RELEASEV(ro);
}

void compute_target::cleari(command_list& cl, const uint4& clear_value)
{
	// @tony: note it may not be possible to clear an append/counter target, so if that ends up being true, create another uav just for this.
	get_dc(cl)->ClearUnorderedAccessViewUint((UnorderedAccessView*)rw, (const uint*)&clear_value);
}

void compute_target::clearf(command_list& cl, const float4& clear_value)
{
	// @tony: note it may not be possible to clear an append/counter target, so if that ends up being true, create another uav just for this.
	get_dc(cl)->ClearUnorderedAccessViewFloat((UnorderedAccessView*)rw, (const float*)&clear_value);
}

void compute_target::set_draw_target(command_list& cl, uint slot, uint initial_count) const
{
	DeviceContext* dc = get_dc(cl);
	unset_all_dispatch_targets(dc);
	dc->OMSetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, slot, 1, (UnorderedAccessView**)&rw, &initial_count);

	// any pixel shader operation will quitely noop if a viewport isn't specified
	// so set a 1x1 if none other is set.
	uint nViewports = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	D3D11_VIEWPORT Viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	dc->RSGetViewports(&nViewports, Viewports);
	if (!nViewports)
	{
		D3D11_VIEWPORT& v = Viewports[0];
		v.TopLeftX = 0.0f;
		v.TopLeftY = 0.0f;
		v.Width = 1.0f;
		v.Height = 1.0f;
		v.MinDepth = 0.0f;
		v.MaxDepth = 1.0f;
		dc->RSSetViewports(1, Viewports);
	}
}

void compute_target::set_dispatch_target(command_list& cl, uint slot, uint initial_count) const
{
	DeviceContext* dc = get_dc(cl);
	unset_all_draw_targets(dc);
	dc->CSSetUnorderedAccessViews(slot, 1, (UnorderedAccessView**)&rw, &initial_count);
}

}}
