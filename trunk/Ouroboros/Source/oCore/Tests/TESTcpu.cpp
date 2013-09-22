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
#include <oCore/cpu.h>
#include <oCore/tests/oCoreTestRequirements.h>
#include <oBase/stringize.h>

namespace ouro {
	namespace tests {

void TESTcpu(requirements& _Requirements)
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

	_Requirements.report("%s %s %s%s%s %d HWThreads", ouro::as_string(inf.type), inf.string.c_str(), inf.brand_string.c_str(), HasHT ? " HT" : "", HasAVX ? " AVX" : "", inf.hardware_thread_count);
}

	} // namespace tests
} // namespace ouro
