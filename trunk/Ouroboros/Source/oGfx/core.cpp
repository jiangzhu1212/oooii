// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
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
