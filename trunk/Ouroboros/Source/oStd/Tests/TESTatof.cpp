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
#include <oStd/algorithm.h>
#include <oStd/atof.h>
#include <oStd/assert.h>
#include <oStd/function.h>
#include <oStd/equal.h>
#include <oStd/macros.h>
#include <oStd/throw.h>
#include <oStd/timer.h>
#include <vector>
#include <oStd/tests/oStdTestRequirements.h>

namespace oStd {
	namespace tests {

void TESTatof(requirements& _Requirements)
{
	static const char* FloatStrings[] = 
	{
		"3.1415926535897932384",
		"+0.341251",
		"-0.0959615",
		"3.15819e-06",
	};

	static const float Floats[] = 
	{
		3.1415926535897932384f,
		0.341251f,
		-0.0959615f,
		3.15819e-06f,
	};

	float f;
	oFORI(i, FloatStrings)
	{
		if (!oStd::atof(FloatStrings[i], &f))
			oTHROW(protocol_error, "Failed to oAtof string %s", FloatStrings[i]);

		if (!oStd::equal(f, Floats[i]))
			oTHROW(protocol_error, "Float mismatch %f != %f", f, Floats[i]);
	}

	#ifdef _DEBUG // takes too long in debug
		static const size_t kNumFloats = 20000;
	#else
		static const size_t kNumFloats = 200000;
	#endif

	oTRACEA("Preparing test data...");
	std::vector<char> buf(oMB(3));
	char* fstr = oStd::data(buf);
	char* end = fstr + oStd::size(buf);
	for (size_t i = 0; i < kNumFloats; i++)
	{
		float rand01 = (_Requirements.rand() % RAND_MAX) / static_cast<float>(RAND_MAX - 1);
		float f = -1000.0f + (2000.0f * rand01);
		size_t len = snprintf(fstr, std::distance(fstr, end), "%f\n", f);
		fstr += len + 1;
	}

	oTRACEA("Benchmarking atof()...");

	std::vector<float> flist;
	flist.reserve(kNumFloats);

	fstr = oStd::data(buf);
		
	oStd::timer t;
	while (fstr < end)
	{
		float f = static_cast<float>(::atof(fstr));
		flist.push_back(f);
		fstr += strcspn(fstr, "\n") + 1;
	}
		
	double atofDuration = t.milliseconds();
	oTRACEA("atof() %.02f ms", atofDuration);

	oTRACEA("Benchmarking oAtof()...");
	flist.clear();

	fstr = oStd::data(buf);
	t.reset();
	while (fstr < end)
	{
		float f;
		oStd::atof(fstr, &f);
		flist.push_back(f);
		fstr += strcspn(fstr, "\n") + 1;
	}
		
	double oAtofDuration = t.milliseconds();
	oTRACEA("atof() %.02f ms", oAtofDuration);
	_Requirements.report("%.02f v. %.02f ms for %u floats (%.02fx improvement)", atofDuration, oAtofDuration, kNumFloats, atofDuration / oAtofDuration);
}

	} // namespace tests
} // namespace oStd