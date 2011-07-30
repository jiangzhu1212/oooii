// $(header)
#include <oooii/oCPU.h>
#include <oooii/oString.h>
#include <oooii/oTest.h>

struct TESTCPU : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oCPU::DESC desc;
		oCPU::GetDesc(0, &desc);

		sprintf_s(_StrStatus, _SizeofStrStatus, "%s %s %s%s SSE%u", desc.Type == oCPU::X64 ? "x64" : "x86", desc.String, desc.BrandString, desc.HasHyperThreading ? " HT" : "", desc.SSEVersion);
		return SUCCESS;
	}
};

oTEST_REGISTER(TESTCPU);
