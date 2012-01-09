// $(header)
#include <oBasis/oAssert.h>
#include <oPlatform/oSystem.h>
#include <oPlatform/oTest.h>

static const char* sDoubleSlashPath = "c:/my//path";
static const char* sCleanDoubleSlashPath = "c:/my/path";
static const char* sUNCPath = "//c/my/path";
static const char* sCleanUNCPath = "//c/my/path";

struct TESTSystemPath : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		char path[_MAX_PATH];
		oTESTB(oSystemGetPath(path, oSYSPATH_CWD), "Failed to get CWD");
		oTRACE("CWD: %s", path);
		oTESTB(oSystemGetPath(path, oSYSPATH_APP), "Failed to get APP");
		oTRACE("APP: %s", path);
		oTESTB(oSystemGetPath(path, oSYSPATH_TMP), "Failed to get TMP");
		oTRACE("TMP: %s", path);
		oTESTB(oSystemGetPath(path, oSYSPATH_SYS), "Failed to get SYS");
		oTRACE("SYS: %s", path);
		oTESTB(oSystemGetPath(path, oSYSPATH_OS), "Failed to get OS");
		oTRACE("OS: %s", path);
		oTESTB(oSystemGetPath(path, oSYSPATH_DEV), "Failed to get DEV");
		oTRACE("DEV: %s", path);
		oTESTB(oSystemGetPath(path, oSYSPATH_DESKTOP), "Failed to get DESKTOP");
		oTRACE("DESKTOP: %s", path);
		oTESTB(oSystemGetPath(path, oSYSPATH_DESKTOP_ALLUSERS), "Failed to get DESKTOP_ALLUSERS");
		oTRACE("DESKTOP_ALLUSERS: %s", path);
		oTESTB(oSystemGetPath(path, oSYSPATH_P4ROOT), "%s: %s", oAsString(oErrorGetLast()), oErrorGetLastString());
		oTRACE("P4ROOT: %s", path);
		return SUCCESS;
	}
};

oTEST_REGISTER(TESTSystemPath);
