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
#include <oGPU/device.h>

#include "../../test_services.h"

using namespace ouro::gpu;

namespace ouro {
	namespace tests {

void TESTdevice(test_services& _Services)
{
	device_init init("GPU device");
	init.enable_driver_reporting = true;
	init.version = version(10,0); // for more compatibility when running on varied machines
	device d(init);
	device_info i = d.get_info();

	oCHECK(i.adapter_index == 0, "Index is incorrect");
	oCHECK(i.feature_version >= version(9,0), "Invalid version retrieved");
	sstring VRAMSize, SharedSize, IVer, FVer, DVer;
	format_bytes(VRAMSize, i.native_memory, 1);
	format_bytes(SharedSize, i.shared_system_memory, 1);
	_Services.report("%s %s %s %s (%s shared) running on %s v%s drivers (%s)"
		, i.device_description.c_str()
		, as_string(i.api)
		, to_string2(FVer, i.feature_version)
		, VRAMSize.c_str()
		, SharedSize.c_str()
		, as_string(i.vendor)
		, to_string2(DVer, i.driver_version)
		, i.driver_description.c_str());
}

	} // namespace tests
} // namespace ouro
