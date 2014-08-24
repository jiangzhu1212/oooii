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
#include <oGPU/timer_query.h>
#include <oCore/windows/win_util.h>
#include "d3d_util.h"

using namespace ouro::gpu::d3d;

namespace ouro { namespace gpu {

Device* get_device(device& dev);
DeviceContext* get_dc(command_list& cl);

namespace timer_query_part
{	enum value {

	start,
	stop,
	disjoint,

	count,

};}

void timer_query::initialize(const char* name, device& dev)
{
	deinitialize();

	static const D3D11_QUERY_DESC sQueryDescs[3] =
	{
		{ D3D11_QUERY_TIMESTAMP, 0 },
		{ D3D11_QUERY_TIMESTAMP, 0 },
		{ D3D11_QUERY_TIMESTAMP_DISJOINT, 0 },
	};
	static_assert(oCOUNTOF(sQueryDescs) == oCOUNTOF(impl), "array mismatch");
			
	Device* D3DDevice = get_device(dev);

	intrusive_ptr<Query> q[timer_query_part::count];

	for (int i = 0; i < oCOUNTOF(sQueryDescs); i++)
		oV(D3DDevice->CreateQuery(&sQueryDescs[i], &q[i]));

	for (int i = 0; i < oCOUNTOF(sQueryDescs); i++)
	{
		q[i]->AddRef();
		impl[i] = q[i];
	}
}

void timer_query::deinitialize()
{
	for (int i = 0; i < oCOUNTOF(impl); i++)
		oSAFE_RELEASEV(impl[i]);
}

void timer_query::begin(command_list& cl)
{
	DeviceContext* dc = get_dc(cl);
	dc->Begin((Query*)impl[timer_query_part::disjoint]);
	dc->End((Query*)impl[timer_query_part::start]);
}

void timer_query::end(command_list& cl)
{
	DeviceContext* dc = get_dc(cl);
	dc->End((Query*)impl[timer_query_part::stop]);
	dc->End((Query*)impl[timer_query_part::disjoint]);
}

double timer_query::get_time(bool blocking)
{
	intrusive_ptr<Device> dev;
	((Query*)impl[timer_query_part::start])->GetDevice(&dev);

	intrusive_ptr<DeviceContext> dc;
	dev->GetImmediateContext(&dc);

	ullong results[timer_query_part::count];
	
	if (!copy_async_data(dc, (Query*)impl[timer_query_part::start], &results[timer_query_part::start], blocking))
		return -1.0;

	if (!copy_async_data(dc, (Query*)impl[timer_query_part::stop], &results[timer_query_part::stop], blocking))
		return -1.0;

	D3D11_QUERY_DATA_TIMESTAMP_DISJOINT Disjoint;
	if (!copy_async_data(dc, (Query*)impl[timer_query_part::disjoint], &Disjoint, blocking))
		return -1.0;

	oCHECK(!Disjoint.Disjoint, "timer is disjoint (laptop AC unplugged, overheating, GPU throttling)");
	return (results[timer_query_part::stop] - results[timer_query_part::start]) / double(Disjoint.Frequency);
}

}}
