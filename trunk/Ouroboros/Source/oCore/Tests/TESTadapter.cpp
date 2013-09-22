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
#include <oCore/adapter.h>
#include <oCore/tests/oCoreTestRequirements.h>
#include <oBase/assert.h>

using namespace oStd;

namespace ouro {
	namespace tests {

void TESTadapter(requirements& _Requirements)
{
	adapter::info inf;
	int nAdapters = 0;

	adapter::enumerate([&](const adapter::info& _Info)->bool
	{
		sstring StrVer;
		version min_ver = adapter::minimum_version(_Info.vendor);
		if (nAdapters == 0)
			_Requirements.report("%s v%s%s", _Info.description.c_str(), to_string2(StrVer, _Info.version), _Info.version >= min_ver ? " (meets version requirements)" : "below min requirments");
		oTRACE("%s v%s%s", _Info.description.c_str(), to_string2(StrVer, _Info.version), _Info.version >= min_ver ? " (meets version requirements)" : "below min requirments");
		nAdapters++;
		return true;
	});
};

	} // namespace tests
} // namespace ouro
