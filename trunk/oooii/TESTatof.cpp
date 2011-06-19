// $(header)
#include <oooii/oAtof.h>
#include <oooii/oMath.h>
#include <oooii/oStdio.h>
#include <oooii/oSTL.h>
#include <oooii/oTest.h>

struct TESTatof : public oTest
{
	inline float frand01()
	{
		return (rand() % RAND_MAX) / static_cast<float>(RAND_MAX - 1);
	}

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
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
			oTESTB(oAtof(FloatStrings[i], &f), "Failed to atof string %s", FloatStrings[i]);
			oTESTB(oEqual(f, Floats[i]), "Float mismatch %f != %f", f, Floats[i]);
		}

		#ifdef _DEBUG // takes too long in debug
			static const size_t kNumFloats = 20000;
		#else
			static const size_t kNumFloats = 200000;
		#endif

		oTRACE("Preparing test data...");
		srand(1234);
		std::vector<char> buf(oMB(3));
		char* fstr = oGetData(buf);
		char* end = fstr + oGetDataSize(buf);
		for (size_t i = 0; i < kNumFloats; i++)
		{
			float f = -1000.0f + (2000.0f * frand01());
			size_t len = sprintf_s(fstr, std::distance(fstr, end), "%f\n", f);
			fstr += len + 1;
		}

		oTRACE("Benchmarking atof()...");

		std::vector<float> flist;
		flist.reserve(kNumFloats);

		fstr = oGetData(buf);
		double tend, tstart = oTimer();
		while (fstr < end)
		{
			float f = static_cast<float>(atof(fstr));
			flist.push_back(f);
			fstr += strcspn(fstr, "\n") + 1;
		}
		tend = oTimer();
		double atofTimeMS = (tend - tstart) * 1000.0;
		oTRACE("atof() %.04f ms", atofTimeMS);

		oTRACE("Benchmarking oAtof()...");
		flist.clear();

		fstr = oGetData(buf);
		tstart = oTimer();
		while (fstr < end)
		{
			float f;
			oAtof(fstr, &f);
			flist.push_back(f);
			fstr += strcspn(fstr, "\n") + 1;
		}
		tend = oTimer();
		double oAtofTimeMS = (tend - tstart) * 1000.0;
		oTRACE("oAtof() %.04f ms", oAtofTimeMS);

		sprintf_s(_StrStatus, _SizeofStrStatus, "%.02f v. %.02f ms for %u floats (%.02fx improvement)", atofTimeMS, oAtofTimeMS, kNumFloats, atofTimeMS / oAtofTimeMS);
		return SUCCESS;
	}
};

oTEST_REGISTER(TESTatof);