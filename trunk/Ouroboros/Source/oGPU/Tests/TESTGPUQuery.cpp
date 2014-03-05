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
#include <oGPU/oGPU.h>

using namespace ouro::gpu;

namespace ouro {
	namespace tests {

void TESTquery()
{
	device_init init("GPU Query");
	init.driver_debug_level = gpu::debug_level::normal;
	init.version = version(10,0);
	std::shared_ptr<device> d = device::make(init);

	command_list* icl = d->immediate();

	// Test timer
	{
		query_info i;
		i.type = query_type::timer;
		std::shared_ptr<query> q = d->make_query("Timer", i);

		icl->begin_query(q);
		std::this_thread::sleep_for(std::chrono::seconds(1));
		icl->end_query(q);

		double SecondsPast = 0.0;
		d->read_query(q, &SecondsPast);
		oCHECK(SecondsPast > 0.0, "No time past!");
	}
}

	} // namespace tests
} // namespace ouro
