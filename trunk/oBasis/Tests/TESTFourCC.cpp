// $(header)
#include <oBasis/oFourCC.h>
#include <oBasis/oStringize.h>
#include "oBasisTestCommon.h"
#include <cstring>

bool oBasisTest_oFourCC()
{
	oFourCC fcc('TEST');

	char str[16];

	oTESTB(oToString(str, fcc), "oToString on oFourCC failed 1");
	oTESTB(!strcmp("TEST", str), "oToString on oFourCC failed 2");

	const char* fccStr = "RGBA";
	oTESTB(oFromString(&fcc, fccStr), "oFromSTring on oFourCC failed 1");
	oTESTB(oFourCC('RGBA') == fcc, "oFromSTring on oFourCC failed 2");

	return true;
}
