// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
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

	}
}
