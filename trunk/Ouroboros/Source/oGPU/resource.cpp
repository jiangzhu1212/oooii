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
#include <oGPU/resource.h>
#include "d3d_debug.h"
#include "d3d_util.h"
#include "dxgi_util.h"

using namespace ouro::gpu::d3d;

namespace ouro { namespace gpu {

DeviceContext* get_dc(command_list* cl);

void resource::deinitialize()
{
	if (ro)
		((View*)ro)->Release();
	ro = nullptr;
}

char* resource::name(char* dst, size_t dst_size) const
{
	return ro ? debug_name(dst, dst_size, (View*)ro) : "uninitialized";
}

void resource::set(command_list* cl, uint slot, uint num_resources, resource* const* resources)
{
	ShaderResourceView* srvs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
	for (uint i = 0; i < num_resources; i++)
		srvs[i] = (ShaderResourceView*)resources[i]->get_resource();
	set_srvs(get_dc(cl), slot, num_resources, srvs); 	
}

void resource::set(command_list* cl, uint slot)
{
	set_srvs(get_dc(cl), slot, 1, (ShaderResourceView* const*)&ro);
}

void resource::set_highest_mip(command_list* cl, float mip)
{
	intrusive_ptr<Resource> r;
	((View*)ro)->GetResource(&r);
	get_dc(cl)->SetResourceMinLOD(r, mip);
}

void resource::update(command_list* cl, uint subresource, const void* src, uint row_pitch, uint depth_pitch)
{
	intrusive_ptr<Buffer> b;
	((View*)ro)->GetResource((Resource**)&b);
	get_dc(cl)->UpdateSubresource(b, subresource, nullptr, src, row_pitch, row_pitch);
}

}}
