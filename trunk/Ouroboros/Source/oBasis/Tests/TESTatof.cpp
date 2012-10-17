/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#include <oBasis/oAtof.h>
#include <oBasis/oAlgorithm.h>
#include <oBasis/oAssert.h>
#include <oBasis/oEqual.h>
#include <oBasis/oError.h>
#include <oBasis/oFunction.h>
#include <oBasis/oTimer.h>
#include <oBasisTests/oBasisTests.h>
#include <vector>

bool oBasisTest_oAtof(const oBasisTestServices& _Services)
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
	for (size_t i = 0; i < oCOUNTOF(FloatStrings); i++)
	{
		if (!oAtof(FloatStrings[i], &f))
			return oErrorSetLast(oERROR_GENERIC, "Failed to oAtof string %s", FloatStrings[i]);

		if (!oEqual(f, Floats[i]))
			return oErrorSetLast(oERROR_GENERIC, "Float mismatch %f != %f", f, Floats[i]);
	}

	#ifdef _DEBUG // takes too long in debug
		static const size_t kNumFloats = 20000;
	#else
		static const size_t kNumFloats = 200000;
	#endif

	oTRACE("Preparing test data...");
	std::vector<char> buf(oMB(3));
	char* fstr = oGetData(buf);
	char* end = fstr + oGetDataSize(buf);
	for (size_t i = 0; i < kNumFloats; i++)
	{
		float rand01 = (_Services.Rand() % RAND_MAX) / static_cast<float>(RAND_MAX - 1);
		float f = -1000.0f + (2000.0f * rand01);
		size_t len = oPrintf(fstr, std::distance(fstr, end), "%f\n", f);
		fstr += len + 1;
	}

	oTRACE("Benchmarking atof()...");

	std::vector<float> flist;
	flist.reserve(kNumFloats);

	fstr = oGetData(buf);
		
	oLocalTimer t;
	while (fstr < end)
	{
		float f = static_cast<float>(atof(fstr));
		flist.push_back(f);
		fstr += strcspn(fstr, "\n") + 1;
	}
		
	double atofDuration = t.Milliseconds();
	oTRACE("atof() %.02f ms", atofDuration);

	oTRACE("Benchmarking oAtof()...");
	flist.clear();

	fstr = oGetData(buf);
	t.Reset();
	while (fstr < end)
	{
		float f;
		oAtof(fstr, &f);
		flist.push_back(f);
		fstr += strcspn(fstr, "\n") + 1;
	}
		
	double oAtofDuration = t.Milliseconds();
	oTRACE("atof() %.02f ms", oAtofDuration);

	oErrorSetLast(oERROR_NONE, "%.02f v. %.02f ms for %u floats (%.02fx improvement)", atofDuration, oAtofDuration, kNumFloats, atofDuration / oAtofDuration);
	return true;
}
