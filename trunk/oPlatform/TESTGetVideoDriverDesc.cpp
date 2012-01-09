// $(header)
#include <oPlatform/oTest.h>

struct TESTGetVideoDriverDesc : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		
		oWINDOWS_VIDEO_DRIVER_DESC desc;
		memset(&desc, 0, sizeof(oWINDOWS_VIDEO_DRIVER_DESC));
		bool bSuccess = oWinGetVideoDriverDesc(&desc);

		if (bSuccess)
		{
			sprintf_s(_StrStatus, _SizeofStrStatus, "Video Driver: %i.%i", desc.MajorVersion, desc.MinorVersion);
		}
		else
		{
			sprintf_s(_StrStatus, _SizeofStrStatus, "ATI NOT IMPLEMENTED");
			bSuccess = false;
		}
		return bSuccess ? oTest::SUCCESS : oTest::FAILURE;
	}
};

oTEST_REGISTER(TESTGetVideoDriverDesc);