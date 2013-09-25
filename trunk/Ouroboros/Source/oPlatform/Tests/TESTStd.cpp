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
#include <oStd/tests/oStdTests.h>
#include <oPlatform/oProcessStatsMonitor.h>
#include "oTestIntegration.h"
#include <cstdlib>

namespace oStd {
	namespace tests {

struct requirements_implementation : requirements
{
	int rand() override { return ::rand(); }

	void get_cpu_utilization(float* _pAverage, float* _pPeek) override
	{
		oPROCESS_CPU_STATS stats;
		PSM.GetStats(&stats);
		*_pAverage = stats.AverageUsage;
		*_pPeek = stats.HighUsage;
	}

	void reset_cpu_utilization() override
	{
		PSM.Reset();
	}

	void vreport(const char* _Format, va_list _Args) override
	{
		oErrorSetLastV(0, _Format, _Args);
		oTRACEA("%s", oErrorGetLastString());
	}

	oProcessStatsMonitor PSM;
};

	} //namespace tests
} // namespace oStd

using namespace oStd::tests;

#define oTEST_REGISTER_STD_TEST0(_Name) oTEST_THROWS_REGISTER0(oCONCAT(oStd_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_STD_TEST(_Name) oTEST_THROWS_REGISTER(oCONCAT(oStd_, _Name), oCONCAT(TEST, _Name))

#define oTEST_REGISTER_STD_TEST_BUGGED0(_Name) oTEST_THROWS_REGISTER_BUGGED0(oCONCAT(oStd_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_STD_TEST_BUGGED(_Name) oTEST_THROWS_REGISTER_BUGGED(oCONCAT(oStd_, _Name), oCONCAT(TEST, _Name))

oTEST_REGISTER_STD_TEST(future);
