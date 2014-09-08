// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

#include <oString/atof.h>
#include <oMemory/equal.h>
#include <array>
#include <vector>

#include "../../test_services.h"

namespace ouro { namespace tests {

void TESTatof(test_services& services)
{
	static const std::array<const char*, 4> sFloatStrings = 
	{
		"3.1415926535897932384",
		"+0.341251",
		"-0.0959615",
		"3.15819e-06",
	};

	static const std::array<float, 4> sFloats = 
	{
		3.1415926535897932384f,
		0.341251f,
		-0.0959615f,
		3.15819e-06f,
	};

	std::vector<char> buf(3*1024*1024);

	float f;
	for (size_t i = 0; i < sFloats.size(); i++)
	{
		if (!atof(sFloatStrings[i], &f))
		{
			services.snprintf(buf.data(), buf.size(), "ouro::atof failed on %f", sFloatStrings[i]);
			throw std::invalid_argument(buf.data());
		}

		if (!equal(f, sFloats[i]))
		{
			services.snprintf(buf.data(), buf.size(), "ouro::atof failed on %f", sFloatStrings[i]);
			throw std::logic_error(buf.data());
		}
	}

	#ifdef _DEBUG // takes too long in debug
		static const size_t kNumFloats = 20000;
	#else
		static const size_t kNumFloats = 200000;
	#endif

	services.report("Preparing test data...");
	char* fstr = buf.data();
	char* end = fstr + buf.size();
	for (size_t i = 0; i < kNumFloats; i++)
	{
		float rand01 = (services.rand() % RAND_MAX) / static_cast<float>(RAND_MAX - 1);
		float f = -1000.0f + (2000.0f * rand01);
		size_t len = services.snprintf(fstr, std::distance(fstr, end), "%f\n", f);
		fstr += len + 1;
	}

	services.report("Benchmarking stdc atof()...");

	std::vector<float> flist;
	flist.reserve(kNumFloats);

	fstr = buf.data();
		
	double start = services.now();
	while (fstr < end)
	{
		float f = static_cast<float>(::atof(fstr));
		flist.push_back(f);
		fstr += strcspn(fstr, "\n") + 1;
	}
		
	double atofDuration = services.now() - start;
	services.report("atof() %.02f ms", atofDuration);

	services.report("Benchmarking ouro::atof()...");
	flist.clear();

	fstr = buf.data();
	start = services.now();
	while (fstr < end)
	{
		float f;
		atof(fstr, &f);
		flist.push_back(f);
		fstr += strcspn(fstr, "\n") + 1;
	}
		
	double ouroAtofDuration = services.now() - start;
	services.report("atof() %.02f ms", ouroAtofDuration);
	services.report("%.02f v. %.02f ms for %u floats (%.02fx improvement)", atofDuration, ouroAtofDuration, kNumFloats, atofDuration / ouroAtofDuration);
}

}}
