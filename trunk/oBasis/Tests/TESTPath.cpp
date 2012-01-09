// $(header)
#include <oBasis/oError.h>
#include <oBasis/oPath.h>
#include "oBasisTestCommon.h"

static const char* sDoubleSlashPath = "c:/my//path";
static const char* sCleanDoubleSlashPath = "c:/my/path";
static const char* sUNCPath = "//c/my/path";
static const char* sCleanUNCPath = "//c/my/path";

bool oBasisTest_oPath()
{
	char path[_MAX_PATH];

	oCleanPath(path, sDoubleSlashPath, '/');
	oTESTB(!strcmp(path, sCleanDoubleSlashPath), "Failed to clean path \"%s\"", sDoubleSlashPath);
	oCleanPath(path, sUNCPath, '/');
	oTESTB(!strcmp(path, sCleanUNCPath), "Failed to clean path \"%s\"", sUNCPath);

	oErrorSetLast(oERROR_NONE, "");
	return true;
}

