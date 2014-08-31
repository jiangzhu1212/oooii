// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oCore/adapter.h>
#include <oBase/assert.h>

#include "../../test_services.h"

namespace ouro {
	namespace tests {

void TESTadapter(test_services& _Services)
{
	adapter::info inf;
	int nAdapters = 0;

	adapter::enumerate([&](const adapter::info& _Info)->bool
	{
		sstring StrVer;
		version min_ver = adapter::minimum_version(_Info.vendor);
		if (nAdapters == 0)
			_Services.report("%s v%s%s", _Info.description.c_str(), to_string2(StrVer, _Info.version), _Info.version >= min_ver ? " (meets version requirements)" : "below min requirments");
		oTRACE("%s v%s%s", _Info.description.c_str(), to_string2(StrVer, _Info.version), _Info.version >= min_ver ? " (meets version requirements)" : "below min requirments");
		nAdapters++;
		return true;
	});
};

	} // namespace tests
} // namespace ouro
