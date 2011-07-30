// $(header)

#include <oooii/oGPU.h>
#include <oooii/oString.h>
#include <oooii/oTest.h>
#include <oooii/oWindows.h>

struct TESTGPU : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oGPU::DESC desc;
		oGPU::GetDesc(0, &desc);

		if (oGetWindowsVersion() <= oWINDOWS_VISTA)
			oTESTB(desc.Index == 0, "Running on an OS earlier than Vista, so this won't work.");
		else
			oTESTB(desc.Index == 0, "Index is incorrect");

		char VRAMSize[64];
		oFormatMemorySize(VRAMSize, desc.VRAM, 1);
		char SharedSize[64];
		oFormatMemorySize(SharedSize, desc.SharedSystemMemory, 1);
		sprintf_s(_StrStatus, _SizeofStrStatus, "%s D3D%.01f %s (%s shared)", desc.Description, desc.D3DVersion, VRAMSize, SharedSize);

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTGPU);
