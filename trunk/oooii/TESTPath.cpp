// $(header)
#include <oooii/oAssert.h>
#include <oooii/oPath.h>
#include <oooii/oTest.h>

static const char* sDoubleSlashPath = "c:/my//path";
static const char* sCleanDoubleSlashPath = "c:/my/path";
static const char* sUNCPath = "//c/my/path";
static const char* sCleanUNCPath = "//c/my/path";

struct TESTPath : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		char path[_MAX_PATH];
		oTESTB(oGetSysPath(path, oSYSPATH_CWD), "Failed to get CWD");
		oTRACE("CWD: %s", path);
		oTESTB(oGetSysPath(path, oSYSPATH_APP), "Failed to get APP");
		oTRACE("APP: %s", path);
		oTESTB(oGetSysPath(path, oSYSPATH_TMP), "Failed to get TMP");
		oTRACE("TMP: %s", path);
		oTESTB(oGetSysPath(path, oSYSPATH_SYS), "Failed to get SYS");
		oTRACE("SYS: %s", path);
		oTESTB(oGetSysPath(path, oSYSPATH_OS), "Failed to get OS");
		oTRACE("OS: %s", path);
		oTESTB(oGetSysPath(path, oSYSPATH_DEV), "Failed to get DEV");
		oTRACE("DEV: %s", path);
		oTESTB(oGetSysPath(path, oSYSPATH_DESKTOP), "Failed to get DESKTOP");
		oTRACE("DESKTOP: %s", path);
		oTESTB(oGetSysPath(path, oSYSPATH_DESKTOP_ALLUSERS), "Failed to get DESKTOP_ALLUSERS");
		oTRACE("DESKTOP_ALLUSERS: %s", path);
		oTESTB(oGetSysPath(path, oSYSPATH_P4ROOT), "%s: %s", oGetErrnoString(oGetLastError()), oGetLastErrorDesc());
		oTRACE("P4ROOT: %s", path);

		oCleanPath(path, sDoubleSlashPath, '/');
		oTESTB(!strcmp(path, sCleanDoubleSlashPath), "Failed to clean path \"%s\"", sDoubleSlashPath);
		oCleanPath(path, sUNCPath, '/');
		oTESTB(!strcmp(path, sCleanUNCPath), "Failed to clean path \"%s\"", sUNCPath);

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTPath);
