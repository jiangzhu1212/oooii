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
#include <oGfx/core.h>

namespace ouro { namespace gfx {

void core::initialize(const char* name, bool enable_driver_reporting)
{
	gpu::device_init di;
	di.debug_name = name;
	di.enable_driver_reporting = enable_driver_reporting;
	device.initialize(di);

	bs.initialize(name, device);
	dss.initialize(name, device);
	rs.initialize(name, device);
	ss.initialize(name, device);

	ls.initialize(name, device);
	vs.initialize(device);
	ps.initialize(device);
}

void core::deinitialize()
{
	ps.deinitialize();
	vs.deinitialize();
	ls.deinitialize();

	ss.deinitialize();
	rs.deinitialize();
	dss.deinitialize();
	bs.deinitialize();

	device.deinitialize();
}

}}
