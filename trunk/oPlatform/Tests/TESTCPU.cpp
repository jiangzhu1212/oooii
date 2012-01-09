// $(header)
#include <oPlatform/oCPU.h>
#include <oBasis/oString.h>
#include <oPlatform/oTest.h>

struct TESTCPU : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oCPU_DESC cpu;
		oCPUEnum(0, &cpu);

		sprintf_s(_StrStatus, _SizeofStrStatus, "%s %s %s%s SSE%u", cpu.Type == oCPU_X64 ? "x64" : "x86", cpu.String, cpu.BrandString, cpu.HasHyperThreading ? " HT" : "", cpu.SSEVersion);
		return SUCCESS;
	}
};

oTEST_REGISTER(TESTCPU);
