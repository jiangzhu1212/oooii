// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oCore/cpu.h>
#include <oString/stringize.h>

#include "../../test_services.h"

namespace ouro {
	namespace tests {

void TESTcpu(test_services& _Services)
{
	cpu::info inf = cpu::get_info();
	bool HasHT = false, HasAVX = false;
	cpu::enumerate_features([&](const char* _FeatureName, const cpu::support::value& _Support)->bool
	{
		if (!_stricmp("Hyperthreading", _FeatureName))
			HasHT = _Support == cpu::support::full;
		else if (!_stricmp("AVX1", _FeatureName))
			HasAVX = _Support == cpu::support::full;
		return true;
	});

	_Services.report("%s %s %s%s%s %d HWThreads", ouro::as_string(inf.type), inf.string.c_str(), inf.brand_string.c_str(), HasHT ? " HT" : "", HasAVX ? " AVX" : "", inf.hardware_thread_count);
}

	}
}
