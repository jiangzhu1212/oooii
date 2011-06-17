// $(header)
#include <oooii/oTest.h>
#include <oooii/oooii.h>

static const char* TESTHash_Filename = "lena.png";

struct TESTHash : public oTest
{
	
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		char tmp[_MAX_PATH];
		oGetSysPath(tmp, oSYSPATH_TMP);
		sprintf_s(tmp, "%s%s", tmp, TESTHash_Filename);

		oRef<threadsafe oBuffer> buffer1;
		oTESTB(oBuffer::Create(tmp, false, &buffer1), "Load failed: %s", tmp);
		oLockedPointer<oBuffer> lockedBuffer1(buffer1);

		uint128_t ExpectedHash = {5036109567207105818, 7580512480722386524};
		uint128_t ComputedHash = oHash_murmur3_x64_128( lockedBuffer1->GetData(), static_cast<unsigned int>( lockedBuffer1->GetSize() ) );
		oTESTB(ExpectedHash == ComputedHash, "Hash doesn't match" );

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTHash);