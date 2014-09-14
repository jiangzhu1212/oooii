// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oGPU/device.h>
#include <oGPU/timer_query.h>

using namespace ouro::gpu;

namespace ouro {
	namespace tests {

void TESTquery()
{
	device_init init("GPU Query");
	init.enable_driver_reporting = true;
	init.version = version(10,0);
	device d(init);

	command_list& cl = d.immediate();

	// Test timer
	{
		timer_query q;
		q.initialize("Timer", d);
		q.begin(cl);
		std::this_thread::sleep_for(std::chrono::seconds(1));
		q.end(cl);

		double SecondsPast = q.get_time();
		oCHECK(SecondsPast > 0.0, "No time past!");
	}
}

	}
}
