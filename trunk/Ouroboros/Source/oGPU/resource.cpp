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
#include <oCore/windows/win_util.h>
#include "d3d_debug.h"
#include "d3d_resource.h"
#include <string>

using namespace ouro::gpu::d3d;

namespace ouro {
	namespace gpu {

Device* get_device(device* dev);

resource_info get_info(resource* r)
{
	return d3d::get_info((View*)r);
}

static resource* make_resource(Device* dev, const resource_info& info, const char* debug_name)
{
	oD3D_RESOURCE_DESC rdesc = to_resource_desc(info);
	intrusive_ptr<Resource> r = make_resource(dev, rdesc, debug_name);

	oD3D_VIEW_DESC vdesc = to_view_desc(info);
	intrusive_ptr<View> v = make_view(r, vdesc, debug_name);

	v->AddRef();
	return (resource*)v.c_ptr();
}

resource* make_resource(device* dev, const resource_info& info, const char* debug_name)
{
	return make_resource(get_device(dev), info, debug_name);

}

void unmake_resource(resource* r)
{
	if (r)
		((IUnknown*)r)->Release();
}

readback* make_readback_copy(resource* r, const char* debug_name)
{ 
	resource_info i = get_info(r);
	i.usage = resource_usage::readback;

	mstring rname;
	d3d::debug_name(rname, (View*)r);
	rname += ".cpu_copy";

	intrusive_ptr<Device> dev;
	((View*)r)->GetDevice(&dev);
	return (readback*)make_resource(dev, i, rname);
}

	} // namespace gpu
} // namespace ouro
